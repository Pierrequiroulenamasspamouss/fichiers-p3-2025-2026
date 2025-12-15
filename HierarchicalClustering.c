#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "HierarchicalClustering.h"
#include "LinkedList.h"
#include "Dict.h"

struct Hclust_t
{
    BTree *dendrogramme;
    List *liste_distances_fusion; // à voir si on s'en sert
    int nombre_feuilles;          // N
};
typedef struct
{
    char *o1;
    char *o2;
    double dist; // mis en double car c'est mieux. est ce qu'on peut mettre unsigned ?

} Paire;

typedef struct
{
    Dict *dico;
    BTree *T_big;
    BTree *T_small;
} val;

static int comparePaires(void *a, void *b)
{
    Paire *pa = (Paire *)a;
    Paire *pb = (Paire *)b;
    if (pa->dist < pb->dist)
        return -1;
    if (pa->dist > pb->dist)
        return 1;
    return 0;
}

static void dic(void *data, void *fparams)
{
    // fonction utile pour builtree
    val *values = (val *)fparams;
    BTree *survivorTree = values->T_big;
    Dict *dico = values->dico;
    char *key = (char *)data;            // data est la clé (char*)
    dictInsert(dico, key, survivorTree); // Utiliser la clé directement
}

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{

    int n = llLength(objects);
    if (n <= 1)
        return NULL;
    Dict *dico = dictCreate(llLength(objects));
    List *paires = llCreateEmpty(); // liste de ( objet1, objet2, distance )
    BTree *T_big = NULL;
    BTree *T_small = NULL;
    Node *noeud_A = llHead(objects);

    while (noeud_A) // tant que noeud_A n'est pas NULL

    {
        char *o1 = (char *)llData(noeud_A);

        // noeud_B doit commencer à l'élément suivant
        Node *noeud_B = llNext(noeud_A);
        while (noeud_B) // tant que noeud_B n'est pas NULL

        {

            char *o2 = (char *)llData(noeud_B);
            double dist = distFn(o1, o2, distFnParams);

            Paire *p = malloc(sizeof(Paire));
            p->o1 = o1;
            p->o2 = o2;
            p->dist = dist;
            llInsertLast(paires, p);

            noeud_B = llNext(noeud_B);
        }
        noeud_A = llNext(noeud_A);
    }

    // triage efficace
    llSort(paires, comparePaires);

    // initialisation des clusters ( chaque objet est son propre cluster )
    Node *current_obj_node = llHead(objects);
    while (current_obj_node) // tant que l'objet n'est pas NULL
    {
        char *obj = (char *)llData(current_obj_node); // c'est un char*
        BTree *tree = btCreate();

        // copie de la chaine de caractère
        // créer une racine pour l'arbre avec le nom de l'objet(feuille)
        // on doit maalloc pour que l'arbre possède sa propre donnée
        char *objCopy = malloc((strlen(obj) + 1) * sizeof(char)); // sinon garbage output
        if (objCopy)
            strcpy(objCopy, obj);
        btCreateRoot(tree, objCopy);

        // le dictionnaire map l'objet (clé) à l'arbre (valeur) qui représente son cluster
        dictInsert(dico, obj, tree);

        current_obj_node = llNext(current_obj_node);
    }

    // fusion des clusters( tant qu'il n'y a pas k=1 cluster )

    while (llLength(paires) > 0)
    {
        Node *headNode = llHead(paires);
        Paire *minPair = (Paire *)llData(headNode);
        // trouver les sous-arbres ( aka clusters ) des objets
        BTree *T_o1 = dictSearch(dico, minPair->o1);
        BTree *T_o2 = dictSearch(dico, minPair->o2);
        if (!T_o1 || !T_o2 || T_o1 == T_o2)
        {
            llPopFirst(paires);
            free(minPair);
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
        // malloc des valeurs utile pour btMapleaves et btMergeLeaves
        double *distPtr = malloc(sizeof(double));
        *distPtr = minPair->dist;
        val *values = malloc(sizeof(val));
        values->dico = dico;
        values->T_big = T_big;
        values->T_small = T_small;

        // je m'assure que tout les les element de big et tsmall et tbig pointe vers t

        btMapLeaves(T_small, btRoot(T_small), dic, values);
        btMergeTrees(T_big, T_small, distPtr);
        free(values); // T_big est l'arbre final, T_small a été fusionné.
        // supprimer la paire traitee
        llPopFirst(paires);
        free(minPair);
    }
    // creer la structure Hclust a retourner
    Hclust *hc = malloc(sizeof(Hclust));
    hc->dendrogramme = T_big; // l'arbre final
    hc->nombre_feuilles = n;
    hc->liste_distances_fusion = paires; // garde la liste des paires (maintenant vide)
    dictFree(dico);
    return hc;
}

static void collectLeaves(BTree *tree, BTNode *node, List *leaves)
{

    if (!node || !tree || !leaves)
        return;

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
    if (!node || !dendrogramme || !clustersList)
        return;

    // on check d'abord si externe ( aka si feuille )
    if (btIsExternal(dendrogramme, node))
    {
        List *cluster = llCreateEmpty();
        llInsertLast(cluster, btGetData(dendrogramme, node)); // le nom du cluster
        llInsertLast(clustersList, cluster);                  // rajouter le cluster à la liste
        return;
    }

    double *distPtr = (double *)btGetData(dendrogramme, node);
    double node_distance = *distPtr;

    // si la distance est <= au seuil -> c'est un cluster du bon threshold
    if (node_distance <= distanceThreshold)
    {
        // lister toutes les feuilles de ce sous-arbre
        List *cluster = llCreateEmpty();
        collectLeaves(dendrogramme, node, cluster);
        llInsertLast(clustersList, cluster); // ajouter le cluster à la liste
        return;
    }
    // sinon, continuer la descente recursivement
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

    size_t k_size = (size_t)k;
    if (!hc || !hc->dendrogramme || k <= 0 || k > hc->nombre_feuilles)
        return NULL;

    List *clustersNodes = llCreateEmpty();
    // on insere la racine (l'unique cluster initial)
    llInsertFirst(clustersNodes, btRoot(hc->dendrogramme));

    // tant que le nombre de clusters est inférieur à k
    while (llLength(clustersNodes) < k_size)
    {
        BTNode *maxNode = NULL;
        Node *nodeBeforeMax = NULL;
        double max_dist = -1.0;
        Node *current = llHead(clustersNodes);
        Node *prev = NULL;

        // chercher le noeud (cluster) qui a la plus grande distance de fusion.
        while (current)
        {
            BTNode *cluster = (BTNode *)llData(current);
            // on ne peut diviser que les noeuds internes
            if (btIsInternal(hc->dendrogramme, cluster))
            {
                // la distance de fusion est stockee dans la data du noeud interne
                double *distPtr = (double *)btGetData(hc->dendrogramme, cluster);
                double current_dist = *distPtr;

                if (current_dist > max_dist)
                {
                    max_dist = current_dist;
                    maxNode = cluster;
                    nodeBeforeMax = prev;
                }
            }
            prev = current;
            current = llNext(current);
        }

        // si on a trouve aucun noeud interne (tous sont des feuilles) on peut pas diviser davantage.
        if (maxNode == NULL)
            break;

        // retirer le noeud parent trouve de la liste des clusters
        if (nodeBeforeMax == NULL)
        {
            // c'est la tete de liste
            llPopFirst(clustersNodes); // Note : le retour est void*, ignoré ici
        }
        else
        {
            // retirer le noeud du milieu de la liste (logique de liste chaînée simple)
            // pas de moyen de retirer un node spécifique donc on est obligé de faire ça:

            List *newClustersList = llCreateEmpty();
            Node *current = llHead(clustersNodes);
            while (current)
            {
                BTNode *cluster = (BTNode *)llData(current);
                if (cluster != maxNode)
                {
                    llInsertLast(newClustersList, cluster);
                }
                current = llNext(current);
            }
            llFree(clustersNodes);
            clustersNodes = newClustersList;
        }

        // ajouter ses deux enfants (les nouveaux clusters)
        llInsertLast(clustersNodes, btLeft(hc->dendrogramme, maxNode));
        llInsertLast(clustersNodes, btRight(hc->dendrogramme, maxNode));
    }

    // conversion pointeur BTNode vers List de strings

    List *finalList = llCreateEmpty();
    Node *curr = llHead(clustersNodes);
    while (curr)
    {
        BTNode *node = (BTNode *)llData(curr);
        List *leaves = llCreateEmpty();
        collectLeaves(hc->dendrogramme, node, leaves);
        llInsertLast(finalList, leaves);
        curr = llNext(curr);
    }
    llFree(clustersNodes); // on libère la liste temporaire de noeuds
    return finalList;
}

BTree *hclustGetTree(Hclust *hc)
{
    return hc->dendrogramme;
}

static void hclustFreeDistancesRec(BTree *tree, BTNode *node)
{
    if (!node)
        return;

    hclustFreeDistancesRec(tree, btLeft(tree, node));
    hclustFreeDistancesRec(tree, btRight(tree, node));

    if (btIsInternal(tree, node))
    {
        // la data d'un noeud interne est le double* distPtr
        double *distPtr = (double *)btGetData(tree, node);
        if (distPtr)
            free(distPtr);
    }
    else if (btIsExternal(tree, node))
    {
        // les feuilles contiennent maintenant des copies malloc (char*)
        char *str = (char *)btGetData(tree, node);
        if (str)
            free(str);
    }
}
void hclustFree(Hclust *hc)
{
    if (!hc || !hc->dendrogramme)
    {
        if (hc)
            free(hc); // sécurité
        return;
    }
    hclustFreeDistancesRec(hc->dendrogramme, btRoot(hc->dendrogramme)); // libérer les double* ET les char*
    btFree(hc->dendrogramme);                                           // libérer les noeuds

    // sécurité pour la liste
    if (hc->liste_distances_fusion)
        llFree(hc->liste_distances_fusion);

    free(hc);
}

static int hclustDepthRec(Hclust *hc, BTNode *node)
{
    // Traitement des noeud vide
    if (!node)
        return 0;

    // condition d'arret
    if (btIsExternal(hc->dendrogramme, node))
        return 0;

    BTNode *left = btLeft(hc->dendrogramme, node);
    BTNode *right = btRight(hc->dendrogramme, node);

    int t_left = hclustDepthRec(hc, left);
    int t_right = hclustDepthRec(hc, right);

    // on ajoute 1 pour l'arête
    return 1 + (t_left >= t_right ? t_left : t_right);
}

int hclustDepth(Hclust *hc)
{
    if (!hc || !hc->dendrogramme)
        return 0;

    BTNode *root = btRoot(hc->dendrogramme);
    if (!root)
        return 0;

    return hclustDepthRec(hc, root);
}

int hclustNbLeaves(Hclust *hc)
{
    return hc->nombre_feuilles;
}
static double getNodeHeight(BTree *tree, BTNode *node)
{
    if (!node)
        return 0.0;

    // si c'est une feuille -> height =0
    if (btIsExternal(tree, node))
        return 0.0;

    // si c'est un nœud interne
    double *d = (double *)btGetData(tree, node);
    if (d)
        return *d; 

    return 0.0;
}

static void hclustPrintTreeRec(FILE *fp, BTree *tree, BTNode *node, double parentHeight, int isRoot)
{
    // securite 1
    if (!node)
        return;

    double currentHeight = getNodeHeight(tree, node);
    double branchLength = parentHeight - currentHeight;

    // securite 2
    if (branchLength < 0)
        branchLength = 0;

    if (btIsExternal(tree, node))
    {
        // juste une feuille affiche sa longueur
        fprintf(fp, "%s:%f", (char *)btGetData(tree, node), branchLength);
    }
    else
    {
        fprintf(fp, "(");

        // appels récursifs
        hclustPrintTreeRec(fp, tree, btLeft(tree, node), currentHeight, 0);

        fprintf(fp, ",");

        hclustPrintTreeRec(fp, tree, btRight(tree, node), currentHeight, 0);

        fprintf(fp, ")");

        // on n'affiche la longueur partout sauf à root
        if (!isRoot)
        {
            fprintf(fp, ":%f", branchLength);
        }
    }
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{
    if (!hc || !hc->dendrogramme)
        return;

    BTNode *root = btRoot(hc->dendrogramme);
    double rootHeight = getNodeHeight(hc->dendrogramme, root);

    // on appelle la récursion en précisant que c'est la root
    hclustPrintTreeRec(fp, hc->dendrogramme, root, rootHeight, 1);

    fprintf(fp, ";\n");
}