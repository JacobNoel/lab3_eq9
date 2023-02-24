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

    // Obtenir les clés pour les zones de mémoire partagée
    int opt;
    int largeur_sortie = 0;
    int hauteur_sortie = 0;
    int rMode = 0;
    while ((opt = getopt(argc, argv, "w:h:r:")) != -1) {
        switch (opt) {
            case 'w':
                largeur_sortie = atoi(optarg);
                break;
            case 'h':
                hauteur_sortie = atoi(optarg);
                break;
            case 'r':
                rMode = atoi(optarg);
                break;
            default:
                perror("Usage: %s -w <largeur> -h <hauteur> -r <rMode> /chemin/vers/mem_entree /chemin/vers/mem_sortie");
                return -1;
        }
    }
    if (optind + 2 != argc) {
        perror("Usage: %s -w <largeur> -h <hauteur> -r <rMode> /chemin/vers/mem_entree /chemin/vers/mem_sortie");
        return -1;
    }
    char *input_mem, *output_mem;
    input_mem = argv[optind];
    output_mem = argv[optind + 1];

/*
    initMemoirePartageeLecteur et initMemoirePartageeEcrivain

    lire une image depuis une zone mémoire partagée

    switch (rMode) {
        case 0:
            ResizeGrid rg = resizeNearestNeighborInit();
            resizeNearestNeighbor();
            écrire la nouvelle image dans la zone mémoire partagée
            break;
        case 1:
            ResizeGrid rg = resizeBilinearInit();
            resizeBilinear();
            écrire la nouvelle image dans la zone mémoire partagée
            break;
        default:
            perror("L'argument r doit être 0 ou 1.");
    }
*/

    return 0;
}
