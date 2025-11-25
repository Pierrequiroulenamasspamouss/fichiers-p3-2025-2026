#ifndef PHYLOGENETIC_H
#define PHYLOGENETIC_H

#include "HierarchicalClustering.h"

/**
 * @brief Computes the distance between two DNA sequences as explained in the
 *        project description
 * 
 * @param dna1 the first DNA sequence
 * @param dna2 the second DNA sequence
 * @return double the distance between the sequences
 */
double phyloDNADistance(char *dna1, char *dna2);

/**
 * @brief Create a hierarchical clustering from the set of DNA sequences contained
 *        in the file filename.
 * 
 * @param filename the name of the file containing the sequences
 * @return Hclust* the hierarchical clustering
 */
Hclust *phyloTreeCreate(char *filename);

#endif
