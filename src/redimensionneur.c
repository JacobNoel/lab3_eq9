// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>
#include "schedsupp.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#include "allocateurMemoire.h"
#include "commMemoirePartagee.h"
#include "utils.h"


int main(int argc, char* argv[]){
    // On desactive le buffering pour les printf(), pour qu'il soit possible de les voir depuis votre ordinateur
	setbuf(stdout, NULL);
    
    // Écrivez le code permettant de redimensionner une image (en utilisant les fonctions précodées
    // dans utils.c, celles commençant par "resize"). Votre code doit lire une image depuis une zone 
    // mémoire partagée et envoyer le résultat sur une autre zone mémoire partagée.
    // N'oubliez pas de respecter la syntaxe de la ligne de commande présentée dans l'énoncé.

    // Vérifier que la ligne de commande est correcte
    if (argc != 4) {
        fprintf(stderr, "Usage: %s input_shm_key output_shm_key scale_factor\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Obtenir les clés pour les zones de mémoire partagée
    int input_shm_key = atoi(argv[1]);
    int output_shm_key = atoi(argv[2]);

    // Obtenir l'échelle de redimensionnement à partir des arguments de la ligne de commande
    float scale_factor = atof(argv[3]);

    // Obtenir l'image d'entrée à partir de la zone de mémoire partagée
    int input_shm_id = shmget(input_shm_key, 0, 0);
    if (input_shm_id == -1) {
        perror("shmget for input image failed");
        exit(EXIT_FAILURE);
    }
    unsigned char *input_image = (unsigned char *) shmat(input_shm_id, NULL, 0);
    if (input_image == (unsigned char *) -1) {
        perror("shmat for input image failed");
        exit(EXIT_FAILURE);
    }

    // Obtenir les dimensions de l'image d'entrée
    int input_width, input_height, input_channels;

    // Allouer de la mémoire pour l'image de sortie
    int output_width = (int) (input_width * scale_factor);
    int output_height = (int) (input_height * scale_factor);
    int output_channels = input_channels;
    unsigned char *output_image = (unsigned char *) malloc(output_width * output_height * output_channels * sizeof(unsigned char));
    if (output_image == NULL) {
        perror("malloc for output image failed");
        exit(EXIT_FAILURE);
    }

    // Redimensionner l'image
    // TODO trouver quoi faire avec rg
    ResizeGrid rg;
    resizeNearestNeighbor(input_image, input_height, input_width, output_image, output_height, output_width, rg, output_channels);

    // Envoyer l'image de sortie vers la zone de mémoire partagée
    int output_shm_id = shmget(output_shm_key, output_width * output_height * output_channels * sizeof(unsigned char), IPC_CREAT | 0666);
    if (output_shm_id == -1) {
        perror("shmget for output image failed");
        exit(EXIT_FAILURE);
    }
    unsigned char *output_shm_ptr = (unsigned char *) shmat(output_shm_id, NULL, 0);
    if (output_shm_ptr == (unsigned char *) -1) {
        perror("shmat for output image failed");
        exit(EXIT_FAILURE);
    }
    memcpy(output_shm_ptr, output_image, output_width * output_height * output_channels * sizeof(unsigned char));

    // Détacher les zones de mémoire partagée
    shmdt(input_image);
    shmdt(output_shm_ptr);

    // Libérer la mémoire allouée pour les images
    free(output_image);

    return 0;
}
