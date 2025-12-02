#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Phylogenetic.h"
#include "Dict.h"

// exactement le nombre de caractere utilisé dans main-feature
#define MAXLINELENGTH 2000

double phyloDNADistance(char *dna1, char *dna2)
{
    unsigned long longeur = 0;
    unsigned long P = 0;
    unsigned long Q = 0;

    if (strlen(dna1) < strlen(dna2))
        longeur = strlen(dna1);
    else
        longeur = strlen(dna2);

    for (unsigned long i = 0; i < longeur; i++)
    {
        char a = dna1[i];
        char b = dna2[i];
        if (a == b)
            continue;
        if ((a == 'A' || b == 'A') && (b == 'G' || a == 'G'))
        {
            Q++;
            continue;
        }
        if ((a == 'C' || b == 'C') && (b == 'T' || a == 'T'))
        {
            Q++;
            continue;
        }
        P++;
    }

    double distance = ((-1 / 2) * log(1 - 2 * P - Q)) - ((1 / 4) * log(1 - 2 * Q)); // log == ln ?
    return distance;
}

Hclust *phyloTreeCreate(char *filename)
{
    char buffer[MAXLINELENGTH];
    FILE *fp = fopen(filename, "r");
    fgets(buffer, MAXLINELENGTH, fp);
    int nbInfo = 2;
    // on nous le dit il y a 1 nom d'espece et un ADN s'aparé par une ',' donc 2 info par espece

    List *names = llCreateEmpty();
    Dict *dicfeatures = dictCreate(1000);

    while (fgets(buffer, MAXLINELENGTH, fp))
    {
        int lenstr = strlen(buffer) - 1;
        buffer[lenstr] = '\0'; // replace \n with \0

        // jusqu'a le premier ',' (nom)
        int i = 0;
        while (buffer[i] != ',')
            i++;
        buffer[i] = '\0';

        // gestion du nom
        char *objectName = malloc((i + 1) * sizeof(char));
        strcpy(objectName, buffer);
        llInsertLast(names, objectName);

        double *featureVector = malloc(nbInfo * sizeof(double));
        int pos = 0;
        i++;
        // la construction suivante est adapté a dictINseret que je n'ai pas ecrite
        // elle doit surement etre juste
        while (i < lenstr && pos < nbInfo)
        {
            int j = i;
            while (j < lenstr && buffer[j] != ',')
                j++;
            if (buffer[j] == ',')
                buffer[j] = '\0';
            featureVector[pos] = atof(buffer + i);
            pos++;
            i = j + 1;
        }
        dictInsert(dicfeatures, objectName, featureVector);
    }
    Hclust *hc = hclustBuildTree(names, phyloDNADistance, fp);
    // free the memory
    dictFreeValues(nbInfo, free);
    llFreeData(names);
    fclose(fp);

    return hc;
}

// j'ai enlevé les sercurité pour y voir plus claire
// mais ca semblait etre que du copier coller :/ reverifie au cas ou
