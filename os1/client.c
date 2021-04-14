#include <sys/types.h> // key_t
#include <sys/ipc.h> // IPC_CREAT, IPC_EXCL
#include <sys/msg.h> // msgget, msgsnd
#include <stdio.h>
#include <string.h> // strtok, strcat
#include <unistd.h> // close

#define MSGMAX 1024

struct msgbuf {
    long mtype;
    char mtext[MSGMAX];
};

int main() {
    // Создаём очередь сообщений

    int msqid = msgget(666, IPC_CREAT | IPC_EXCL | 0600); // NOLINT(hicpp-signed-bitwise)
    if (msqid == -1) {
        perror("Client: msgget() error");
        return 1;
    }

    // Создаём первое сообщение

    struct msgbuf msg;
    msg.mtype = 1;
    bzero(msg.mtext, sizeof msg.mtext);

    // Записываем в него имена всех бинарных файлов текущего каталога

    FILE* cmd_output = popen("file * -i | grep binary | awk -F : '{print $1}'", "r");
    fread(msg.mtext, 1, MSGMAX, cmd_output);
    fclose(cmd_output);

    // Отправляем в очередь

    if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        perror("Client: msgsnd() 1 error");
        return 1;
    }

    // Определяем размер этих файлов в байтах

    char cmd[1124] = "wc -c ";
    char* p = strtok(msg.mtext, "\n");
    while (p != NULL) {
        strcat(cmd, p);
        strcat(cmd, " ");
        p = strtok(NULL, "\n");
    }
    strcat(cmd, " | tail -n 1");
    cmd_output = popen(cmd, "r");

    // Создаём второе сообщение и записываем в него эту информацию

    msg.mtype = 2;
    bzero(msg.mtext, sizeof msg.mtext);
    fscanf(cmd_output, "%s", msg.mtext);
    fclose(cmd_output);

    // Отправляем в очередь

    if (msgsnd(msqid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        perror("Client: msgsnd() 2 error");
        return 2;
    }

    close(msqid);
    return 0;
}
