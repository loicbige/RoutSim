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
#define NO_BASE_PORT 17900

void createRoutingTable(routing_table_t *myRoutingTable, const char *adr, const char *id)
{

  char idInitConfigFile[20]; // Id of the configuration file of the router
  char myId[32];

  /* Building ID Router from command args */
  sprintf(myId, "R%s %s", id, adr);
  printf("ROUTEUR : %s\n", myId);
  // printf("construction id fichier\n");
  /* Building Config File ID from command args */
  sprintf(idInitConfigFile, "R%sCfg", id);
  strcat(idInitConfigFile, ".txt");
  // printf("\n Nom fichier Configuration : %s",idInitConfigFile);
  /* Loading My Routing Table from Initial Config file */
  init_routing_table(myRoutingTable, idInitConfigFile);
  printf("ROUTEUR : %d entrées initialement chargées \n", myRoutingTable->nb_entry);
  display_routing_table(myRoutingTable, myId);
}

int main(int argc, char const *argv[])
{
  if (argc != 4)
  {
    fprintf(stderr, "usage %s : <Routeur Adresse> <routeur id> <neighbor id>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  routing_table_t rt;

  createRoutingTable(&rt, argv[1], argv[2]);

  /* MY ROUTER*/

  struct sockaddr_in mySockAdr;
  memset(&mySockAdr, 0, sizeof(mySockAdr));

  mySockAdr.sin_family = AF_INET;
  mySockAdr.sin_port = htons(NO_BASE_PORT + atoi(argv[2]));

  if (inet_pton(AF_INET, LOCALHOST, &mySockAdr.sin_addr) <= 0)
  {
    perror("inet_pton adr src");
    exit(2);
  }

  /* NEIGHBOR ROUTER */

  struct sockaddr_in neighborSockAdr;
  memset(&neighborSockAdr, 0, sizeof(neighborSockAdr));

  neighborSockAdr.sin_family = AF_INET;
  neighborSockAdr.sin_port = htons(NO_BASE_PORT + atoi(argv[3]));
  socklen_t lenNeighborSocket = sizeof(neighborSockAdr);

  if (inet_pton(AF_INET, LOCALHOST, &neighborSockAdr.sin_addr) <= 0)
  {
    perror("inet_pton neighbor");
    exit(3);
  }

  switch (fork())
  {
  case -1:
    perror("fork");
    exit(5);
  case 0:
  {
    sleep(5);
    int transmiterSock = socket(AF_INET, SOCK_DGRAM, 0);

    if (transmiterSock == -1)
    {
      perror("socket EM");
      exit(6);
    }

    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);

    sprintf(buffer, "%d", rt.nb_entry);
    int nbSent = sendto(transmiterSock, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&neighborSockAdr, sizeof(neighborSockAdr));
    if (nbSent <= 0)
    {
      perror("sendto nbentry");
      shutdown(transmiterSock, SHUT_RDWR);
      exit(7);
    }
    printf("[ENVOI] : Annonce de %s routes vers le R%d sur le port %d...\n", buffer, atoi(argv[3]), ntohs(neighborSockAdr.sin_port));
    for (int entry = 0; entry < rt.nb_entry; entry++)
    {
      memset(buffer, 0, BUF_SIZE);
      sprintf(buffer, "%s", rt.tab_entry[entry]);
      nbSent = sendto(transmiterSock, buffer, strlen(buffer) + 1, 0, (struct sockaddr *)&neighborSockAdr, sizeof(neighborSockAdr));
      if (nbSent <= 0)
      {
        perror("send for loop transmiter");
      }
    }
    printf("[ENVOI] : La table a été transmise avec succés.\n");
    shutdown(transmiterSock, SHUT_RDWR);
    exit(EXIT_SUCCESS);
  }
  default:
  {
    int receiverSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (receiverSock == -1)
    {
      perror("receiver sock");
      exit(7);
    }

    if (bind(receiverSock, (struct sockaddr *)&mySockAdr, sizeof(mySockAdr)) == -1)
    {
      perror("bind");
      shutdown(receiverSock, SHUT_RDWR);
      exit(8);
    }
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE);

    int nbRecv = recvfrom(receiverSock, buffer, BUF_SIZE, 0, (struct sockaddr *)&neighborSockAdr, &lenNeighborSocket);
    if (nbRecv <= 0)
    {
      perror("Recvform : nbentry");
      exit(9);
    }
    // Il serait intéressant pour le robustesse du code d'ajouter la Primitive SO_RCVTIMEO pour time_out le processus si il ne reçoit rien au bout de x secondes et éviter qu'il bloque indéfiniement

    printf("[ROUTEUR] : En attente d'annonce des %s routes transmit par R%d sur le port : %d ...\n", buffer, atoi(argv[3]), ntohs(neighborSockAdr.sin_port));

    int incommingTableSize = 0;
    sscanf(buffer, "%d", &incommingTableSize);
    for (int entry = 0; entry < incommingTableSize; entry++)
    {
      memset(buffer, 0, BUF_SIZE);

      nbRecv = recvfrom(receiverSock, buffer, BUF_SIZE, 0, (struct sockaddr *)&neighborSockAdr, &lenNeighborSocket);
      if (nbRecv <= 0)
      {
        perror("recvfrom : for loop");
        continue; // pas necessaire de stopper le programme. On peut continuer à distribuer le reste de la table
      }

      if (!is_present_entry_table(&rt, buffer))
      {
        add_entry_routing_table(&rt, buffer);
        printf("[ROUTEUR] : %s -> ajouté à la table\n", buffer);
      }
      else
      {
        printf("[ROUTEUR] : %s -> ignoré\n", buffer);
      }
    }
    wait(NULL); // Il faut attendre que le proccessus ai tout envoyer avant de recevoir les informations de l'autre routeur.

    shutdown(receiverSock, SHUT_RDWR);

    printf("[ROUTEUR] : Toutes les annonces ont été reçue. \n");
    const char *myId = argv[2];

    display_routing_table(&rt, (char *)myId);
    exit(EXIT_SUCCESS);
  }
  }

  return 1;
}
