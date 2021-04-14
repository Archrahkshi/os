#include <sys/types.h> // key_t
#include <sys/ipc.h> // IPC_CREAT, IPC_EXCL, IPC_RMID
#include <sys/sem.h> // semget, struct sembuf, semop, semctl
#include <sys/shm.h> // shmget, shmat, shmdt, shmctl
#include <stdio.h>
#include <string.h> // bzero, strlen, strtok, strcpy, strcat
#include <unistd.h> // close

int main() {
    int shmid = shmget(666, 8192, IPC_CREAT | IPC_EXCL | 0600); // NOLINT(hicpp-signed-bitwise)
    if (shmid == -1) {
        perror("Server: shmget() error");
        return 1;
    }
    int semid = semget(666, 1, IPC_CREAT | IPC_EXCL | 0600); // NOLINT(hicpp-signed-bitwise)
    if (semid == -1) {
        perror("Server: semget() error");
        return 2;
    }

    char *shmPtr;
    FILE *cmd_output;
    char cmd[260];
    char response[8192];
    bzero(response, 8192);
    struct sembuf
            captureShm[2] = {{0, 0, 0}, {0, 1, 0}},
            sub_1 = {0, -1, 0},
            wait_0 = {0, 0, 0};
    for (int i = 0; i < 2;) {
        semop(semid, &sub_1, 1);
        semop(semid, captureShm, 2);
        int fsize;
        shmPtr = (char *) shmat(shmid, NULL, 0);
        if (strlen(shmPtr) > 0) {
            char *p = strtok(shmPtr, "\n");
            while (p != NULL) {
                bzero(cmd, sizeof cmd);
                strcpy(cmd, "wc -l ");
                strcat(cmd, p);
                cmd_output = popen(cmd, "r");
                fscanf(cmd_output, "%i", &fsize); // NOLINT(cert-err34-c)
                fclose(cmd_output);
                if (fsize > 4) {
                    strcat(response, p);
                    strcat(response, " ");
                }
                p = strtok(NULL, "\n");
            }
            bzero(shmPtr, 8192);
            strcpy(shmPtr, response);
            bzero(response, 8192);
            shmdt(shmPtr);
            semop(semid, &sub_1, 1);
            i++;
        } else {
            shmdt(shmPtr);
            semop(semid, &sub_1, 1);
        }
    }
    semop(semid, &wait_0, 1);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID, NULL);
    close(semid);
    close(shmid);
    return 0;
}
