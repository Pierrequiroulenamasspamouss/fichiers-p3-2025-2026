#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "HierarchicalClustering.h"
#include "LinkedList.h"

typedef struct Hclust_t
{
    BTree *dendrogramme;
    List *liste_distances_fusion; // à voir si on s'en sert
    int nombre_objets;            // N
} Hclust;
typedef struct
{
    char *o1;
    char *o2;
    int dist; // pourrait être double

} Paire;

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
    // TODO
    int n = llSize(objects);
    if (n <= 1)
        return NULL; // vide

    BTree *dico_clusters = btCreate(); // ( nom_objet -> Sous-arbre du dendrogramme )
    List *paires = llCreateEmpty();    // Liste de ( objet1, objet2, distance )
    BTree *T_big;
    BTree *T_small;

    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            char *o1 = (char *)llGet(objects, i);
            char *o2 = (char *)llGet(objects, j);
            double dist = distFn(o1, o2, distFnParams);

            Paire *p = malloc(sizeof(Paire));
            p->o1 = o1;
            p->o2 = o2;
            p->dist = dist;
            llInsertLast(paires, p);
        }
    }
    // tri des paires ( important pour efficacité je crois )

    // llSort(paires); // faire pour tous les éléments de paires ( le faire d'une façon smart)

    // initialisation des clusters ( chaque objet est son propre cluster )
    for (int i = 0; i < n; i++)
    {
        char *obj = (char *)llData(objects[i]); // 
        BTree *tree = btCreate();
        btCreateRoot(tree, obj);
        // dicInsert(dico_clusters, obj, tree);
    }
    // fusion des clusters( tant qu'il n'y a pas k=1 cluster )
    while (llSize(paires) > 0)
    {
        Paire *minPair = (Paire *)llData(paires, 0); // paire la plus proche

        // trouver les sous-arbres ( aka clusters ) des objets
        BTree *T_o1 = dictSearch(dico_clusters, minPair->o1);
        BTree *T_o2 = dictSearch(dico_clusters, minPair->o2);

        if (!T_o1 || !T_o2 || T_o1 == T_o2)
        {
            llRemoveFirst(paires);
            continue;
        }
        // un ordre précis avec T_big et T_small pour optimiser
        if (btSize(T_o1) >= btSize(T_o2))
        {
            T_big = T_o1;
            T_small = T_o2;
        }
        else
        {
            T_big = T_o2;
            T_small = T_o1;
        };

        // fusionner
        btMergeTrees(T_big, T_small, minPair->dist); // 'dist' est le nouveau node dedans, possible mene a segfault

        // mettre à jour le dictionnaire
        btMapLeaves(T_big, btRoot(T_small), updateClusterDict(), dico_clusters); // NB updateClusterDict n'est pas défini

        // supprimer la paire traitee
        llRemoveFirst(paires);

        // verifier si on a atteint 1 seul sous-arbre
        if (llSize(dico_clusters) == 1)
            break;
    }

    // creer la structure Hclust a retourner
    Hclust *hc = malloc(sizeof(Hclust));
    hc->dendrogramme = T_big; // L'arbre final
    hc->nombre_objets = n;
    hc->liste_distances_fusion = paires;
    dicFree(dico_clusters);
    return hc;
}

static void collectLeaves(BTree *tree, BTNode *node, List *leaves)
{
    // TODO
    return;
}

static void hclustGetClustersDistRec(BTree *dendrogramme, BTNode *node, double distanceThreshold, List *clustersList)
{
    // TODO

    if (!node)
        return; // safety first

    double node_distance = (double)(uintptr_t)btGetData(dendrogramme, node);

    if (btIsExternal(dendrogramme, node))
    {
        // rajouter si c'est une feuille
        llInsertLast(clustersList, btGetData(dendrogramme, node));
        return;
    }

    // si la distance est <= au seuil --> c'est un cluster final
    if (node_distance <= distanceThreshold)
    {
        // lister toutes les feuilles de ce sous-arbre
        List *cluster = llCreateEmpty();
        collectLeaves(dendrogramme, node, cluster); // TODO : fausse fonction
        llInsertLast(clustersList, cluster);        // ajouter le cluster
        return;
    }
    // Sinon, continuer la descente recursivement
    if (btHasLeft(dendrogramme, node))
        hclustGetClustersDistRec(dendrogramme, btLeft(dendrogramme, node),
                                 distanceThreshold, clustersList);
    if (btHasRight(dendrogramme, node))
        hclustGetClustersDistRec(dendrogramme, btRight(dendrogramme, node),
                                 distanceThreshold, clustersList);
}

List *hclustGetClustersDist(Hclust *hc, double distanceThreshold)
{

    if (!hc || !hc->dendrogramme)
        return NULL;

    List *clustersList = llCreateEmpty();
    hclustGetClustersDistRec(hc->dendrogramme, btRoot(hc->dendrogramme),
                             distanceThreshold, clustersList);
    return clustersList;
}

List *hclustGetClustersK(Hclust *hc, int k)
{
    // TODO
}

BTree *hclustGetTree(Hclust *hc)
{
    return hc->dendrogramme;
}

void hclustFree(Hclust *hc)
{

    btFree(hc->dendrogramme);
    free(hc);
    return;
}

int hclustDepth(Hclust *hc)
{
    // TODO 
    return 0 ;
}

int hclustNbLeaves(Hclust *hc)
{
    return hc->nombre_objets; // note : lors de la fabrication de hc on devra incrémenter nombre_objets
}

static int hclustPrintTreeRec(FILE *fp, Hclust *hc)
{
    if (!hc|| hc == NULL)return;
    hclustPrintTreeRec(fp,hc);
    fprintf(fp, "hello");
}
void hclustPrintTree(FILE *fp, Hclust *hc)
{
    char s = "a";
    fprintf(fp,"%d\n",s);


    // etape 1 ) commencer par une feuille -> prendre parent et le rajouter. 
    
    return;
}
