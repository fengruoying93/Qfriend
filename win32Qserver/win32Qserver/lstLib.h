#ifndef LSTLIB_H
#define LSTLIB_H

typedef struct node	/* Node of a linked list. */
{
    struct node *next;	    /* Points at the next node in the list */
    struct node *previous;  /* Points at the previous node in the list */
} NODE;


typedef struct   /* Header for a linked list. */
{
    NODE node;   /* Header list node */
    int count;   /* Number of nodes in list */
} LIST;


void lstInit(LIST *pList);
void lstAdd(LIST *pList, NODE *pNode);
void lstConcat(LIST *pDstList, LIST *pAddList);
int lstCount(LIST *pList);
void lstDelete(LIST *pList, NODE *pNode);
void lstExtract(LIST *pSrcList, NODE *pStartNode, NODE *pEndNode, LIST *pDstList);
NODE *lstFirst(LIST *pList);
NODE *lstGet(LIST *pList);
void lstInsert(LIST *pList, NODE *pPrev, NODE *pNode);
NODE *lstLast(LIST *pList);
NODE *lstNext(NODE *pNode);
NODE *lstNth(LIST *pList, int nodenum);
NODE *lstPrevious(NODE *pNode);
NODE *lstNStep(NODE *pNode, int nStep);
int lstFind(LIST *pList, NODE *pNode);
void lstFree(LIST *pList);


#endif

