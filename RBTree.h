/*
 * Copyright (C) 2015 Roy O'Young <roy2220@outlook.com>.
 */


#pragma once


#include <stdint.h>


enum __RBTreeNodeColor
{
    RBTreeNodeRed,
    RBTreeNodeBlack
};


struct RBTreeNode
{
    struct RBTreeNode *parent;
    struct RBTreeNode *leftChild;
    struct RBTreeNode *rightChild;
    enum __RBTreeNodeColor color;
};


struct RBTree
{
    struct RBTreeNode nil;
    struct RBTreeNode *root;
};


void RBTree_Initialize(struct RBTree *);
void RBTree_InsertNode(struct RBTree *, struct RBTreeNode *, int (*)(const struct RBTreeNode *
                                                                     , const struct RBTreeNode *));
void RBTree_RemoveNode(struct RBTree *, const struct RBTreeNode *);
struct RBTreeNode *RBTree_Search(const struct RBTree *, uintptr_t, int (*)(const struct RBTreeNode *
                                                                           , uintptr_t));
struct RBTreeNode *RBTree_FindMin(const struct RBTree *);
struct RBTreeNode *RBTree_FindMax(const struct RBTree *);
