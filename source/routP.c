#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h> // struct sockaddr_in
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#include "../Lib/tabrout.h"

#define BUF_SIZE 64        // we should receive less...
#define IPV4_ADR_STRLEN 16 // == INET_ADDRSTRLEN
#define LOCALHOST "127.0.0.1"
#define DASHBOARD_PORT 8080
#define DASHBOARD_IP "127.0.0.1"

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
    perror("dahs socket");
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
int main(int argc, char const *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <ip> <id> <port>\n", argv[0]);
  }
  routingTable rt;
  createRoutingTable(&rt, argv[1], argv[2]);
  display_routingTable(&rt);
  char message[BUF_SIZE];
  sprintf(message, "\"ip\": \"%s\"", argv[1]);
  sendtoDashboard("HELLO", message, atoi(argv[2]));

  return 1;
}
