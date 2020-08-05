// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BTREEPLUS_ON_DISK
#define SORACHANCOIN_FS_BTREEPLUS_ON_DISK

#include "fs_types.h"
#include "fs_memory.h"
#include "fs_const.h"
#include "fs_btree.h"

/*
* ** fs_btreeplus_on_disk **
*
* [INDEX_BUFFER] B-tree+ on disk.
*
*/

typedef enum _tag_btree_status {
    BTREEPLUS_SUCCESS = 0,
    BTREEPLUS_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
} btreeplus_status;

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
    btreeplus_status status;
    fsize_t dimension;
    fsize_t root_size;
    byte_t reserved[32];
    byte_t root[1]; /* INDEX_BUFFER */
} FSINDEX;
#pragma pack(pop)

typedef struct _tag_FSBTREEPLUS {
    FSFRAGVECTOR *vch;
    btreeplus_status status;
} FSBTREEPLUS;

static inline bool_t fs_btreeplus_setsuccess(FSBTREEPLUS *fbpp) {
    fbpp->status = BTREEPLUS_SUCCESS;
    return true_t;
}

static inline bool_t fs_btreeplus_seterror(FSBTREEPLUS *fbpp, btreeplus_status status) {
    fbpp->status = status;
    return false_t;
}

static inline bool_t fs_btree_open(FSBTREEPLUS **fbpp) {
    (*fbpp) = (FSBTREEPLUS *)fs_malloc(sizeof(FSBTREEPLUS));
    if(!*fbpp) return false_t;
    if(!fs_fragvector_open(&(*fbpp)->vch, 0, sizeof(VECTOR_DATA))) return fs_btreeplus_seterror(*fbpp, BTREEPLUS_ERROR_MEMORY_ALLOCATE_FAILURE);


    return fs_btreeplus_setsuccess(*fbpp);
}

static inline bool_t fs_btreeplus_close(FSBTREEPLUS *fbp, bool_t ret) {
    return fs_free(fbp, fs_fragvector_close(fbp->vch, ret));
}

#endif
