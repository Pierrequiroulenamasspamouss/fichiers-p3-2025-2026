// gcc -o clusteranimal main_animal.c BTree.c Dict.c LinkedList.c HierarchicalClustering.c

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Dict.h"
#include "LinkedList.h"
#include "BTree.h"
#include "HierarchicalClustering.h"

#define MAXLINELENGTH 2000

typedef struct ParamFeatures_t
{
    Dict *dict;
    int nbFeatures;
} ParamFeatures;

static double euclideanDistance(const char *obj1, const char *obj2, void *param)
{
    ParamFeatures *pfparam = param;
    Dict *d = pfparam->dict;
    int nbf = pfparam->nbFeatures;

    double *feat1 = dictSearch(d, obj1);
    double *feat2 = dictSearch(d, obj2);
    double sum = 0.0;

    for (int i = 0; i < nbf; i++)
    {
        double diff = (feat1[i] - feat2[i]);
        sum += diff * diff;
    }

    return sqrt(sum);
}

static Hclust *FeatureTreeCreate(char *filename)
{
    char buffer[MAXLINELENGTH];

    FILE *fp = fopen(filename, "r");

    if (!fgets(buffer, MAXLINELENGTH, fp))
    {
        fprintf(stderr, "FeatureTreeCreate: the file is empty.\n");
        exit(EXIT_FAILURE);
    }

    // Count the number of features from the header
    int nbFeatures = 0;
    int i = 0;
    while (buffer[i] != '\n' || buffer[i] == '\0')
    {
        if (buffer[i] == ',')
            nbFeatures++;
        i++;
    }

    // collect the data
    List *names = llCreateEmpty();
    Dict *dicfeatures = dictCreate(1000);

    while (fgets(buffer, MAXLINELENGTH, fp))
    {
        int lenstr = strlen(buffer) - 1;
        buffer[lenstr] = '\0'; // replace \n with \0

        // Extract species name
        int i = 0;
        while (buffer[i] != ',')
            i++;
        buffer[i] = '\0';

        char *objectName = malloc((i + 1) * sizeof(char));
        strcpy(objectName, buffer);

        llInsertLast(names, objectName);

        double *featureVector = malloc(nbFeatures * sizeof(double));
        int pos = 0;
        i++;
        while (i < lenstr && pos < nbFeatures)
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
        if (pos != nbFeatures)
        {
            fprintf(stderr, "FeatureTreeCreate: not enough features for object %s.\n", objectName);
            exit(EXIT_FAILURE);
        }
        dictInsert(dicfeatures, objectName, featureVector);
    }

    printf("%zu objects read, with %d features\n", llLength(names), nbFeatures);

    ParamFeatures pf;
    pf.nbFeatures = nbFeatures;
    pf.dict = dicfeatures;

    printf("Construction of the phylogenetic tree\n");

    Hclust *hc = hclustBuildTree(names, euclideanDistance, &pf);

    // free the memory
    dictFreeValues(pf.dict, free);
    llFreeData(names);
    fclose(fp);

    return hc;
}

int main(int argc, char *argv[])
{

    if (argc < 4)
    {
        fprintf(stderr, "Not enough arguments.\n"
                        "Usage: hcfeatures (-th <threshold> | -k <num_clusters>) "
                        "<input_file> [<output_file>]\n");
        exit(0);
    }

    double threshold = 0.0;
    int num_clusters = 0;
    int use_threshold = 1;
    char *ifile = NULL;

    if (strcmp(argv[1], "-th") == 0)
    {
        threshold = atof(argv[2]);
        use_threshold = 1;
        ifile = argv[3];
    }
    else if (strcmp(argv[1], "-k") == 0)
    {
        num_clusters = atoi(argv[2]);
        use_threshold = 0;
        ifile = argv[3];
    }
    else
    {
        fprintf(stderr, "Invalid option.\n"
                        "Usage: hcfeatures (-th <threshold> | -k <num_clusters>) "
                        "<input_file> [<output_file>]\n");
        exit(0);
    }

    Hclust *hc = FeatureTreeCreate(ifile);

    // print the clusters
    List *clusters = NULL;

    if (use_threshold)
    {
        clusters = hclustGetClustersDist(hc, threshold);
        printf("Clusters (%zu) with a distance threshold of %f:\n",
               llLength(clusters), threshold);
    }
    else
    {
        clusters = hclustGetClustersK(hc, num_clusters);
        printf("Clusters for k=%d:\n", num_clusters);
    }

    Node *p = llHead(clusters);
    int i = 1;
    while (p != NULL)
    {
        printf("- Cluster %d (size=%zu)", i, llLength(llData(p)));
        Node *first = llHead(llData(p));
        Node *pp = first;
        while (pp != NULL)
        {
            if (pp == first)
                printf(": %s", (char *)llData(pp));
            else
                printf(", %s", (char *)llData(pp));
            pp = llNext(pp);
        }
        printf("\n");
        i++;
        p = llNext(p);
    }

    FILE *foutput;
    if (argc == 5)
    {
        printf("Outputing the tree in file %s.\n", argv[4]);
        foutput = fopen(argv[4], "w");
    }
    else
    {
        printf("The tree in Newick format:\n");
        foutput = stdout;
    }

    hclustPrintTree(foutput, hc);

    for (Node *i = llHead(clusters); i != NULL; i = llNext(i))
    {
        List *inner = llData(i);
        llFree(inner);
    }
    llFree(clusters);
    hclustFree(hc);
    if (foutput != stderr)
        fclose(foutput);

    exit(0);
}
