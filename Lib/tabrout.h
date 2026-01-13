#ifndef __TABROUT_H__
#define __TABROUT_H__

#define NB_MAX_ENTRY 10
#include <stdbool.h>

// Routing Table
// ===============
typedef struct {
    unsigned short int nb_entry;
    char * tab_entry[NB_MAX_ENTRY];
} routing_table_t;


/* ===================================================== */
/* UPLOAD FROM CONFIGURATION FILE                        */
/* Fonction qui initialise la Table de Routage initiale  */
/* par lecture d'un fichier de configuration initiale    */
/* dont l'identifiant est passé en argument              */
void init_routing_table(routing_table_t * rt, char * fileConfig);

/* ===================================================== */
/* DISPLAY CURRENT ROUTING TABLE CONTENT                 */
/* Fonction qui affiche à l'écran la Table de Routage    */
/* du routeur dont l'identifiant est précisé             */
void display_routing_table(routing_table_t * rt, char * id_router);


/* ===================================================== */
/* ADD ONE ENTRY                                         */
/* Fonction qui rajoute une entrée en fin de la Table de */
/* routage                                               */
void add_entry_routing_table(routing_table_t * rt, char * entry);

/* IS ALREADY PRESENT AN ENTRY ?                          */
/* Fonction qui détecte si une entrée donnée existe déjà  */
/* dans la Table de routage                               */
bool is_present_entry_table(routing_table_t * rt, char * entry);

#endif
