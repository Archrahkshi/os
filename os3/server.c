#include <sys/socket.h> // socket, AF_INET, SOCK_DGRAM, bind, recvfrom, sendto, struct sockaddr
#include <netinet/in.h> // struct sockaddr_in, htons, htonl, INADDR_ANY
#include <stdio.h>
#include <stdlib.h> // exit
#include <string.h>
#include <unistd.h>

#define MSGMAX 2048
#define PORT 7500


int main() {
    // Создаём гнездо без установления соединения домена INET

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Server: socket() 1 error");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sock, (struct sockaddr *) &addr, sizeof addr) == -1) {
        perror("Server: bind() error");
        exit(1);
    }

    // Поступает клиентское сообщение

    char msg[MSGMAX];
    int bytes_read = recvfrom(sock, msg, sizeof msg, 0, NULL, NULL);
    msg[bytes_read] = '\0';
    printf("%s\n", msg);

    char cmd1[2148] = "echo \"";
    strcat(cmd1, msg);
    strcat(cmd1, "\" | sed -e \"s/[[:space:]]\\+/ /g\" | sed -e \"s/[[:space:]]/\\\\\\| /g\"");

    FILE *f = popen(cmd1, "r");
    if (f == NULL) {
        perror("Server: popen() 1 error");
        exit(1);
    }
    char buffer[MSGMAX];
    while (!feof(f))
        fgets(buffer, sizeof buffer, f);
    fclose(f);

    // Сортируем клиентские файлы по времени создания

    char cmd2[2148] = "ls -lct | grep '";
    strcat(cmd2, buffer);
    cmd2[strlen(cmd2) - 1] = ' ';
    cmd2[strlen(cmd2) - 3] = ' ';
    cmd2[strlen(cmd2) - 4] = '\'';
    strcat(cmd2, "| awk '{ print $9 }' | tr \"\\n\" \" \"");

    memset(buffer, 0, sizeof buffer);
    f = popen(cmd2, "r");
    if (f == NULL) {
        perror("Server: popen() 2 error");
        exit(1);
    }
    while (!feof(f))
        fgets(buffer, sizeof buffer, f);
    fclose(f);

    // Выводим этот список на экран...

    printf("by time of creation: %s\n", buffer);


    close(sock);

    // ...и записываем в клиентское гнездо

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Server: socket() 2 error");
        exit(1);
    }

    sendto(sock, buffer, sizeof buffer, 0, (struct sockaddr *) &addr, sizeof addr);

    close(sock);
}
