#include <sys/socket.h> // socket, AF_INET, SOCK_DGRAM, sendto, bind, struct sockaddr, recvfrom
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
        perror("Client: socket() 1 error");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Получаем полное имя текущего каталога,...

    FILE *f = popen("pwd", "r");
    if (f == NULL) {
        perror("Client: popen() 1 error");
        exit(1);
    }
    char msg[MSGMAX];
    while (!feof(f))
        fgets(msg, sizeof msg, f);
    fclose(f);

    // ...список файлов, содержащих "main",...

    char buffer[MSGMAX];
    f = popen("grep -ls main * | tr '\\n' ' '", "r");
    if (f == NULL) {
        perror("Client: popen() 2 error");
        exit(1);
    }
    while (!feof(f))
        fgets(buffer, sizeof buffer, f);
    fclose(f);

    // ...и передаём в серверное гнездо

    strcat(msg, buffer);
    printf("%s\n", msg);

    sendto(sock, msg, sizeof msg, 0, (struct sockaddr *) &addr, sizeof addr);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("Client: socket() 2 error");
        exit(1);
    }

    // Получаем ответ сервера и печатаем его

    while (bind(sock, (struct sockaddr *) &addr, sizeof addr));

    char buf[MSGMAX];
    int bytes_read = recvfrom(sock, buf, sizeof buf, 0, NULL, NULL);
    buf[bytes_read] = '\0';
    printf("%s", buf);

    close(sock);
}
