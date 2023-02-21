#include "allocateurMemoire.h"
#include <sys/mman.h>

// TODO: Implementez ici votre allocateur memoire utilisant l'interface decrite dans allocateurMemoire.h


// Prépare les buffers nécessaires pour une allocation correspondante aux tailles
// d'images passées en paramètre. Retourne 0 en cas de succès, et -1 si un
// problème (par exemple manque de mémoire) est survenu.
int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie)
{
    int *mem =(int*) (malloc(5*tailleImageEntree*sizeof(int)));

    if (mem == NULL)
    {
        printf("Échec de l'allocation\n");
        return -1;
    }
    mlockall(MCL_CURRENT);

    return 0;
}

// Ces deux fonctions doivent pouvoir s'utiliser exactement comme malloc() et free()
// (dans la limite de la mémoire disponible, bien sûr)
void* tempsreel_malloc(size_t taille)
{
    return malloc(taille);
}


void tempsreel_free(void* ptr)
{
    free(ptr);
}

// N'oubliez pas de créer le fichier allocateurMemoire.c et d'y implémenter les fonctions décrites ici!
