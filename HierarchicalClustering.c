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
    double dist; // mis en double car c'est mieux. est ce qu'on peut mettre unsigned ? 

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
        char *obj = (char *)llData(objects); // souci je veux la data de l'objet i mais je donne tous les objets
        BTree *tree = btCreate();
        btCreateRoot(tree, obj);
        // dicInsert(dico_clusters, obj, tree);
    }
    // fusion des clusters( tant qu'il n'y a pas k=1 cluster )
    while (llSize(paires) > 0)
    {
        Paire *minPair = (Paire *)llData(paires); // paire la plus proche distance zéro je crois ? 

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

        double *distPtr = malloc(sizeof(double));

        *distPtr = minPair->dist;
        // fusionner
        btMergeTrees(T_big, T_small, distPtr); // j'ai changé avant ça menait a un segfault si on mettait la distance en brut

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
    if (!node || !tree || !leaves)
        return; // safety

    if (btIsExternal(tree, node))
    {
        // c'est une feuille, on ajoute le nom de l'objet
        llInsertLast(leaves, btGetData(tree, node));
    }
    else
    {
        // c'est un noeud interne, on descend
        collectLeaves(tree, btLeft(tree, node), leaves);
        collectLeaves(tree, btRight(tree, node), leaves);
    }
}

static void hclustGetClustersDistRec(BTree *dendrogramme, BTNode *node, double distanceThreshold, List *clustersList)
{
    // TODO

    if (!node || dendrogramme || distanceThreshold || clustersList)
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
    List *clustersNodes = llCreateEmpty();
    llInsertFirst(clustersNodes, btRoot(hc->dendrogramme)); // on commence avec la racine (1 cluster)
    // TODO
    BTNode *maxNode;
    while (llSize(clustersNodes) < k)
    {
        // on cherche dans la liste le cluster qui a la plus grande distance ( le plus haut).
        maxNode = "cluster1dist">"cluster2dist" ? "cluster1" : "cluster2" ; // TODO  c'est du filler

        // si feuille on retire
        if ("c-est feuille")
            break;

        // on ajoute les deux children à droite et gauche de la liste
        llInsertLast(clustersNodes, btLeft(hc->dendrogramme, maxNode));
        llInsertLast(clustersNodes, btRight(hc->dendrogramme, maxNode));
        // on recommence
    }
    // retourner le resultat sous forme de liste.
    List *result = llCreateEmpty();
    return result;
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

static int hclustDepthRec(Hclust *hc, int depth, BTNode *node) // node doit etre un pointeur
{
    // TODO c'est du caca ce que j'ai fait je crois 
    BTNode *left = btLeft(hc->dendrogramme, node)   ;
    BTNode *right = btRight(hc->dendrogramme, node) ; 

    return 1 + max(hclustDepthRec(hc,depth+1,left),hclustDepthRec(hc,depth+1,right));
}

int hclustDepth(Hclust *hc)
{
    int depth = 0;
    BTNode *root = btRoot(hc->dendrogramme);
    if (!root)
        return depth;
    depth = hclustDepthRec(hc, depth + 1, root);
    return depth;
}

int hclustNbLeaves(Hclust *hc)
{
    return hc->nombre_objets; // note : lors de la fabrication de hc on devra incrémenter nombre_objets
}

void hclustPrintTreeRec(FILE *fp, BTree *tree, BTNode *node)
{
    if ("c-est une feuille")
    { // TODO : trouver la condition
        fprintf(fp, "%s", (char *)btGetData(tree, node));
    }
    else
    {
        // autre cas que juste une feuille, on met les parenthèses qu'il faut aux bons endroits
        fprintf(fp, "(");
        hclustPrintTreeRec(fp, tree, btLeft(tree, node));
        fprintf(fp, ",");
        hclustPrintTreeRec(fp, tree, btRight(tree, node));
        fprintf(fp, ")");

        // récupérer la distance
        double *d = (double *)btGetData(tree, node); // cast en double parce que getData return void
        fprintf(fp, ":%.3f", *d);
    }
}
void hclustPrintTree(FILE *fp, Hclust *hc)
{
    hclustPrintTreeRec(fp, hc->dendrogramme, btRoot(hc->dendrogramme));
    fprintf(fp, ";\n"); // finir le newick par un ";" dans l'exemple du cours
}
