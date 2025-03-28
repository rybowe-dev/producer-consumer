#include "shared.hpp"

int main () {
    // creating a shared memory object using shm_open()
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cout << "Producer failed to create shared memory object. " << std::endl;
        return 0;
    }

    // resize region of shared memory according size of region that producer and consumer will share
    ftruncate(shm_fd, sizeof(sharedMemory));
    
    // map producer memory space to region of shared memory using mmap()
    sharedMemory* shared = (sharedMemory*) mmap(NULL, sizeof(sharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared == MAP_FAILED) {
        std::cout << "Producer failed to map shared memory region to process memory space. " << std::endl;
        return 0;
    }

    // intialize in and out 
    shared->in = 0;
    shared->out = 0;

    // open semaphores for # of empty indexes, # of filled indexes, and mutual exclusion to protect critical section 
    sem_t* empty = sem_open(SEM_EMPTY_NAME, O_CREAT, 0644, BUFFER_SIZE);
    sem_t* full = sem_open(SEM_FULL_NAME, O_CREAT, 0644, 0);
    sem_t* mutex = sem_open(SEM_MUTEX_NAME, O_CREAT, 0644, 1);

    if (empty == SEM_FAILED || full == SEM_FAILED || mutex == SEM_FAILED) {
        std::cout << "Failed to open one or more semaphores. " << std::endl;
        return 0;
    }

    std::cout << "Producer process started. " << std::endl;
    
    for (int i = 0; i < 10; ++i) { // main loop for producer
        // ensure that there is space in buffer for data to be allocated
        sem_wait(empty);
        // ensure that consumer is not executing in its critical section
        sem_wait(mutex);
        
        int item = i * 10;

        shared->buffer[shared->in] = item;
        std::cout << "Produced: " << item << " at index: " << shared->in << std::endl;
        // move in to next index of buffer, must utilize circular array concept
        shared->in = (shared->in + 1) % BUFFER_SIZE;

        // signal to consumer that producer has exited crtical section
        sem_post(mutex);
        // alert consumer that there is data has been allocated to buffer
        sem_post(full);

        sleep(2);
    }
    // close all semaphores
    sem_close(empty);
    sem_close(full);
    sem_close(mutex);

    // unlink all semaphores
    sem_unlink(SEM_EMPTY_NAME);
    sem_unlink(SEM_FULL_NAME);
    sem_unlink(SEM_MUTEX_NAME);
    
    // unmap shared memory from process memory space
    munmap(shared, sizeof(sharedMemory));
    // close shared memory region
    close(shm_fd);

    shm_unlink(SHM_NAME);
    
}