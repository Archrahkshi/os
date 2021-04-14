#include <sys/types.h> // key_t
#include <sys/ipc.h> // IPC_NOWAIT
#include <sys/sem.h> // semget, struct sembuf, semop
#include <sys/shm.h> // shmget, shmat, shmdt
#include <stdio.h>
#include <string.h> // bzero

int main() {
    int shmid, semid;
    do {
        shmid = shmget(666, 0, 0);
    } while (shmid == -1);
    do {
        semid = semget(666, 0, 0);
    } while (semid == -1);
    struct sembuf
            captureShm[2] = {{0, 0, 0}, {0, 2, 0}},
            sub_1 = {0, -1, 0},
            hard_sub_2 = {0, -2, IPC_NOWAIT};
    char *shmPtr;
    for (;;) {
        semop(semid, captureShm, 2);
        shmPtr = (char *) shmat(shmid, 0, 0);
        if (shmPtr[0] != 0) {
            shmdt(shmPtr);
            if (semop(semid, &hard_sub_2, 1) == -1)
                semop(semid, &sub_1, 1);
        } else {
            bzero(shmPtr, 8192);
            FILE *cmd_output = popen("file * | grep ELF | awk -F : '$1{print $1;}'", "r");
            fread(shmPtr, 1, 8192, cmd_output);
            shmdt(shmPtr);
            if (semop(semid, &hard_sub_2, 1) == -1)
                semop(semid, &sub_1, 1);
            semop(semid, captureShm, 2);
            shmPtr = shmat(shmid, 0, 0);
            printf("Client2: Got a response from server: %s\n", shmPtr);
            bzero(shmPtr, 8192);
            shmdt(shmPtr);
            if (semop(semid, &hard_sub_2, 1) == -1)
                semop(semid, &sub_1, 1);
            return 0;
        }
    }
}
