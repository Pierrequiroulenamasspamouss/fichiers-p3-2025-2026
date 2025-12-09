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

    val *values = (val *)fparams;
    BTree *survivorTree = values->T_big;
    Dict *dico = values->dico;
    char *key = (char *)data;            // data est la clé (char*)
    dictInsert(dico, key, survivorTree); // Utiliser la clé directement
} //@Tanguy c'est pas Intellisense c'est Copilot alors si tu as de l'autocomplete multi lignes

Hclust *hclustBuildTree(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{

    int n = llLength(objects);
    if (n <= 1)
        return NULL; // vide
    Dict *dico = dictCreate(llLength(objects));
    List *paires = llCreateEmpty(); // liste de ( objet1, objet2, distance )
    BTree *T_big = NULL;
    BTree *T_small = NULL;
    Node *noeud_A = llHead(objects);

    // @Tanguy, ici il y avait un souci avec les pointeurs dans les while. c'est maintenant corrigé.
    while (noeud_A) // tant que noeud_A n'est pas NULL

    {
        char *o1 = (char *)llData(noeud_A);

        // noeud_B doit commencer à l'élément suivant
        Node *noeud_B = llNext(noeud_A);
        while (noeud_B) // tant que noeud_B n'est pas NULL

        {
            // j'ai aussi modifié pour les pointeurs  cette partie du code avec les différentes paires o1et o2
            // on m'a dit de faire gaffe si dist(o1, o2) != dist(o2, o1) mais je crois pas que ce soit le cas.
            char *o2 = (char *)llData(noeud_B);
            double dist = distFn(o1, o2, distFnParams);

            Paire *p = malloc(sizeof(Paire));
            p->o1 = o1;
            p->o2 = o2;
            p->dist = dist;
            llInsertLast(paires, p);

            noeud_B = llNext(noeud_B); // Avance B
        }
        noeud_A = llNext(noeud_A); // Avance A
    }

    // triage efficace
    llSort(paires, comparePaires);

    // initialisation des clusters ( chaque objet est son propre cluster )
    // @Tanguy j'ai aussi modif l'initialisation des clusters.
    Node *current_obj_node = llHead(objects);
    while (current_obj_node) // tant que l'objet n'est pas NULL
    {
        char *obj = (char *)llData(current_obj_node); // c'est un char*
        BTree *tree = btCreate();

        // copie de la chaine de caractère ---
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
        Node *headNode = llHead(paires);            // CORRECTION : On utilise la head
        Paire *minPair = (Paire *)llData(headNode); 
        // trouver les sous-arbres ( aka clusters ) des objets
        BTree *T_o1 = dictSearch(dico, minPair->o1);
        BTree *T_o2 = dictSearch(dico, minPair->o2);
        if (!T_o1 || !T_o2 || T_o1 == T_o2) // arbre different (pris en compte ) OK
        {
            llPopFirst(paires);
            free(minPair); // CORRECTION : libération de la paire
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
        val *values = malloc(sizeof(val));
        values->dico = dico;
        values->T_big = T_big;
        values->T_small = T_small;

        // je m'assure que tout les les element de big et tsmall et tbig pointe vers t

        btMapLeaves(T_small, btRoot(T_small), dic, values); // CORRECTION : Fonction 'dic' passée

        // FAIT CHIER je dois sauvegarder Tbif precedent j'espere que cette sauvegarde ne me coutera pas un malloc non opportun
        // @Tanguy non tkt pas de mallocs... J'ai changé c'est pas trop nécessaire.

        btMergeTrees(T_big, T_small, distPtr);

        //@Tanguy y'a plus besoin :)

        free(values); // T_big est l'arbre final, T_small a été fusionné.
        // supprimer la paire traitee
        llPopFirst(paires);
        free(minPair); // CORRECTION : on libère la paire après l'avoir pop @Tanguy
        // verifier si on a atteint 1 seul sous-arbre
        // if (dictGetNbKeys(dico) == n) // @Tanguy j'ai trouvé mieux : c'est plus nécessaire.
        //     break;
    }
    // creer la structure Hclust a retourner
    Hclust *hc = malloc(sizeof(Hclust));
    hc->dendrogramme = T_big; // L'arbre final
    hc->nombre_feuilles = n;
    hc->liste_distances_fusion = paires; // Garde la liste des paires (maintenant vide)
    dictFree(dico);                      // ajout : il faut libérer le dico ici, plus besoin après
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
    if (!node || !dendrogramme || !clustersList)
        return; // safety first

    // correction : GetData sur feuille = char*, sur noeud = double*.
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

    // si la distance est <= au seuil --> c'est un cluster du bon threshold
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
    // on insère la racine (l'unique cluster initial)
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
                // la distance de fusion est stockée dans la data du noeud interne
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

        // si on a trouvé aucun noeud interne (tous sont des feuilles) on peut pas diviser davantage.
        if (maxNode == NULL)
            break;

        // retirer le noeud parent trouvé de la liste des clusters
        if (nodeBeforeMax == NULL)
        {
            // c'est la tête de liste
            // llPopFirst gère la suppression du noeud, la mise à jour de la tête et de la longueur.
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
    // Le main veut une liste de listes de noms, pas une liste de noeuds d'arbre.
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

    // IMPORTANT : nettoyage complet (double* ET char*) , avant le nettoyage était pas bon ce qui causait le double free
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
            free(hc); // Sécurité
        return;
    }
    hclustFreeDistancesRec(hc->dendrogramme, btRoot(hc->dendrogramme)); // libérer les double* ET les char*
    btFree(hc->dendrogramme);                                           // libérer les noeuds BTNode*

    // Sécurité pour la liste
    if (hc->liste_distances_fusion)
        llFree(hc->liste_distances_fusion);

    free(hc);
}

static int hclustDepthRec(Hclust *hc, BTNode *node) // node doit etre un pointeur
{
    // traitement des noeud pourris. Merci @Tanguy
    if (!node)
        return 0;

    BTNode *left = btLeft(hc->dendrogramme, node);
    BTNode *right = btRight(hc->dendrogramme, node);
    if (!left && !right)
        return 1;
    int t_left = hclustDepthRec(hc, left);
    int t_right = hclustDepthRec(hc, right);
    return 1 + (t_left >= t_right ? t_left : t_right);
}

int hclustDepth(Hclust *hc)
{
    // fonction wrapper recursive pour connaitre la profondeur de hc. C'est une recursivite de base
    int depth = 0;
    if (!hc || !hc->dendrogramme)
        return 0; // sécurité ajoutée
    BTNode *root = btRoot(hc->dendrogramme);
    if (!root)
        return depth;
    depth = hclustDepthRec(hc, root);
    return depth;
}

int hclustNbLeaves(Hclust *hc)
{
    // pas se casser les pieds
    return hc->nombre_feuilles;
}

static void hclustPrintTreeRec(FILE *fp, BTree *tree, BTNode *node)
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
        double *d = (double *)btGetData(tree, node); // cast en (double) parce que getData return void
        if (d)
            fprintf(fp, ":%.3f", *d);
    }
}

void hclustPrintTree(FILE *fp, Hclust *hc)
{
    if (!hc || !hc->dendrogramme)
        return; // safety first
    hclustPrintTreeRec(fp, hc->dendrogramme, btRoot(hc->dendrogramme));
    fprintf(fp, ";\n"); // finir le newick par un ";" dans l'exemple du cours
}