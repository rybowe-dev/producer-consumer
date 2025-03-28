// Rylan Bowe
// 3/28/2025
// producer-consumer problem

#ifndef MY_SHARED_HPP
#define MY_SHARED_HPP

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

const int BUFFER_SIZE = 2; // size of shared buffer

// shared memory object name
const char* SHM_NAME = "/my_shared_memory";

// names that will be used for semaphores
const char* SEM_EMPTY_NAME = "/sem_empty";
const char* SEM_FULL_NAME = "/sem_full";
const char* SEM_MUTEX_NAME = "/sem_mutex";

// stuct that will be stored in shared region of memory
struct sharedMemory {
    int buffer[BUFFER_SIZE];
    int in; // producer process writes here
    int out; // consumer process reads here
};



#endif