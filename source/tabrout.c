
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Lib/tabrout.h"

/* ===================================================== */
/* UPLOAD FROM CONFIGURATION FILE                        */
/* Fonction qui initialise la Table de Routage initiale  */
/* par lecture d'un fichier de configuration initiale    */
/* dont l'identifiant est passé en argument              */
void init_routing_table(routing_table_t * rt, char * fileConfig) {
  FILE *fichier = NULL;
  char ligne[64];
  int i=0;

  fichier = fopen(fileConfig, "rt");
  if (fichier == NULL) {
    perror("[Config] Error opening configuration file.\n");
    exit(EXIT_FAILURE);
  }

  printf("\nROUTEUR : Loading Configuration intiale\n");
  while (!feof(fichier)) {
      // read line
      rt->tab_entry[i] = (char *) malloc(sizeof(ligne));
      fgets(ligne, sizeof(ligne), fichier);
      ligne[strlen(ligne)-1]='\0';
      strcpy(rt->tab_entry[i],ligne);
      //printf("\t %d",i);
      i++;// remove '\n'
      }
  // ending string array with an empty string
  rt->nb_entry = i-1;
  //printf("\t NB final : %d\n",rt->nb_entry);
  fclose(fichier);
  return ;
}

/* ===================================================== */
/* DISPLAY CURRENT ROUTING TABLE CONTENT                 */
/* Fonction qui affiche à l'écran la Table de Routage    */
/* du routeur dont l'identifiant est précisé             */

void display_routing_table(routing_table_t * rt, char * id_router) {
  printf("\nROUTEUR : ETAT courant de la TABLE de ROUTAGE de %s",id_router);
  printf("\nROUTEUR : NB d'ENTREES : %d",rt->nb_entry);
  for (int i=0; i<rt->nb_entry; i++)
    printf("\n\t%s",rt->tab_entry[i]);
  printf("\n");
}

/* ===================================================== */
/* ADD ONE ENTRY                                         */
/* Fonction qui rajoute une entrée en fin de la Table de */
/* routage                                               */

void add_entry_routing_table(routing_table_t * rt, char * entry) {
  rt->tab_entry[rt->nb_entry] = (char *)malloc(strlen(entry)+1);
  strcpy(rt->tab_entry[rt->nb_entry], entry);
  rt->nb_entry++;
}

/* IS ALREADY PRESENT AN ENTRY ?                          */
/* Fonction qui détecte si une entrée donnée existe déjà  */
/* dans la Table de routage                               */

bool is_present_entry_table(routing_table_t * rt, char * entry) {
  for (int i=0; i<rt->nb_entry; i++) {
    if (strcmp(rt->tab_entry[i],entry) == 0) return(true);
  }
  return(false);
}
