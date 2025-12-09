
// Replaced by hclustDepthRec
static int hclustRec(Hclust *hc, int *nbleaves ,BTNode *node)
{

    BTNode *left = btLeft(hc->dendrogramme, node);
    BTNode *right = btRight(hc->dendrogramme, node);
    if(!left&&!right){
        nbleaves++;
        hc->nombre_feuilles++ ; // pas certain de ce que je fais ici, j'avais dit que je devais rajouter à nombre_feuilles si y'a feuille
        return 1;
    }
    int t_left=hclustRec(hc, nbleaves,left);
    int t_right = hclustRec(hc, nbleaves,right);

    if(t_left<t_right)
        return t_right+1;
    return t_left+1;
}

// replaced by btIsExternal
static bool Isleaves(BTree *tree,BTNode *node){
    // void *tree;
    BTNode *left = btLeft(tree, node);
    BTNode *right = btRight(tree, node);
    if(!left&&!right)
        return true;
    return false;

}
static Hclust *hclustBuildTreeTanguy(List *objects, double (*distFn)(const char *, const char *, void *), void *distFnParams)
{
    // TODO
    // les condtion limite :
    // !!! quand on parcours la liste lié des distance et que 2 point sont dans lememe cluster on ne faait rien
    // sinon on les lie (normalement deja pris en compte grace au dictionnaire )

    int n = llLength(objects);
    if (n <= 1)
        return NULL; // vide

    Dict *dico = dictCreate(llLength(objects));
    List *paires = llCreateEmpty(); // liste de ( objet1, objet2, distance )
    BTree *T_big;
    BTree *T_small;
    Node *noeud_A = llHead(objects);
    // @Tanguy, ici il y avait un souci avec les pointeurs dans les while. c'est maintenant corrigé.
    while (noeud_A) // tant que noeud_A n'est pas NULL
    {
        char *o1 = (char *)llData(noeud_A);
        // noeud_B doit commencer à l'élément suivant
        Node *noeud_B = llNext(noeud_A);
        while (noeud_B) // tant que noeud_B n'est pas NULL
        {
            // j'ai aussi optimisé cette partie du code avec les différentes paires o1et o2
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
        // créer une racine pour l'arbre avec le nom de l'objet(feuille)
        btCreateRoot(tree, obj);
        // le dictionnaire map l'objet (clé) à l'arbre (valeur) qui représente son cluster
        dictInsert(dico, obj, tree);

        current_obj_node = llNext(current_obj_node);
    }
    // fusion des clusters( tant qu'il n'y a pas k=1 cluster )
    noeud_A = llHead(paires);
    while (llLength(paires) > 0)

    {
        Paire *minPair = (Paire *)llData(noeud_A); // paire la plus proche distance zéro je crois ?
        // trouver les sous-arbres ( aka clusters ) des objets
        BTree *T_o1 = dictSearch(dico, minPair->o1);
        BTree *T_o2 = dictSearch(dico, minPair->o2);

        if (!T_o1 || !T_o2 || T_o1 == T_o2) // arbre different (pris en compte ) OK
        {
            llPopFirst(paires);
            llNext(noeud_A);
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
        btMapLeaves(T_small, btRoot(T_small), btIsExternal(T_big, current_obj_node), values);
        // FAIT CHIER    je dois sauvegard Tbif precedent j'espere que cette sauvegarde ne me coutera pas un malloc non opportun
        BTree *T_temporaire = btCreate();
        T_temporaire = T_small;
        btMergeTrees(T_big, T_small, distPtr);
        values->dico = dico;
        values->T_big = T_big;
        values->T_small = T_temporaire;
        btMapLeaves(T_big, btRoot(T_big), dico, values);
        btFree(T_temporaire);
        free(values);

        // supprimer la paire traitee
        llPopFirst(paires);
        llNext(noeud_A);
        // verifier si on a atteint 1 seul sous-arbre
        if (llLength(paires) == 1)
            break;
    }

    // creer la structure Hclust a retourner
    Hclust *hc = malloc(sizeof(Hclust));
    hc->dendrogramme = T_big; // L'arbre final
    hc->nombre_feuilles = n;
    hc->liste_distances_fusion = paires;
    dictFree(dico);
    return hc;
}