#include "common.h" 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSZ 1024

void usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void receive_messages(int s) {
    char buf[BUFSZ];
    struct sockaddr_in from_addr;
    socklen_t from_addr_len = sizeof(from_addr);

    while (1) {
        memset(buf, 0, BUFSZ);
        int count = recvfrom(s, buf, BUFSZ, 0, (struct sockaddr *)&from_addr, &from_addr_len);
        if (count == -1) {
            perror("recvfrom");
            close(s);
            exit(EXIT_FAILURE);
        } else if (count == 0) {
            printf("Servidor desconectado.\n");
            break;
        } else {
            printf("Frase do servidor: %s\n", buf);
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    while (1) {
        printf("\nMenu:\n");
        printf("1. Batati e batata\n");
        printf("2. Ai que calor\n");
        printf("3. Odeio filme\n");
        printf("4. Sair\n");
        printf("Escolha uma opção: ");

        int option;
        scanf("%d", &option);

        if (option == 4) {
            printf("Saindo...\n");
            break;
        }

        if (option < 1 || option > 3) {
            printf("Opção inválida. Tente novamente.\n");
            continue;
        }

        int s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
            perror("inet_pton");
            exit(EXIT_FAILURE);
        }

        char buf[BUFSZ];
        snprintf(buf, BUFSZ, "%d", option);
        if (sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            perror("sendto");
            close(s);
            exit(EXIT_FAILURE);
        }

        printf("Conectado ao servidor. Aguardando frases...\n");
        receive_messages(s);

        close(s);
    }

    return 0;
}
