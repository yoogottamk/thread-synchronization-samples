#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "main.h"
#include "cab.h"
#include "server.h"
#include "rider.h"

void init() {
    srand(time(0));

    // currently unlocked
    pthread_mutex_init(&accessCabs, 0);
    pthread_mutex_init(&paymentMutex, 0);

    sem_init(&paymentServers, 0, 0);

    freePoolOnes = 0;
    freeCabs = N;

    waitingForCab = (CabType *) getSharedMemory(sizeof(int) * N);
    for(int i = 0; i < N; i++)
        waitingForCab[i] = 0;

    allCabs = (Cab **) getSharedMemory(sizeof(Cab *) * N);
    allRiders = (Rider **) getSharedMemory(sizeof(Rider *) * M);

    servers_t = (pthread_t **) getSharedMemory(sizeof(pthread_t *) * K);
    riders_t = (pthread_t **) getSharedMemory(sizeof(pthread_t *) * M);

    for(int i = 0; i < K; i++)
       servers_t[i] = (pthread_t *) getSharedMemory(sizeof(pthread_t));

    for(int i = 0; i < M; i++)
        riders_t[i] = (pthread_t *) getSharedMemory(sizeof(pthread_t));

    condWait = (pthread_cond_t *) getSharedMemory(sizeof(pthread_cond_t) * M);
    for(int i = 0; i < M; i++)
        pthread_cond_init(&condWait[i], 0);

    for(int i = 0; i < N; i++)
        initCab(i);

    for(int i = 0; i < M; i++)
        initRider(i);

    for(int i = 0; i < K; i++)
        initPaymentServer(i);
}

void destroy() {

}

int main() {
    scanf("%d %d %d", &N, &M, &K);

    init();

    for(int i = 0; i < M; i++)
        pthread_join(*riders_t[i], 0);

    // sending signal to server that
    // it's over
    for(int i = 0; i < K; i++)
        sem_post(&paymentServers);

    for(int i = 0; i < K; i++)
        pthread_join(*servers_t[i], 0);

    destroy();

    return 0;
}


void * getSharedMemory(size_t size) {
    key_t key = IPC_PRIVATE;
    int id = shmget(key, size, IPC_CREAT | 0666);

    if(id == -1)
        perror("SHMGET FAILED");

    void* ans = (void *)shmat(id, 0, 0); 
    if(ans == (void*) -1){
        perror("SHMAT FAILED");
    }

    return ans;
}
