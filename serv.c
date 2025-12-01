#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "list.h"

#define PORT 9001
#define MAX 1024

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(server_fd, 5) < 0) {  // Allow queue
        perror("listen"); exit(1);
    }

    printf("Server running on port %d (waiting for connections...)\n", PORT);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); continue; }

        list_t* list = list_alloc();
        char buffer[MAX], response[MAX];

        printf("New client connected.\n");

        while (1) {
            int n = recv(client_fd, buffer, MAX-1, 0);
            if (n <= 0) break;
            buffer[n] = '\0';

            char* cmd = strtok(buffer, " \t\n");
            if (!cmd) continue;

            response[0] = '\0';

            if (strcmp(cmd, "exit") == 0) {
                strcpy(response, "Goodbye!\n");
                send(client_fd, response, strlen(response), 0);
                break;
            }
            else if (strcmp(cmd, "print") == 0) {
                snprintf(response, MAX, "%s\n", listToString(list));
            }
            else if (strcmp(cmd, "get_length") == 0) {
                snprintf(response, MAX, "Length = %d\n", list_length(list));
            }
            else if (strcmp(cmd, "add_front") == 0) {
                char* val = strtok(NULL, " \t\n");
                if (!val) strcpy(response, "ERROR: Missing value\n");
                else {
                    list_add_to_front(list, atoi(val));
                    snprintf(response, MAX, "Added %d to front\n", atoi(val));
                }
            }
            else if (strcmp(cmd, "add_back") == 0) {
                char* val = strtok(NULL, " \t\n");
                if (!val) strcpy(response, "ERROR: Missing value\n");
                else {
                    list_add_to_back(list, atoi(val));
                    snprintf(response, MAX, "Added %d to back\n", atoi(val));
                }
            }
            else if (strcmp(cmd, "add_position") == 0) {
                char* idx = strtok(NULL, " \t\n");
                char* val = strtok(NULL, " \t\n");
                if (!idx || !val) strcpy(response, "ERROR: Usage: add_position <index> <value>\n");
                else {
                    int i = atoi(idx), v = atoi(val);
                    if (list_add_at_index(list, i, v) == 0)
                        snprintf(response, MAX, "Added %d at position %d\n", v, i);
                    else
                        strcpy(response, "ERROR: Index out of bounds\n");
                }
            }
            else if (strcmp(cmd, "remove_front") == 0) {
                int v = list_remove_from_front(list);
                snprintf(response, MAX, v != -1 ? "Removed %d from front\n" : "ERROR: List empty\n", v);
            }
            else if (strcmp(cmd, "remove_back") == 0) {
                int v = list_remove_from_back(list);
                snprintf(response, MAX, v != -1 ? "Removed %d from back\n" : "ERROR: List empty\n", v);
            }
            else if (strcmp(cmd, "remove_position") == 0) {
                char* idx = strtok(NULL, " \t\n");
                if (!idx) strcpy(response, "ERROR: Missing index\n");
                else {
                    int v = list_remove_at_index(list, atoi(idx));
                    snprintf(response, MAX, v != -1 ? "Removed %d at position %s\n" : "ERROR: Invalid index\n", v, idx);
                }
            }
            else if (strcmp(cmd, "get") == 0) {
                char* idx = strtok(NULL, " \t\n");
                if (!idx) strcpy(response, "ERROR: Missing index\n");
                else {
                    int v = list_get_elem_at(list, atoi(idx));
                    snprintf(response, MAX, v != -1 ? "Value at %s: %d\n" : "ERROR: Index out of bounds\n", idx, v);
                }
            }
            else {
                strcpy(response, "ERROR: Unknown command\n");
            }

            send(client_fd, response, strlen(response), 0);
        }

        list_free(list);
        close(client_fd);
        printf("Client disconnected. Waiting for new connection...\n");
    }

    close(server_fd);
    return 0;
}
