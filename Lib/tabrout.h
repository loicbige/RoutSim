#ifndef __TABROUT_H__
#define __TABROUT_H__

#include <stdint.h>
#include <stdbool.h>
#define NB_MAX_ENTRY 30
#define MAX_IP_SIZE 32

// Routing Table
// ===============
typedef enum {TERMINAL=0,ROUTER=1} nodeType;


typedef struct s_Entry {
    char destination[MAX_IP_SIZE];
    char nextHop[MAX_IP_SIZE];
    uint16_t port;
    uint16_t weight;
    nodeType type;
}entry;

typedef struct {
    unsigned short int nb_entry;
    entry entries[NB_MAX_ENTRY];
} routingTable;


// OPERATOR :

void init_routingTable(routingTable * rt, char * fileConfig);
int add_routingTable(routingTable *rt,char *entry);
void display_routingTable(routingTable *rt);
bool isInRoutingTable(routingTable *rt,char *entry);
bool searchInRoutingTable(routingTable *rt, char*entry);
#endif
