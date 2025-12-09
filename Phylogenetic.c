#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "Phylogenetic.h"
#include "Dict.h"

#define MAXLINELENGTH 2000

typedef struct ParamFeatures_t
{
    Dict *dict;
} ParamFeatures;

// Wrapper to bridge the generic distance function with the specific DNA dictionary
static double DnaDistanceWrapper(const char *name1, const char *name2, void *param)
{
    ParamFeatures *p = (ParamFeatures *)param;
    char *dna1 = (char *)dictSearch(p->dict, name1);
    char *dna2 = (char *)dictSearch(p->dict, name2);

    // safety first
    if (!dna1 || !dna2)
        return 1.0;

    return phyloDNADistance(dna1, dna2);
}

double phyloDNADistance(char *dna1, char *dna2)
{
    unsigned long longueur = 0;
    double Transition = 0;
    double Transversion = 0;

    // safety first
    size_t len1 = strlen(dna1);
    size_t len2 = strlen(dna2);

    if (len1 < len2)
        longueur = len1;
    else
        longueur = len2;

    for (unsigned long i = 0; i < longueur; i++)
    {
        char a = dna1[i];
        char b = dna2[i];
        if (a == b)
            continue;

        if ((a == 'A' && b == 'G') || (b == 'A' && a == 'G') ||
            (a == 'T' && b == 'C') || (b == 'T' && a == 'C'))
        {
            Transition += 1;
        }
        else
        {
            Transversion += 1;
        }
    }

    // calculer les proportions de P et Q parce que c'est important
    double P = Transition / longueur;
    double Q = Transversion / longueur;

    // faire en plusieurs etapes plutot qu'un calcul
    double terme1 = 1.0 - 2.0 * P - Q;
    double terme2 = 1.0 - 2.0 * Q;

    
    if (terme1 <= 0.0000001)
        terme1 = 0.0000001;
    if (terme2 <= 0.0000001)
        terme2 = 0.0000001;

    double distance = -0.5 * log(terme1) - 0.25 * log(terme2);

    // pas de distance negative
    return (distance < 0) ? 0.0 : distance;
}

Hclust *phyloTreeCreate(char *filename)
{
    char buffer[MAXLINELENGTH];
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        return NULL;
    }

    // passer si y'a un header, comme dans l'exemple
    if (!fgets(buffer, MAXLINELENGTH, fp))
    {
        fclose(fp);
        return NULL;
    }

    List *names = llCreateEmpty();
    Dict *dicfeatures = dictCreate(1000);

    while (fgets(buffer, MAXLINELENGTH, fp))
    {
        size_t lenstr = strlen(buffer);
        // retirer les \n comme dans l'exemple
        if (lenstr > 0 && buffer[lenstr - 1] == '\n')
            buffer[--lenstr] = '\0';

        if (lenstr == 0)
            continue; // si y'a des lignes vides

        // extraire les noms
        int i = 0;
        while (buffer[i] != ',' && buffer[i] != '\0')
            i++;

        buffer[i] = '\0'; // Cut string at the comma

        char *objectName = malloc((strlen(buffer) + 1) * sizeof(char));
        if (!objectName)
            break;
        strcpy(objectName, buffer);

        llInsertLast(names, objectName);

        // on extrait l'ADN
        char *dnaSequence = NULL;
        // check si y'ade l'ADN apres la virgule
        if (i < (int)lenstr)
        {
            // buffer + i + 1 points apres la virgule
            dnaSequence = malloc((strlen(buffer + i + 1) + 1) * sizeof(char));
            if (dnaSequence)
                strcpy(dnaSequence, buffer + i + 1);
        }
        else
        {
            // si y'a un souci avec le décodage
            dnaSequence = malloc(sizeof(char));
            if (dnaSequence)
                *dnaSequence = '\0';
        }

        dictInsert(dicfeatures, objectName, dnaSequence);
    }

    ParamFeatures parameters;
    parameters.dict = dicfeatures;

    printf("Construction of the phylogenetic tree\n");

    Hclust *hc = hclustBuildTree(names, DnaDistanceWrapper, &parameters);

    dictFreeValues(dicfeatures, free);
    llFree(names);

    fclose(fp);
    return hc;
}