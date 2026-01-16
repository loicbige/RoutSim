#include <stdio.h>

#include "../Lib/tabrout.h"

#include <stdlib.h>
#include <string.h>


void init_routingTable(routingTable * rt, char * fileConfig) {
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
bool isMyRoutingAdress(const char* ip, const uint16_t port,const char *entry) {
    char dest[MAX_IP_SIZE];
    uint16_t destPort;
    uint16_t weight;
    uint8_t type;

    sscanf(entry, "%s %hu %hu %hhu", dest, &destPort, &weight, &type);

    return strcmp(ip, entry) == 0 && destPort == port;
}


bool isEntryEquals(entry *e,char *entry) {
    char dest[MAX_IP_SIZE];
    uint16_t destPort;
    uint16_t weight;
    uint8_t type;

    sscanf(entry, "%s %hu %hu %hhu", dest, &destPort, &weight, &type);
    return (strcmp(dest, e->destination) == 0 && e->port == destPort);
}

bool searchInRoutingTable(routingTable *rt, char *entry) {
    printf("[SEARCH] : Searching the entry %s in the routing table....\n", entry);
    for (int i = 0; i < rt->nb_entry; ++i) {
        printf("[SEARCH] : Compare with entry %d : %s %hu %hu %hhu\n", i,
            rt->entries[i].destination,
            rt->entries[i].port,
            rt->entries[i].weight,
            rt->entries[i].type);
        if (isEntryEquals(&rt->entries[i], entry)) {
            printf("[SEARCH] : Found entry %s in the routing table\n", entry);
            return true;
        }
    }
    return false;
}

int add_routingTable(routingTable *rt,char *entry)
{
    uint8_t nbEntries = rt->nb_entry;

    if (nbEntries >= NB_MAX_ENTRY) {
        fprintf(stderr, "[ADD] : Too many routes.\n");
        return EXIT_FAILURE;
    }
    if (searchInRoutingTable(rt, entry) == false) {
        uint8_t type;
        printf("[ADD] : Adding entry %s in the routing table\n", entry);
        sscanf(entry, "%s %hu %hu %hhu",
            rt->entries[nbEntries].destination,
            &rt->entries[nbEntries].port,
            &rt->entries[nbEntries].weight,
            &type);
        rt->entries[nbEntries].type = type;
        rt->nb_entry++;
    } else {
        printf("[ADD] : Entry already exists.\n");
    }
    return EXIT_SUCCESS;
}


void displayEntry(entry *e) {
    uint8_t type = e->type;
    printf("[%s]:%hu weight : %hu & type=%hhu\n",
        e->destination,
        e->port,
        e->weight,
        type);
}

void display_routingTable(routingTable *rt) {
    for (int i = 0; i < rt->nb_entry; i++) {
        displayEntry(&rt->entries[i]);

    }
    printf("\n");

}

