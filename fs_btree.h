// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BTREE
#define SORACHANCOIN_FS_BTREE

#include "fs_types.h"
#include "fs_memory.h"
#include "fs_const.h"
#include "fs_disk.h"

/*
* ** fs_btree **
*
* [INDEX_BUFFER] B-tree on memory and, B-Tree+ on disk.
*
* filesystem implement.
*/

typedef enum _tag_node_type {
    n_unused,
    n_node,
    n_leaf,
} node_type;

typedef enum _tag_node_connected {
    connect_no,
    connected,
} node_connected;

typedef enum _tag_del_type {
    del_no,
    del_ok,
    del_removed,
    del_need_balance,
} del_type;

typedef struct _tag_NODE_CONFIRMED {
    node_type type;
    union {
        struct {
            counter_t num;
            struct _tag_NODE **ptr;
            index_t *begin_ptr;
        } node;
        struct {
            index_t index;
        } leaf;
    } tree;
} NODE_CONFIRMED;

void fs_btree_initnode(NODE_CONFIRMED *node) {
    node->tree.node.ptr = NULL;
    node->tree.node.begin_ptr = NULL;
}

void fs_btree_insertnode(NODE_CONFIRMED *dest, const NODE_CONFIRMED *src) {
    dest->type = src->type;
    if(src->type == n_leaf)
        dest->tree.leaf.index = src->tree.leaf.index;
    else {
        dest->tree.node.num = src->tree.node.num;
        dest->tree.node.ptr = src->tree.node.ptr;
        dest->tree.node.begin_ptr = dest->tree.node.begin_ptr;
    }
}

typedef enum _tag_index_status {
    INDEX_SUCCESS = 0,
} index_status;

#pragma pack(push, 1)
typedef struct _tag_INDEX_BUFFER {
    byte_t signature[4]; /* INDX */
    byte_t leaf; /* 1: leaf, 0: node */
    byte_t target; /* 1: need to process, 0: no change */
    byte_t reserved[32];
    union unit {
        struct leaf {
            fsize_t key_size;
            fsize_t value_size;
            byte_t data[1];
        };
        struct node {
            fsize_t ptr_num;
            index_t ptr[1];
        };
    };
} INDEX_BUFFER;

typedef struct _tag_FSINDEX {
    byte_t signature[4]; /* INDX */
    index_status status;
    fsize_t dimension;
    fsize_t root_size;
    byte_t reserved[32];
    byte_t root[1]; /* INDEX_BUFFER */
} FSINDEX;
#pragma pack(pop)

#endif
