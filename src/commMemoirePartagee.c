#include "commMemoirePartagee.h"

// TODO: implementez ici les fonctions decrites dans commMemoirePartagee.h

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas du lecteur)
int initMemoirePartageeLecteur(const char* identifiant, struct memPartage *zone)
{
    int mem,fd;
    size_t size;
    while (!zone | (sizeof(zone)<sizeof(struct memPartageHeader))){
        continue;
    }
    while (zone->header->frameWriter==0){
        continue;
    }
    
    mem = mmap(zone->fd,sizeof(struct memPartageHeader), NULL,  NULL,fd, NULL);
    size= zone->tailleDonnees;
    while (zone->header->frameWriter==0)
    {
        continue;
    }
    
}



// Appelé au début du programme pour l'initialisation de la zone mémoire (cas de l'écrivain)
int initMemoirePartageeEcrivain(const char* identifiant, struct memPartage *zone, size_t taille, struct memPartageHeader* headerInfos)
{
    int fd, l, mem, m;
    //Ouverture de l'espace mémoire avec shm_open
    fd = shm_open(identifiant, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
    if (fd < 0){
        printf("shm_open failed");
        return -1;
    }
    
    //Agrandissement espace mémoire avec ftruncate
    l = ftruncate(fd,taille);
    if (l < 0){
        printf("ftruncate failed");
        return -1;
    }

    //Utilisation de mmap
    mem = mmap(NULL,sizeof(headerInfos), PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, fd, NULL);

    //Assignation des différents champs de zone:
    zone->fd = mem;
    zone->header = headerInfos;
    zone->tailleDonnees = taille;
    //zone->data = 
    zone->copieCompteur = headerInfos->frameReader;
    
    //Creation du mutex 
    m = pthread_mutex_init(headerInfos->mutex,NULL);
    if (m < 0){
        printf("mutex creation failed");
    }

    //Incrémenter framewriter
    zone->header->frameWriter++;

    return 0;
}



// Appelé par le lecteur pour se mettre en attente d'un résultat
int attenteLecteur(struct memPartage *zone)
{
    while (zone->header->frameWriter==zone->copieCompteur){
        continue;
    }
    pthread_mutex_lock(&(zone->header->mutex));
}



// Fonction spéciale similaire à attenteLecteur, mais asynchrone : cette fonction ne bloque jamais.
// Cela est utile pour le compositeur, qui ne doit pas bloquer l'entièreté des flux si un seul est plus lent.
int attenteLecteurAsync(struct memPartage *zone);



// Appelé par l'écrivain pour se mettre en attente de la lecture du résultat précédent par un lecteur
int attenteEcrivain(struct memPartage *zone)
{
    while (zone->header->frameReader==zone->copieCompteur){
        continue;
    }
    pthread_mutex_lock(&(zone->header->mutex));
}