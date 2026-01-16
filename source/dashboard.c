//
// Created by Bigé Loïc on 15/01/2026.
//

#include "../Lib/dashboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int loadDashboardConf(int *dashSock, struct sockaddr_in *dashAdr) {
    *dashSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (*dashSock < 0) {
        perror("[DASHBOARD] : dash socket");
        return EXIT_FAILURE;
    }

    dashAdr->sin_family = AF_INET;
    dashAdr->sin_port = htons(DASHBOARD_PORT);
    if (inet_pton(AF_INET, DASHBOARD_IP,&dashAdr->sin_addr) != 1) {
        perror("[DASHBOARD] : inet_aton");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int sendHello(const int id, const char* ip) {
    struct sockaddr_in dashAdr = {0};
    int dashSock;
    if (loadDashboardConf(&dashSock, &dashAdr) != EXIT_SUCCESS) {
        close(dashSock);
        return EXIT_FAILURE;
    }

    char buffer[DASHBOARD_MESSAGE_LENGTH];
    sprintf(buffer,"{\"id\": \"R%d\", \"type\": \"PING\",\"ip\": \"%s\"}",id,ip);
    if (sendto(dashSock, buffer, strlen(buffer),0,(struct sockaddr *)&dashAdr, sizeof(struct sockaddr_in))<0) {
        perror("[DASHBOARD] : sendto");
        return EXIT_FAILURE;
    }

    close(dashSock);
    return EXIT_SUCCESS;
}



int sendLinkUp(const int id1, const int id2) {

    struct sockaddr_in dashAdr = {0};
    int dashSock;
    if (loadDashboardConf(&dashSock, &dashAdr) != EXIT_SUCCESS) {
        close(dashSock);
        return EXIT_FAILURE;
    }

    char buffer[DASHBOARD_MESSAGE_LENGTH];
    sprintf(buffer, "{\"type\": \"LINK\",\"from\": \"R%d\",\"to\": \"R%d\",\"status\": \"UP\"}",id1,id2 );
    if (sendto(dashSock, buffer, strlen(buffer),0,(struct sockaddr *)&dashAdr, sizeof(struct sockaddr_in))<0) {
        perror("[DASHBOARD] : sendto");
        return EXIT_FAILURE;
    }

    close(dashSock);
    return EXIT_SUCCESS;
}


int sendLinkDown(const int id1, const int id2) {
    struct sockaddr_in dashAdr = {0};
    int dashSock;
    if (loadDashboardConf(&dashSock, &dashAdr) != EXIT_SUCCESS) {
        close(dashSock);
        return EXIT_FAILURE;
    }

    char buffer[DASHBOARD_MESSAGE_LENGTH];
    sprintf(buffer, "{\"type\": \"LINK\",\"from\": \"R%d\",\"to\": \"R%d\",\"status\": \"DOWN\"}",id1,id2 );
    if (sendto(dashSock, buffer, strlen(buffer),0,(struct sockaddr *)&dashAdr, sizeof(struct sockaddr_in))<0) {
        perror("[DASHBOARD] : sendto");
        return EXIT_FAILURE;
    }

    close(dashSock);
    return EXIT_SUCCESS;
}