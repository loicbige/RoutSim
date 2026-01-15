#include <stdio.h>

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
        uint8_t type;
        fscanf(fc, "%s %hu %hu %hhu", rt->entries[i].destination,&rt->entries[i].port, &rt->entries[i].weight, &type);
        rt->entries[i].type = type;
    }

    fclose(fc);
}

void add_routingTable(routingTable *rt,char *entry)
{
    if (rt->nb_entry == NB_MAX_ENTRY) {
        fprintf(stderr, "[CONFIG] : Too many routes.\n");
        return;
    }
    uint8_t type;
    sscanf(entry, "%s %hu %hu %hhu",
        rt->entries[rt->nb_entry].destination,
        &rt->entries[rt->nb_entry].port,
        &rt->entries[rt->nb_entry].weight,
        &type);
    rt->entries[rt->nb_entry].type = type;
    rt->nb_entry++;
}


void display_routingTable(routingTable *rt) {
    for (int i = 0; i < rt->nb_entry; i++) {
        uint8_t type = rt->entries[i].type;
        printf("[%s]:%hu weight : %hu & type=%hhu\n",rt->entries[i].destination,rt->entries[i].port, rt->entries[i].weight, type);

    }
    printf("\n");

}

