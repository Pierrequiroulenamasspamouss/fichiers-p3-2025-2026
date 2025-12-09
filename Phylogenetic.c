#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Phylogenetic.h"
#include "Dict.h"

// exactement le nombre de caractere utilisé dans maihclustBuildTreen-feature
#define MAXLINELENGTH 2000
static double DnaDistanceWrapper(const char *dna1, const char *dna2, void *useless)
{
    return phyloDNADistance(dna1, dna2);
}
double phyloDNADistance(char *dna1, char *dna2)
{
    printf("%s",dna1);
    unsigned long longeur = 0;
    int Transition = 0;
    int Transversion = 0;

    if (strlen(dna1) < strlen(dna2))
        longeur = strlen(dna1);
    else
        longeur = strlen(dna2);
    //printf("%lu\n", longeur);

    for (unsigned long i = 0; i < longeur; i++)
    {
        char a = dna1[i];
        char b = dna2[i];
        if (a == b)
            continue;

        if ((a == 'A' && b == 'G') || (b == 'A' && a == 'G') || (a == 'T' && b == 'C') || (b == 'T' && a == 'C'))
        {
            Transition += 1;
            continue;
        }
        else
            Transversion++;
    }
    //printf(" %d\n", Transition);

    //double distance = ((-1.0 / 2.0) * log(1 - 2.0 * P - Q)) - ((1.0 / 4.0) * log(1 - 2.0 * Q)); // log == ln ?
    // printf("%f\n",distance);
    return 0;//distance;
}

Hclust *phyloTreeCreate(char *filename)
{

    char buffer[MAXLINELENGTH];
    char buffer2[MAXLINELENGTH];
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
        {
            i++;
        }
        while (buffer[i] != '\0')
        {

            buffer2[i] = buffer[i];
            i++;
        }
        buffer2[i] = '\0';
        buffer[i] = '\0';
        strcpy(buffer, buffer2);
        // gestion du nom
        char *objectName = malloc((i + 1) * sizeof(char));
        strcpy(objectName, buffer2);
        llInsertLast(names, objectName);

        // char *objectDNA = malloc((i + 1) * sizeof(char));
        // strcpy(objectDNA, buffer);
        // llInsertLast(names, objectName);

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
   Hclust *hc = hclustBuildTree(names, DnaDistanceWrapper,buffer2);
    // free the memory
    llFreeData(names);
    fclose(fp);
    
    return hc;
}

// j'ai enlevé les sercurité pour y voir plus claire
// mais ca semblait etre que du copier coller :/ reverifie au cas ou
