#include "shared.hpp"

int main () {
    // creating a shared memory object using shm_open()
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666); 
    if (shm_fd == -1) {
        std::cout << "Consumer failed to open shared memory object. " << std::endl;
        return 0;
    }

    // mapping process memory space to region of shared memory 
    sharedMemory* shared = (sharedMemory*) mmap(NULL, sizeof(sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 
    if (shared == MAP_FAILED) {
        std::cout << "Consumer failed to map shared memory region to process memory space. " << std::endl;
        return 0;
    }

    // opening semaphores that will be used for # of empty slots, # of full slots, and a mutex to ensure mutual exclusion inside critical section
    sem_t* empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0644, BUFFER_SIZE);
    sem_t* full = sem_open(SEM_FULL_NAME, O_CREAT, 0644, 0);
    sem_t* mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);

    // check if all semaphores opened successfully
    if (empty == SEM_FAILED || full == SEM_FAILED || mutex == SEM_FAILED) {
        std::cout << "Failed to open one or more semaphores. " << std::endl;
        return 0;
    }

    std::cout << "Consumer process started. " << std::endl;

    for (int i = 0; i < 10; ++i) { // main loop for consumer process
        // consumer waits until there is data to consume in buffer
        sem_wait(full);
        // wait until critical section is available for entry to avoid simultaneous access
        sem_wait(mutex);

        int j = shared->buffer[shared->out];
        std::cout << "Consumed: " << j << " at index: " << shared->out << std::endl;
        // move to next index of buffer, uses circular array
        shared->out = (shared->out + 1) % BUFFER_SIZE;

        // signal to producer that it is free to enter critical section
        sem_post(mutex); 
        // signal to producer that data has been consumed
        sem_post(empty); 

        sleep(2); 
    }
    // close all semaphores that were used
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);

    // unlink all semaphores
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_MUTEX_NAME);

    // unmap process memory space from shared memory region
    munmap(shared, sizeof(sharedMemory));
    // close shared memory region that was opened at the beginning
    close(shm_fd); 
    // unlink shared memory
    shm_unlink(SHM_NAME);
}