#include <sys/types.h> // key_t
#include <sys/ipc.h> // IPC_STAT, IPC_RMID
#include <sys/msg.h> // msgget, struct msqid_ds, msgrcv, msgctl
#include <unistd.h> // close
#include <stdio.h>
#include <string.h> // strtok, strcat

#define MSGMAX 1024

struct msgbuf {
    long mtype;
    char mtext[MSGMAX];
};

int main() {
    // Получаем идентификатор очереди сообщений

    int msqid;
    do {
        msqid = msgget(666, 0);
    } while (msqid == -1);
    printf("Server: Connected to MQ\n");

    // Выбираем первое сообщение из очереди

    struct msgbuf msg;
    if (msgrcv(msqid, &msg, MSGMAX, 1, 0) == -1) {
        perror("Server: msgrcv() error");
        return 1;
    }

    // Определяем файл максимального размера, его владельца, время последней модификации и время записи в очередь

    char cmd[1124] = "ls -l --time-style=+%s ";
    char *p = strtok(msg.mtext, "\n");
    while (p != NULL) {
        strcat(cmd, p);
        strcat(cmd, " ");
        p = strtok(NULL, "\n");
    }
    FILE *cmd_output;
    cmd_output = popen(cmd, "r");
    char maxSizeFileOwner[255], fileOwner[255], filename[252], maxSizeFilename[252], someData[255];
    long fileModTime, maxSizeFileModTime, fileSize, maxFileSize = 0;
    while (fscanf(cmd_output, "%s %s %s %s %li %li %s",  // NOLINT(cert-err34-c)
                  someData, someData, fileOwner, someData, &fileSize, &fileModTime, filename
    ) != EOF) {
        if (fileSize > maxFileSize) {
            maxFileSize = fileSize;
            strcpy(maxSizeFileOwner, fileOwner);
            strcpy(maxSizeFilename, filename);
            maxSizeFileModTime = fileModTime;
        }
    }
    printf("Maximum size binary file stats:\n");
    printf(" Name: %s\n", maxSizeFilename);
    printf(" Owner: %s\n", maxSizeFileOwner);
    printf(" Size: %li\n", maxFileSize);
    printf(" Last modification time: %li\n", maxSizeFileModTime);
    struct msqid_ds mqInfo;
    msgctl(msqid, IPC_STAT, &mqInfo);
    printf(" Send time: %li\n", mqInfo.msg_stime);

    // Удаляем очередь сообщений

    msgctl(msqid, IPC_RMID, NULL);

    close(msqid);
    return 0;
}
