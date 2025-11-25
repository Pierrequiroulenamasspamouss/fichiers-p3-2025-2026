#ifndef _BTREE_H
#define _BTREE_H

typedef struct BTNode_t BTNode;
typedef struct BTree_t BTree;

/**
 * @brief Creates a new binary tree without any node.
 *
 * @return BTree* the newly created tree
 */
BTree *btCreate(void);

/**
 * @brief Frees the tree from memory. The data is unfreed.
 *
 * @param tree
 */
void btFree(BTree *tree);

/**
 * @brief Adds a root to the tree with the associated data. The tree should
 *        not have any node.
 *
 * @param tree the tree to which the root is added
 * @param data the data associated to the root
 * @return BTNode* the created root
 */
BTNode *btCreateRoot(BTree *tree, void *data);

/**
 * @brief Inserts a new node to the left of node n with the data. Node n
 *        should not have a left successor.
 *
 * @param tree the tree
 * @param n the node to which a left successor will be added
 * @param data the data
 * @return BTNode* the new node that is added
 */
BTNode *btInsertLeft(BTree *tree, BTNode *n, void *data);

/**
 * @brief Inserts a new node to the right of node n with the data. Node n
 *        should not have a right successor.
 * @param tree the tree
 * @param n the node to which a right successor will be added
 * @param data the data
 * @return BTNode*  the new node that is added
 */
BTNode *btInsertRight(BTree *tree, BTNode *n, void *data);

/**
 * @brief returns the root of the tree
 *
 * @param tree the tree
 * @return BTNode* the root of the tre
 */
BTNode *btRoot(BTree *tree);

/**
 * @brief returns the left successor of node n.
 *
 * @param tree the tree
 * @param n the node
 * @return BTNode* the left successsor of node n
 */
BTNode *btLeft(BTree *tree, BTNode *n);

/**
 * @brief returns the right successor of node n.
 *
 * @param tree the tree
 * @param n the node
 * @return BTNode* the right successsor of node n
 */
BTNode *btRight(BTree *tree, BTNode *n);

/**
 * @brief returns the parent of node n.
 *
 * @param tree the tree
 * @param n the node
 * @return BTNode* the parent of n
 */
BTNode *btParent(BTree *tree, BTNode *n);

/**
 * @brief Returns the data associated to node n.
 *
 * @param tree the tree
 * @param n the node
 * @return void* the data at node n
 */
void *btGetData(BTree *tree, BTNode *n);

/**
 * @brief Returns the number of nodes in the tree.
 *
 * @param tree
 * @return int
 */
int btSize(BTree *tree);

/**
 * @brief Returns 1 if n is the root of the tree, 0 otherwise.
 *
 * @param tree the tree
 * @param n the node n
 * @return int 1 if n is the root of the tree, 0 otherwise
 */
int btIsRoot(BTree *tree, BTNode *n);

/**
 * @brief Returns 1 if n is an internal node of the tree, 0 otherwise
 *
 * @param tree the tree
 * @param n the node
 * @return int 1 if n is an internal node, 0 otherwise.
 */
int btIsInternal(BTree *tree, BTNode *n);

/**
 * @brief Returns 1 if n is an external node of the tree, 0 otherwise
 *
 * @param tree the tree
 * @param n the node
 * @return int 1 if n is an external node, 0 otherwise.
 */
int btIsExternal(BTree *tree, BTNode *n);

/**
 * @brief Returns 1 if n has a left successor, 0 otherwise.
 *
 * @param tree the tree
 * @param n the node
 * @return int 1 if n has a left successor, 0 otherwise.
 */
int btHasLeft(BTree *tree, BTNode *n);

/**
 * @brief Returns 1 if n has a right successor, 0 otherwise.
 *
 * @param tree the tree
 * @param n the node
 * @return int 1 if n has a right successor, 0 otherwise.
 */
int btHasRight(BTree *tree, BTNode *n);

/**
 * @brief Applies the function f to all data at leaf nodes of the subtree rooted at node n.
 *        f is applied with the node as first argument and fparams as second argument.
 *
 * @param tree the tree
 * @param n the root of the subtree
 * @param f the function to be applied on all leaves
 * @param fparams the second argument to be given to f
 */
void btMapLeaves(BTree *tree, BTNode *n, void (*f)(void *data, void *fparams), void *fparams);

/**
 * @brief Modifies the lefttree so that it gets a new root with the data. The left successor
 *        of the new root is the previous root of lefttree (and all its tree) and its right
 *        successor is the root of righttree (and all its tree). rightree is freed after the merge.
 *
 * @param lefttree
 * @param righttree
 * @param data
 */
void btMergeTrees(BTree *lefttree, BTree *righttree, void *data);

#endif
