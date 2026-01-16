#include <stdio.h>

#include "../Lib/tabrout.h"

#include <stdlib.h>
#include <string.h>


void init_routingTable(routingTable * rt, char *fileConfig) {
    FILE *fc = fopen(fileConfig, "r");
    if (fc == NULL) {
        perror("[CONFIG] : Error opening configuration file.");
        exit(EXIT_FAILURE);
    }
    memset(rt, 0, sizeof(routingTable));
    printf("\n[ROUTEUR] : Loading Configuration intiale\n");
    fscanf(fc, "%hhu", &(rt->nb_entry));

    char dest[MAX_IP_SIZE];

    for (int i = 0; i < rt->nb_entry && !feof(fc); i++) {
        uint8_t type;

        if (fscanf(fc, "%s %hu %hu %hhu",
            dest,
            &rt->entries[i].port,
            &rt->entries[i].weight,
            &type) != 4) {
            fprintf(stderr, "[CONFIG] : Error reading routing table.\n");
            exit(EXIT_FAILURE);
        }
        strncpy(rt->entries[i].destination, dest, MAX_IP_SIZE-1);
        rt->entries[i].destination[MAX_IP_SIZE - 1] = '\0';
        rt->entries[i].type = type;
    }

    fclose(fc);
}
bool isMyRoutingAdress(const char* ip, const uint16_t port,const entry *e) {
    return strcmp(ip, e->destination) == 0 && e->port == port;
}


bool isEntryEquals(const entry *e1,const entry *e2) {
   return strcmp(e1->destination, e2->destination) == 0 && e1->port == e2->port;
}

bool searchInRoutingTable(routingTable *rt,entry *e) {
    printf("[SEARCH] : Searching the entry ");
    displayEntry(e);
    printf("in the routing table....\n");
    for (int i = 0; i < rt->nb_entry; ++i) {
        printf("[SEARCH] : Compare with entry");
        displayEntry(&rt->entries[i]);
        printf("\n");
        if (isEntryEquals(&rt->entries[i], e)) {
            printf("[SEARCH] : Found entry ");
            displayEntry(&rt->entries[i]);
            printf(" in the routing table\n");
            return true;
        }
    }
    return false;
}

int add_routingTable(routingTable *rt,entry *e)
{
    const uint8_t nbEntries = rt->nb_entry;

    if (nbEntries >= NB_MAX_ENTRY) {
        fprintf(stderr, "[ADD] : Too many routes.\n");
        return EXIT_FAILURE;
    }
    if (!searchInRoutingTable(rt, e) && !isEntryEquals(e, &rt->entries[0])) {
        printf("[ADD] : Adding entry ");
        displayEntry(e);
        printf(" in the routing table\n");

        rt->entries[nbEntries] = *e;
        rt->nb_entry++;
    } else {
        printf("[ADD] : Entry already exists.\n");
    }
    return EXIT_SUCCESS;
}


void displayEntry(entry *e) {
    uint8_t type = e->type;
    printf("[%s]:%hu weight : %hu & type=%hhu",
        e->destination,
        e->port,
        e->weight,
        type);
}

void display_routingTable(routingTable *rt) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║");
    printf("ROUTING TABLE :");
    printf("                                         ║");
    printf("\n");
    printf("╚════════════════════════════════════════╣\n");
    for (int i = 0; i < rt->nb_entry; i++) {
        printf("║");
        displayEntry(&rt->entries[i]);
        printf("\n");

    }
    printf("╚════════════════════════════════════════╝\n");

    printf("\n");

}

