
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