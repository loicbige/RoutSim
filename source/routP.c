#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> // struct sockaddr_in
#include <sys/time.h>
#include <sys/select.h>
#include <stdbool.h>

#include "../Lib/dashboard.h"
#include "../Lib/tabrout.h"

#define BUF_SIZE 64        // we should receive less...
#define IPV4_ADR_STRLEN 16 // == INET_ADDRSTRLEN
#define LOCALHOST "127.0.0.1"

#define NO_BASE_PORT 17900

long now(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;

}
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

int sendRoutingTable(const routingTable *rt, int const sock, struct sockaddr_in *neighborAdr) {
  if (rt->nb_entry == 0) {
    fprintf(stderr,"[ROUTER] : can't send, the routing table is empty...\n");
    return EXIT_FAILURE;
  }

  if (sendto(sock, rt, sizeof(*rt), 0, (struct sockaddr *)neighborAdr, sizeof(struct sockaddr_in)) < 0) {
    perror("[ROUTER] : sendto()");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


int recvRoutingTable(const int sock, routingTable *rt) {
  struct sockaddr_in neighborAdr = {0};
  socklen_t neighborAdrLen = sizeof(struct sockaddr_in);

  routingTable neighborRoutingTable = {0};

  if (recvfrom(sock, &neighborRoutingTable, sizeof(neighborRoutingTable), 0,(struct sockaddr *)&neighborAdr, &neighborAdrLen) < 0) {
    perror("[ROUTER] : recvfrom()");
    return EXIT_FAILURE;
  }
  printf("[ROUTER] : received %hhu entries\n", neighborRoutingTable.nb_entry);
  for (int i = 1; i < neighborRoutingTable.nb_entry; i++) { // COMMENCE à 1 pour éviter de compter l'adresse du routeur
    if (add_routingTable(rt, &neighborRoutingTable.entries[i]) == EXIT_FAILURE) {
      perror("[ROUTER] : entry already exist \n");
      continue;
    }
  }
  return ntohs(neighborAdr.sin_port);
}

void findNeighborAndSendRoutingTable(const routingTable *rt, int mySock) {
  struct sockaddr_in neighborAdr;
  for (int  i = 0;  i < rt->nb_entry; ++ i) {
    if (rt->entries[i].type == ROUTER && rt->entries[i].port != rt->entries[0].port) {
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

int main(const int argc, char const *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <ip> <id> <port>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  sendHello(atoi(argv[2]),argv[1]);

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
  long next_ping = now();
  long next_sendrt = now();
  while (true) {
    long t = now();

    if (t >= next_sendrt) {
      findNeighborAndSendRoutingTable(&rt, mySock);
      next_sendrt = t + 5;
    }

    if (t >= next_ping) {
      sendHello(atoi(argv[2]), argv[1]);
      next_ping = t + 5;
    }

    long long const next = (next_ping < next_sendrt) ? next_ping : next_sendrt;
    long long wait = next - now();
    if (wait < 0) wait = 0;


    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(mySock, &readfds);

    struct timeval tv;
    tv.tv_sec  = (wait > 5) ? 5 : (time_t)wait;
    tv.tv_usec = 0;


    int const selectValue = select(mySock+1, &readfds, NULL, NULL, &tv);


    if (selectValue < 0) {
      perror("select");
      continue;
    }


    if (FD_ISSET(mySock, &readfds)) {
      int const neighborPort = recvRoutingTable(mySock, &rt);
      if (neighborPort == EXIT_FAILURE) {
        fprintf(stderr, "[ROUTER] : Err recv entries\n");
        continue;
      }
      sendLinkUp(atoi(argv[2]),neighborPort-NO_BASE_PORT);
      display_routingTable(&rt);
    }
  }
  return 1;
}
