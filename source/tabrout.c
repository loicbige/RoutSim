#include <stdio.h>
#include <string.h>

#include "../Lib/tabrout.h"

#include <stdlib.h>


void init_routingTable(routingTable * rt, char * fileConfig) {
    FILE *fc = fopen(fileConfig, "r");
    if (fc == NULL) {
        perror("[CONFIG] : Error opening configuration file.");
        exit(EXIT_FAILURE);
    }

    printf("\n[ROUTEUR] : Loading Configuration intiale\n");
    fscanf(fc, "%hud", &(rt->nb_entry));

    for (int i = 0; i < rt->nb_entry && !feof(fc); i++) {
        unsigned short int type;
        fscanf(fc, "%s %u %u %hud", rt->entries[i].destination,&rt->entries[i].port, &rt->entries[i].weight, &type);
        rt->entries[i].type = type;
    }

    fclose(fc);
}

void add_routingTable(routingTable *rt, char *destination, unsigned int port, unsigned int weight, nodeType type)
{
    if (rt->nb_entry == NB_MAX_ENTRY) {
        fprintf(stderr, "[CONFIG] : Too many routes.\n");
        return;
    }
    strcat(rt->entries[rt->nb_entry].destination, destination );
    rt->entries[rt->nb_entry].type = type;
    rt->entries[rt->nb_entry].port = port;
    rt->entries[rt->nb_entry].weight = weight;
    rt->nb_entry++;
}


void display_routingTable(routingTable *rt) {
    for (int i = 0; i < rt->nb_entry; i++) {
        int type = rt->entries[i].type;
        printf("[%s]:%d weight : %d & type=%d\n",rt->entries[i].destination,rt->entries[i].port, rt->entries[i].weight, type);

    }
    printf("\n");

}

