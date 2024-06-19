#include "common.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFSZ 1024
#define MAX_CLIENTS 10

sem_t semaphore;

const char *phrases1[] = {
    "Um anel para a todos governar",
    "Na terra de Mordor onde as sombras se deitam",
    "Não é o que temos, mas o que fazemos com o que temos"
};

const char *phrases2[] = {
    "Frase do filme 2 - vamos la 1",
    "Frase do filme 2 - vamos la 2",
    "Frase do filme 2 - vamos la 3"
};

const char *phrases3[] = {
    "Frase do filme 3 - saimos 1",
    "Frase do filme 3 - saimos 2",
    "Frase do filme 3 - saimos 3"
};

typedef struct {
    int socket;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    int option;
} client_info_t;

void *handle_client(void *arg) {
    client_info_t *client_info = (client_info_t *)arg;
    int s = client_info->socket;
    struct sockaddr_storage client_addr = client_info->client_addr;
    socklen_t client_addr_len = client_info->client_addr_len;
    int option = client_info->option;
    free(client_info);

    const char **phrases;
    int phrase_count;

    if (option == 1) {
        phrases = phrases1;
        phrase_count = sizeof(phrases1) / sizeof(phrases1[0]);
    } else if (option == 2) {
        phrases = phrases2;
        phrase_count = sizeof(phrases2) / sizeof(phrases2[0]);
    } else {
        phrases = phrases3;
        phrase_count = sizeof(phrases3) / sizeof(phrases3[0]);
    }

    char buf[BUFSZ];

    for (int i = 0; i < phrase_count; i++) {
        sleep(3);
        snprintf(buf, BUFSZ, "%s", phrases[i]);

        ssize_t count = sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&client_addr, client_addr_len);
        if (count == -1) {
            perror("sendto");
        }
    }

    // Enviar mensagem de finalização
    snprintf(buf, BUFSZ, "END");
    sendto(s, buf, strlen(buf), 0, (struct sockaddr *)&client_addr, client_addr_len);

    sem_post(&semaphore);
    return NULL;
}


void *display_client_count(void *arg) {
    while (1) {
        sleep(4);
        int value;
        sem_getvalue(&semaphore, &value);
        printf("Clientes conectados: %d\n", MAX_CLIENTS - (value + 1));
    }
    return NULL;
}

void usage(int argc, char **argv) {
    printf("usage: %s <V4|V6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s = socket(storage.ss_family, SOCK_DGRAM, 0);
    if (s == -1) {
        loexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        loexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)&storage;
    if (0 != bind(s, addr, sizeof(storage))) {
        loexit("bind");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting for messages...\n", addrstr);

    sem_init(&semaphore, 0, MAX_CLIENTS);
    pthread_t counter_thread;
    pthread_create(&counter_thread, NULL, display_client_count, NULL);
    pthread_detach(counter_thread);

    while (1) {
        sem_wait(&semaphore);

        client_info_t *client_info = malloc(sizeof(client_info_t));
        client_info->socket = s;
        client_info->client_addr_len = sizeof(client_info->client_addr);

        char buf[BUFSZ];
        ssize_t count = recvfrom(s, buf, BUFSZ - 1, 0, (struct sockaddr *)&client_info->client_addr, &client_info->client_addr_len);
        if (count == -1) {
            perror("recvfrom");
            free(client_info);
            sem_post(&semaphore);
            continue;
        }

        buf[count] = '\0';
        char caddrstr[BUFSZ];
        addrtostr((struct sockaddr *)&client_info->client_addr, caddrstr, BUFSZ);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        client_info->option = atoi(buf);

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, client_info);
        pthread_detach(tid);
    }

    close(s);
    sem_destroy(&semaphore);
    return 0;
}
