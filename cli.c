#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9001
#define MAX 1024

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); exit(1); }

    struct sockaddr_in serv_addr = {0};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // FIX: Use 127.0.0.1 instead of INADDR_ANY
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        exit(1);
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(1);
    }

    char buffer[MAX], response[MAX];
    while (1) {
        printf("Enter command (or 'menu'): ");
        fflush(stdout);
        if (!fgets(buffer, MAX, stdin)) break;
        buffer[strcspn(buffer, "\n")] = 0;
        if (strlen(buffer) == 0) continue;

        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "exit", 4) == 0) {
            recv(sock, response, MAX-1, 0);
            printf("SERVER: %s\n", response);
            break;
        }

        if (strncmp(buffer, "menu", 4) == 0) {
            printf("Available: print, get_length, add_front <v>, add_back <v>, add_position <i> <v>, remove_front, remove_back, remove_position <i>, get <i>, exit\n");
            continue;
        }

        int n = recv(sock, response, MAX-1, 0);
        if (n > 0) {
            response[n] = '\0';
            printf("SERVER: %s\n", response);
        }
    }
    close(sock);
    return 0;
}
