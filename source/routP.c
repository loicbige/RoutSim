#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // struct sockaddr_in
#include <time.h>
#include <sys/select.h>
#include <stdbool.h>

#include "../Lib/tabrout.h"

#define BUF_SIZE 64        // we should receive less...
#define IPV4_ADR_STRLEN 16 // == INET_ADDRSTRLEN
#define LOCALHOST "127.0.0.1"
#define DASHBOARD_PORT 8080
#define DASHBOARD_IP "127.0.0.1"
#define NO_BASE_PORT 17900


void createRoutingTable(routingTable *myRoutingTable, const char *adr, const char *id)
{

  char idInitConfigFile[20]; // Id of the configuration file of the router
  char myId[32];

  /* Building ID Router from command args */
  sprintf(myId, "R%s %s", id, adr);
  printf("[ROUTEUR] : %s\n", myId);

  sprintf(idInitConfigFile, "config/rf%s", id);
  strcat(idInitConfigFile, ".txt");
  // printf("\n Nom fichier Configuration : %s",idInitConfigFile);
  /* Loading My Routing Table from Initial Config file */
  init_routingTable(myRoutingTable, idInitConfigFile);
  printf("[ROUTEUR] : %d entrées initialement chargées \n", myRoutingTable->nb_entry);
  display_routingTable(myRoutingTable);
}
void sendtoDashboard(char *type, char* message, int id) {
  int dashSock = socket(AF_INET, SOCK_DGRAM,0);
  if (dashSock < 0) {
    perror("dash socket");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in dashAdr;
  memset(&dashAdr, 0, sizeof(dashAdr));
  dashAdr.sin_family = AF_INET;
  dashAdr.sin_port = htons(DASHBOARD_PORT);
  if (inet_pton(AF_INET, LOCALHOST, &dashAdr.sin_addr) <= 0) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }
  char buf[BUF_SIZE];
  memset(buf, 0, BUF_SIZE);
  sprintf(buf, "{\"id\": \"R%d\", \"type\": \"%s\", %s}", id,type, message);

  if (sendto(dashSock, buf, strlen(buf), 0, (struct sockaddr *)&dashAdr, sizeof(struct sockaddr_in)) < 0) {
    perror("sendto");
    exit(EXIT_FAILURE);
  }
  close(dashSock);
}

int sendRoutingTable(routingTable *rt, int sock, struct sockaddr_in *neighborAdr) {

  if (neighborAdr != NULL) {
    if (sendto(sock,&rt->nb_entry, sizeof(rt->nb_entry), 0, (struct sockaddr *)neighborAdr, sizeof(struct sockaddr_in)) < 0) {
      perror("[ROUTER] : sendto nbentry");
      exit(EXIT_FAILURE);
    }
    char buff[BUF_SIZE];
    memset(buff, 0, BUF_SIZE);
    for (int i = 0; i < rt->nb_entry; i++) {
      memset(buff, 0, BUF_SIZE);
      snprintf(buff, BUF_SIZE, "%s %hu %hu %hhu", rt->entries[i].destination, rt->entries[i].port, rt->entries[i].weight, (uint8_t)rt->entries[i].type);
      if (sendto(sock, buff,strlen(buff),0, (struct sockaddr *)neighborAdr,sizeof(struct sockaddr_in)) < 0) {
        perror("[ROUTER] : sendto loop");
        exit(EXIT_FAILURE);
      }

    }
    return 0;
  }
else {
  printf("[ROUTEUR] : can't send, the routing table is empty...\n");
  return -1;
}
}


int recvRoutingTable(int sock, routingTable *rt) {
  struct sockaddr_in neighborAdr;
  memset(&neighborAdr, 0, sizeof(struct sockaddr_in));
  socklen_t neighborAdrLen = sizeof(struct sockaddr_in);
  int nbEntries = 0;
  char buff[BUF_SIZE];
  memset(buff, 0, BUF_SIZE);
  if (recvfrom(sock,&nbEntries,sizeof(nbEntries), 0,
    (struct sockaddr *)&neighborAdr, &neighborAdrLen ) < 0) {
    perror("[ROUTER] : recvfrom nbentries");
    exit(EXIT_FAILURE);
    }
  printf("[ROUTEUR] : received %d entries\n", nbEntries);
  for (int i = 0; i < nbEntries; i++) {
    memset(buff, 0, BUF_SIZE);
    if (recvfrom(sock,buff,BUF_SIZE, 0,(struct sockaddr *)&neighborAdr, &neighborAdrLen)<0) {
      perror("[ROUTER] : recvfrom loop");
      exit(EXIT_FAILURE);
    }
    if (add_routingTable(rt, buff) == EXIT_FAILURE) {
      perror("[ROUTER] : error adding routing table");
      continue;
    }
  }

  return 0;
}

void findNeighborAndSendRoutingTable(routingTable *rt, int mySock) {
  struct sockaddr_in neighborAdr;
  for (int  i = 0;  i < rt->nb_entry; ++ i) {
    if (rt->entries[i].type == ROUTER) {
      memset(&neighborAdr, 0, sizeof(struct sockaddr_in));
      neighborAdr.sin_family = AF_INET;
      neighborAdr.sin_port = htons(rt->entries[i].port);
      if (inet_pton(AF_INET, LOCALHOST, &neighborAdr.sin_addr) <= 0) {
        perror("inet_pton neighbor sendroutingtable");
        exit(EXIT_FAILURE);
      }
      sendRoutingTable(rt,mySock,&neighborAdr);
    }
  }
}

int main(int argc, char const *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <ip> <id> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char message[BUF_SIZE];
  memset(message, 0, BUF_SIZE);

  sprintf(message, "\"ip\": \"%s\"", argv[1]);
  sendtoDashboard("HELLO", message, atoi(argv[2]));

  routingTable rt;
  createRoutingTable(&rt, argv[1], argv[2]);

  int mySock = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in myAdr = {0};
  myAdr.sin_family = AF_INET;
  myAdr.sin_port = htons((uint16_t)atoi(argv[3]));
  if (inet_pton(AF_INET, LOCALHOST, &myAdr.sin_addr) <= 0) {
    perror("inet_pton");
    exit(EXIT_FAILURE);
  }

  if (mySock < 0) {
    perror("my socket");
    exit(EXIT_FAILURE);
  }

  if (bind(mySock,(struct sockaddr *)&myAdr,sizeof(myAdr)) == -1) {
    perror("bind");
    exit(EXIT_FAILURE);
  }

  while (true) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mySock, &readfds);
    struct timeval tv = {5,0} ;
    int selectValue = select(mySock+1, &readfds, NULL, NULL, &tv);


    if (selectValue < 0) {
      perror("select");
      continue;
    }
    if (selectValue == 0) {
      findNeighborAndSendRoutingTable(&rt, mySock);
      continue;

    }
    if (FD_ISSET(mySock, &readfds)) {
      display_routingTable(&rt);
      recvRoutingTable(mySock, &rt);
    }
  }
  return 1;
}
