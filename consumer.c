
/*consumer*/
#define __LIBRARY__
#include <stdio.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/sem.h>

_syscall2(sem_t *, sem_open, const char *, name, int, value);
_syscall1(int, sem_post, sem_t *, sem);
_syscall1(int, sem_wait, sem_t *, sem);
_syscall1(int, sem_unlink, const char *, name);

_syscall1(int, shmat, int, shmid);
_syscall2(int, shmget, unsigned int, key, size_t, size);

_syscall0(long, get_jiffies);

#define ITEM_NUM 200
#define BUFFER_SIZE 10
#define SHM_KEY 2023

int main(int argc, char *argv[])
{
    sem_t *Empty, *Full, *Mutex;
    int used = 0, shm_id, i, index = 0;
    int *p;
    long jiffies = 0;

    Empty = sem_open("Empty", BUFFER_SIZE);
    Full = sem_open("Full", 0);
    Mutex = sem_open("Mutex", 1);

    if ((shm_id = shmget(SHM_KEY, BUFFER_SIZE * sizeof(int))) < 0)
        printf("shmget failed!\n");

    if ((p = (int *)shmat(shm_id)) < 0)
        printf("link error!\n");
    for (i = 0; i < ITEM_NUM; i++)
    {
        sem_wait(Full);
        sem_wait(Mutex);
        jiffies = get_jiffies();
        printf("Consumer. jiffies: %ld, buffer index: %d, data: %d\n", jiffies, index, p[index]);
        index = (index + 1) % BUFFER_SIZE;
        fflush(stdout);
        sem_post(Mutex);
        sem_post(Empty);
    }
    sem_unlink("Mutex");
    sem_unlink("Full");
    sem_unlink("Empty");
    return 0;
}