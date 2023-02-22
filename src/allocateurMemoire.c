#include "allocateurMemoire.h"
#include <sys/mman.h>
#include <sys/param.h>

// TODO: Implementez ici votre allocateur memoire utilisant l'interface decrite dans allocateurMemoire.h

void *mem;
char occupation[5]={0};// si case 0 -> libre
int found = 0;
void *memspace;


// Prépare les buffers nécessaires pour une allocation correspondante aux tailles
// d'images passées en paramètre. Retourne 0 en cas de succès, et -1 si un
// problème (par exemple manque de mémoire) est survenu.
int prepareMemoire(size_t tailleImageEntree, size_t tailleImageSortie)
{
    //On  trouve la taille max entre les 2 dimensions
    int i;
    i = MAX((int)(tailleImageEntree),(int)(tailleImageSortie));

    //Creation d'un grand espace mémoire
    mem = malloc(5*i*sizeof(int));

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
    int i=0;
    if (mem==NULL){
        printf("pas de memoire pre-allouée\n");
        exit(1);
    }
    while(found==0){
        if (occupation[i]==0){
            found=1;
            occupation[i]=1;
            memspace = (void*)(i*(int)(taille)+sizeof(mem));
        }
        else{i++;}
    }
    found = 0;
    return memspace;
}


void tempsreel_free(void* ptr)
{
    occupation[(int)ptr]=0;
    free(ptr);
}
