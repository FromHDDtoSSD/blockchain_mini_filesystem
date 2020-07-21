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
* [INDEX_BUFFER] B-tree+ on disk.
*
* filesystem implement.
* fs_header(signature: SORA) => fs_hash(signature: HASH), fs_bitmap(signature: BITM), fs_btree(signature: INDX), mini_filesystem(signature: FILE) and fs_logfile(signature: LOGF)
*/

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
