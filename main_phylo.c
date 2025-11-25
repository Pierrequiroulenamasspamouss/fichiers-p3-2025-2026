#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "LinkedList.h"
#include "BTree.h"
#include "Phylogenetic.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "No file names provided.\nUsage: hcphylo <input_file> [<output_file>]\n");
        exit(0);
    }

    Hclust *hc = phyloTreeCreate(argv[1]);

    FILE *foutput;
    if (argc == 3)
    {
        printf("Outputing the tree in file %s.\n", argv[2]);
        foutput = fopen(argv[2], "w");
    }
    else {
        printf("Tree in Newick format:\n");
        foutput = stdout;
    }

    hclustPrintTree(foutput, hc);
    hclustFree(hc);
    if (foutput != stdout)
        fclose(foutput);

    exit(0);
}
