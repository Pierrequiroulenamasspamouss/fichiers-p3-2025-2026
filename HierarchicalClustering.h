#ifndef HCLUS_H
#define HCLUS_H

#include "LinkedList.h"
#include "BTree.h"

typedef struct Hclust_t Hclust;

/**
 * @brief Builds a hierarchical clustering. objects is a list of object names (char *).
 *        distFn is a function computing the distance between two objects. The names of
 *        objects are copied when stored into the structure and can be freed by the caller.
 *
 * @param objects the list of object names (char *)
 * @param distFn a function computing the distance between two objects
 * @param distFnParams a parameter of the distance function
 * @return Hclust* the hierarchical clustering
 */
Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams);

/**
 * @brief Frees the hierarchical clustering from memory.
 * 
 * @param hc the hierarchical clustering
 */
void hclustFree(Hclust *hc);

/**
 * @brief Returns the depth of the dendrogram (the maximum length of a path
 *        from the root to a leaf)
 * 
 * @param hc the hierarchical clustering 
 * @return int the depth of the dendrogram
 */
int hclustDepth(Hclust *hc);

/**
 * @brief Returns the number of leaves in the hierarchical clustering (ie., the number
 *        of objects).
 * 
 * @param hc the hierarchical clustering
 * @return int the number of leaves
 */
int hclustNbLeaves(Hclust *hc);

/**
 * @brief Prints the dendrogram in the file fp (ie., with the command fprintf(fp,...)) in
 *        Newick format (see Ecampus).
 * 
 * @param fp a pointer to the file
 * @param hc the hierarchical clustering
 */
void hclustPrintTree(FILE *fp, Hclust *hc);

/**
 * @brief Returns a list of lists, each containing the (names of the) objects
 *        contained in one of the clusters found by hierarchical clustering, when
 *        only clusters with a distance lower than distanceThreshold are merged.
 *        The caller must free the lists but not the object names in these lists.
 * 
 * @param hc 
 * @param distanceThreshold 
 * @return List* 
 */
List *hclustGetClustersDist(Hclust *hc, double distanceThreshold);

/**
 * @brief Returns a list of k lists, each containing the (names of the) objects
 *        contained in one of the clusters found by hierarchical clustering. The caller
 *        must free the lists but not the object names in these lists.
 * 
 * @param hc the hierarchical clustering
 * @param k the number of clusters one wants to find
 * @return List* a list of k lists of object names
 */
List *hclustGetClustersK(Hclust *hc, int k);

/**
 * @brief Returns a binary tree encoding for the dendrogram. The data at each interior node
 *        should be a pointer to a double containing the distance between the clusters represented
 *        by its left and right subtrees. The data at each leaf should be the object name (char *).
 *        The caller must not free the tree or its data.
 * 
 * @param hc the hierarchical clustering
 * @return BTree* the binary tree representing the dendrogram
 */
BTree *hclustGetTree(Hclust *hc);

#endif
