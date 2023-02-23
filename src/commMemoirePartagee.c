#include "commMemoirePartagee.h"

// TODO: implementez ici les fonctions decrites dans commMemoirePartagee.h

// Appelé au début du programme pour l'initialisation de la zone mémoire (cas du lecteur)
int initMemoirePartageeLecteur(const char* identifiant, struct memPartage *zone)
{
    struct stat buff;
    int* id;
    
    //Tant que espace memoire n'existe pas, ou vide
    while (!zone | (stat(identifiant,&buff) < 0)){
        continue;
    }
    zone->fd = shm_open(identifiant, O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO);
    if (zone->fd < 0){
        printf("shm_open failed\n");
        return -1;
    }
    //Tant que compteur de l'ecrivain est a 0
    while (zone->header->frameWriter==0){
        continue;
    }

    //mmap sur le descripteur de fichier
    id = (int*)(mmap(NULL,buff.st_size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, zone->fd, 0));
    if (id == NULL){
        printf("reader mmap fail\n");
        return 1;
    }
    
    while (zone->header->frameWriter==0)
    {
        continue;
    }
    
    return 0;
}



// Appelé au début du programme pour l'initialisation de la zone mémoire (cas de l'écrivain)
int initMemoirePartageeEcrivain(const char* identifiant, struct memPartage *zone, size_t taille, struct memPartageHeader* headerInfos)
{
    int fd, l, m;
    //Ouverture de l'espace mémoire avec shm_open
    fd = shm_open(identifiant, O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
    if (fd < 0){
        printf("shm_open failed\n");
        return -1;
    }
    
    size_t tailleTotale = sizeof(struct memPartageHeader) + taille;

    //Agrandissement espace mémoire avec ftruncate
    l = ftruncate(fd,tailleTotale);
    if (l < 0){
        printf("ftruncate failed\n");
        return -1;
    }

    //Utilisation de mmap
    int* id;
    id = (int*)(mmap(NULL,tailleTotale, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0));
    if (id==NULL){
        printf("writer mmap fail\n");
        return -1;
    }

    //Assignation des différents champs de zone:
    zone->fd = fd;
    zone->header = headerInfos;
    zone->tailleDonnees = tailleTotale;
    zone->copieCompteur = headerInfos->frameReader;
    zone->data = (unsigned char*)(id + sizeof(struct memPartageHeader));

    //Creation du mutex 
    m = pthread_mutex_init(&headerInfos->mutex,NULL);
    if (m < 0){
        printf("mutex creation failed\n");
    }

    //Acquisition du mutex par l'ecrivain
    m = pthread_mutex_lock(&(zone->header->mutex));
    if (m < 0){
        printf("fail writer mutex lock\n");
        return -1;
    }

    //Incrémentation compteur ecrivain
    zone->header->frameWriter++;

    return 0;
}



// Appelé par le lecteur pour se mettre en attente d'un résultat
int attenteLecteur(struct memPartage *zone)
{
    int i;
    while (zone->header->frameWriter==zone->copieCompteur){
        continue;
    }
    i = pthread_mutex_lock(&(zone->header->mutex));
    if (i < 0){
        printf("fail writer mutex lock\n");
        return -1;
    }
    return 0;
}



// Fonction spéciale similaire à attenteLecteur, mais asynchrone : cette fonction ne bloque jamais.
// Cela est utile pour le compositeur, qui ne doit pas bloquer l'entièreté des flux si un seul est plus lent.
int attenteLecteurAsync(struct memPartage *zone)
{
    int i;
    while (zone->header->frameWriter==zone->copieCompteur){
        continue;
    }
    i = pthread_mutex_trylock(&(zone->header->mutex));
    if (i < 0){
        printf("fail writer mutex trylock\n");
        return -1;
    }
    return 0;
}



// Appelé par l'écrivain pour se mettre en attente de la lecture du résultat précédent par un lecteur
int attenteEcrivain(struct memPartage *zone)
{
    int i;
    while (zone->header->frameReader==zone->copieCompteur){
        continue;
    }

    i = pthread_mutex_lock(&(zone->header->mutex));
    if (i < 0){
        printf("fail writer mutex lock\n");
        return -1;
    }
    return 0;
}
