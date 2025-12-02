#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "HierarchicalClustering.h"
#include "LinkedList.h"
#include "Dict.h"

typedef struct Hclust_t
{
    BTree *dendrogramme;
    List *liste_distances_fusion; // à voir si on s'en sert
    int nombre_feuilles;          // N
} Hclust;
typedef struct
{
    char *o1;
    char *o2;
    double dist; // mis en double car c'est mieux. est ce qu'on peut mettre unsigned ?

} Paire;
static int comparePaires(Paire *a, Paire *b)
{
    if (a->dist < b->dist)
        return -1;
    if (a->dist > b->dist)
        return 1;
    return 0;
}

static void updateClusterDict(BTree *tree, BTNode *node, Dict *dico, BTree *survivorTree)
{
    if (!node) return;

    if (btIsExternal(tree, node))
    {
        char *key = (char *)btGetData(tree, node);
        // on met à jour la valeur dans le dico pour pointer vers l'arbre survivant
        dictInsert(dico, key, survivorTree); 
    }
    else
    {
        updateClusterDict(tree, btLeft(tree, node), dico, survivorTree);
        updateClusterDict(tree, btRight(tree, node), dico, survivorTree);
    }
}

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
    int n = llLength(objects);
    if (n <= 1)
        return NULL; // vide

    BTree *dico_clusters = btCreate(); // ( nom_objet -> Sous-arbre du dendrogramme )
    List *paires = llCreateEmpty();    // liste de ( objet1, objet2, distance )
    BTree *T_big;
    BTree *T_small;
    Paire *p = malloc(sizeof(Paire));
    BTNode *noeud_A = llHead(objects);
    BTNode *noeud_B = llHead(objects);

    while (!noeud_A)
    {
        while (!noeud_B)
        {
            if (noeud_A == noeud_B)
                llNext(noeud_B);
            char *o1 = (char *)llData(noeud_A);
            char *o2 = (char *)llData(noeud_B);
            double dist = distFn(o1, o2, distFnParams);

            p->o1 = o1;
            p->o2 = o2;
            p->dist = dist;
            llInsertLast(paires, p);
            llNext(noeud_B);
        }
        noeud_B = noeud_A;
        llNext(noeud_B);
    }
    // tri des paires ( important pour efficacité je crois )

    llSort(paires, comparePaires); // faire pour tous les éléments de paires ( le faire d'une façon smart)

    // initialisation des clusters ( chaque objet est son propre cluster )
    for (int i = 0; i < n; i++)
    {
        char *obj = (char *)llData(objects); // souci je veux la data de l'objet i mais je donne tous les objets
        BTree *tree = btCreate();
        btCreateRoot(tree, obj);
        dictInsert(dico_clusters, obj, tree);
    }
    // fusion des clusters( tant qu'il n'y a pas k=1 cluster )
    while (llLength(paires) > 0)
    {
        Paire *minPair = (Paire *)llData(paires); // paire la plus proche distance zéro je crois ?

        // trouver les sous-arbres ( aka clusters ) des objets
        BTree *T_o1 = dictSearch(dico_clusters, minPair->o1);
        BTree *T_o2 = dictSearch(dico_clusters, minPair->o2);

        if (!T_o1 || !T_o2 || T_o1 == T_o2)
        {
            llPopFirst(paires);
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
        btMergeTrees(T_big, T_small, distPtr);

        // mettre à jour le dictionnaire

        // supprimer la paire traitee
        llPopFirst(paires);

        // verifier si on a atteint 1 seul sous-arbre
        if (llLength(dico_clusters) == 1)
            break;
    }

    // creer la structure Hclust a retourner
    Hclust *hc = malloc(sizeof(Hclust));
    hc->dendrogramme = T_big; // L'arbre final
    hc->nombre_feuilles = n;
    hc->liste_distances_fusion = paires;
    dictFree(dico_clusters);
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

    if (!node || dendrogramme || distanceThreshold || clustersList)
        return; // safety first

    double node_distance = (double)(uintptr_t)btGetData(dendrogramme, node);

    if (btIsExternal(dendrogramme, node))
    {
        List *cluster = llCreateEmpty();
        llInsertLast(cluster, btGetData(dendrogramme, node)); // le nom du cluster
        llInsertLast(clustersList, cluster);                  // rajouter le cluster à la liste
        return;
    }
    double *distPtr = (double *)btGetData(dendrogramme, node);
    double node_distance = *distPtr;

    // si la distance est <= au seuil --> c'est un cluster du bon threshold
    if (node_distance <= distanceThreshold)
    {
        // lister toutes les feuilles de ce sous-arbre
        List *cluster = llCreateEmpty();
        collectLeaves(dendrogramme, node, cluster);
        llInsertLast(clustersList, cluster); // ajouter le cluster à la liste
        return;
    }
    // Sinon, continuer la descente recursivement
    if (btHasLeft(dendrogramme, node))
        hclustGetClustersDistRec(dendrogramme, btLeft(dendrogramme, node), distanceThreshold, clustersList);
    if (btHasRight(dendrogramme, node))
        hclustGetClustersDistRec(dendrogramme, btRight(dendrogramme, node), distanceThreshold, clustersList);
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
    if (!hc || !hc->dendrogramme || k <= 0 || k > hc->nombre_feuilles)
        return NULL; // safety first

    List *clustersNodes = llCreateEmpty();
    llInsertFirst(clustersNodes, btRoot(hc->dendrogramme)); // on commence avec la racine (1 cluster)

    BTNode *maxNode = NULL;
    BTNode *current = NULL;
    double max_dist = -1.0;
    while (llLength(clustersNodes) < k)
    {
        // on cherche dans la liste le cluster qui a la plus grande distance ( le plus haut).
        maxNode = "cluster1dist" > "cluster2dist" ? "cluster1" : "cluster2"; // TODO  c'est du filler

        // si feuille on retire et on quitte la boucle
        if (btIsExternal(hc->dendrogramme, maxNode))
            break;

        // on ajoute les deux children à droite et gauche de la liste
        llInsertLast(clustersNodes, btLeft(hc->dendrogramme, maxNode));
        llInsertLast(clustersNodes, btRight(hc->dendrogramme, maxNode));
        // on recommence
    }
    // retourner le resultat
    return clustersNodes;
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

    BTNode *left = btLeft(hc->dendrogramme, node);
    BTNode *right = btRight(hc->dendrogramme, node);
    if (!left && !right)
        hc->nombre_feuilles++; // rajouter 1 feuille à hc celle partie est inutile elle compte les feuille pour rien
    int t_left = hclustDepthRec(hc, depth + 1, left);
    int t_right = hclustDepthRec(hc, depth + 1, right);
    return 1 + (t_left >= t_right ? t_left : t_right);
}

int hclustDepth(Hclust *hc)
{
    // fonction wrapper recursive pour connaitre la profondeur de hc. C'est une recursivite de base
    int depth = 0;
    BTNode *root = btRoot(hc->dendrogramme);
    if (!root)
        return depth;
    int *nbleaves = 0;
    depth = hclustRec(hc, nbleaves, root);
    return depth;
}

int hclustNbLeaves(Hclust *hc)
{
    // pas se casser les pieds
    return hc->nombre_feuilles;
}

void hclustPrintTreeRec(FILE *fp, BTree *tree, BTNode *node)
{
    if (btIsExternal(tree, node))
    {
        fprintf(fp, "%s", (char *)btGetData(tree, node));
    }
    else
    {

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
