#ifndef __TABROUT_H__
#define __TABROUT_H__

#define NB_MAX_ENTRY 30
#define MAX_IP_SIZE 16
#include <stdbool.h>

// Routing Table
// ===============
typedef enum {TERMINAL=0,ROUTER=1} nodeType;


typedef struct s_Entry {
    char destination[MAX_IP_SIZE];
    char nextHop[MAX_IP_SIZE];
    unsigned int port;
    unsigned int weight;
    nodeType type;
}entry;

typedef struct {
    unsigned short int nb_entry;
    entry entries[NB_MAX_ENTRY];
} routingTable;


// OPERATOR :

void init_routingTable(routingTable * rt, char * fileConfig);
void add_routingTable(routingTable *rt, char *destination, unsigned int port, unsigned int weight, nodeType type);
void display_routingTable(routingTable *rt);
#endif
