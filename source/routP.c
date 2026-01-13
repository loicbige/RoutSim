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
#define DASHBOARD_PORT 9999
#define DASHBOARD_IP "127.0.0.1"

void createRoutingTable(routingTable *myRoutingTable, const char *adr, const char *id)
{

  char idInitConfigFile[20]; // Id of the configuration file of the router
  char myId[32];

  /* Building ID Router from command args */
  sprintf(myId, "R%s %s", id, adr);
  printf("[ROUTEUR] : %s\n", myId);
  // printf("construction id fichier\n");
  /* Building Config File ID from command args */
  sprintf(idInitConfigFile, "config/rf%s", id);
  strcat(idInitConfigFile, ".txt");
  // printf("\n Nom fichier Configuration : %s",idInitConfigFile);
  /* Loading My Routing Table from Initial Config file */
  init_routingTable(myRoutingTable, idInitConfigFile);
  printf("[ROUTEUR] : %d entrées initialement chargées \n", myRoutingTable->nb_entry);
  display_routingTable(myRoutingTable);
}
sendtoDashboard(char *)
int main(int argc, char const *argv[])
{
  if (argc != 4) {
    fprintf(stderr, "Usage: %s <ip> <id> <port>\n", argv[0]);
  }
  routingTable rt;
  createRoutingTable(&rt, argv[1], argv[2]);

  return 1;
}
