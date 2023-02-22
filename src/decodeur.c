// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>
#include "schedsupp.h"

#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "utils.h"

#include "jpgd.h"


//pour MAP_POPULATE
#if __linux__
#include <linux/version.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,22)
#define _MAP_POPULATE_AVAILABLE
#endif
#endif

#ifdef _MAP_POPULATE_AVAILABLE
#define MMAP_FLAGS (MAP_PRIVATE | MAP_POPULATE)
#else
#define MMAP_FLAGS MAP_PRIVATE
#endif


// Définition de diverses structures pouvant vous être utiles pour la lecture d'un fichier ULV
#define HEADER_SIZE 4
const char header[] = "SETR";

struct videoInfos{
        uint32_t largeur;
        uint32_t hauteur;
        uint32_t canaux;
        uint32_t fps;
};

/******************************************************************************
* FORMAT DU FICHIER VIDEO
* Offset     Taille     Type      Description
* 0          4          char      Header (toujours "SETR" en ASCII)
* 4          4          uint32    Largeur des images du vidéo
* 8          4          uint32    Hauteur des images du vidéo
* 12         4          uint32    Nombre de canaux dans les images
* 16         4          uint32    Nombre d'images par seconde (FPS)
* 20         4          uint32    Taille (en octets) de la première image -> N
* 24         N          char      Contenu de la première image (row-first)
* 24+N       4          uint32    Taille (en octets) de la seconde image -> N2
* 24+N+4     N2         char      Contenu de la seconde image
* 24+N+N2    4          uint32    Taille (en octets) de la troisième image -> N2
* ...                             Toutes les images composant la vidéo, à la suite
*            4          uint32    0 (indique la fin du fichier)
******************************************************************************/


int main(int argc, char* argv[]){
    // On desactive le buffering pour les printf(), pour qu'il soit possible de les voir depuis votre ordinateur
	setbuf(stdout, NULL);
    
    // Écrivez le code de décodage et d'envoi sur la zone mémoire partagée ici!
    // N'oubliez pas que vous pouvez utiliser jpgd::decompress_jpeg_image_from_memory()
    // pour décoder une image JPEG contenue dans un buffer!
    // N'oubliez pas également que ce décodeur doit lire les fichiers ULV EN BOUCLE

    // On ouvre le fichier ULV
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Impossible d'ouvrir le fichier");
        return 1;
    }

    // On va chercher les stats du fd pour mettre la taille dans file_size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Impossible d'obtenir les informations sur le fichier");
        return 1;
    }
    off_t file_size = st.st_size;

    // On map le fichier dans la mémoire vive (RAM)
    char* file_data = (char*)mmap(NULL, file_size, PROT_READ, MAP_POPULATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("Impossible de mapper le fichier en mémoire");
        return 1;
    }

    // On ferme le descripteur de fichier (fd), mais le fichier reste mappé dans la mémoire vive
    close(fd);

    // Boucle de décodage du fichier
    while (1) {

        // On lit le header (4 premiers octets)
        char header[4];
        memcpy(header, file_data, 4);
        if (memcmp(header, "SETR", 4) != 0) {
            perror("Le fichier n'est pas un fichier ULV valide");
            return 1;
        }

        // On lit les infos sur les images
        uint32_t width, height, channels, fps;
        memcpy(&width, file_data + 4, 4);
        memcpy(&height, file_data + 8, 4);
        memcpy(&channels, file_data + 12, 4);
        memcpy(&fps, file_data + 16, 4);

        // Boucle de décodage des images
        while (1) {

            // On va chercher la taille de l'image
            uint32_t size;
            memcpy(&size, file_data + 20, 4);
            // Si la taille est de 0, ca signifie la fin de la vidéo.
            // On termine la boucle de décodage des images
            if (size == 0) {
                break;
            }

            // On décompresse l'image
            int width_out, height_out, comps_out;
            unsigned char* image_data = jpgd::decompress_jpeg_image_from_memory((unsigned char*)(file_data + 24), size, &width_out, &height_out, &comps_out, channels);

            // On initialise la zone mémoire partagée mem1
	        // On l'ouvre en accès read-write
            int shm_fd = shm_open("/mem1", O_RDWR, 0777);
            if (shm_fd < 0) {
                perror("shm_open");
                return 1;
            }

            //comment aller chercher la taille qu'on veut allouer? Est ce que c'est la taille du fichier, de l'image, ou autre?
            // ici, j'ai mis size parce que ca me semblait logique
            if (ftruncate(shm_fd, size) < 0) {
                perror("ftruncate");
                return 1;
            }

            char* ulv_shm = (char*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
            if (ulv_shm == MAP_FAILED) {
                perror("mmap");
                return 1;
            }

            // On copie l'image décompressée dans la zone mémoire partagée
            memcpy(ulv_shm, image_data, width_out * height_out * comps_out);

            // On nettoie la mémoire
            free(image_data);

            // On passe à la prochaine image
            file_data += 20 + size;
        }

        // On retourne au début du fichier pour recommencer la boucle
        file_data = (char*)(file_data - st.st_size);
    }

    return 0;
}
