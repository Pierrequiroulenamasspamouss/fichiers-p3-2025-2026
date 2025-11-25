#include <stdio.h>
#include <stdlib.h>
#include "HierarchicalClustering.h"

typedef struct Hclust_t Hclust;

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams);

List *hclustGetClustersDist(Hclust *hc, double distanceThreshold);

List *hclustGetClustersK(Hclust *hc, int k);

BTree *hclustGetTree(Hclust *hc);

void hclustFree(Hclust *hc){
    free(hc);
    return;
}

int hclustDepth(Hclust *hc){
    return 0 ;
}

int hclustNbLeaves(Hclust *hc){
    return 0;
}

void hclustPrintTree(FILE *fp, Hclust *hc){
    return;
}

