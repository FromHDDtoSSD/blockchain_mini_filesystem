// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BITMAP
#define SORACHANCOIN_FS_BITMAP

#include "fs_types.h"
#include "fs_memory.h"
#include "fs_disk.h"
#include "fs_bpb.h"

/*
* The Bitmap records position to the used cluster.
*
* It is generated at the beginning of each chunk.
* 0: unused, 1: used
*
* sector: include Bitmap
* cluster: NO include Bitmap
*/

#define BITMAP_HEADER_OFFSET_BYTE 0
#define BITS 8
#define BITMAP_SIZE (CLUSTER_CAPACITY/BITS)

#pragma pack(push, 1)
typedef struct _tag_BITMAP_INFO {
    byte_t bitmap[BITMAP_SIZE];
} BITMAP_INFO;
#pragma pack(pop)

typedef enum _tag_bitmap_status {
    BITMAP_SUCCESS = 0,
    BITMAP_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
    BITMAP_ERROR_DRIVE_RW_FAILURE = 2,
} bitmap_status;

typedef struct _tag_FSBITMAP {
    FSDISK *fdp;
    bitmap_status status;
} FSBITMAP;

bool_t fs_bitmap_open(FSBITMAP **bp, FSDISK *fdp);
bool_t fs_bitmap_close(FSBITMAP *bp, bool_t ret);
bool_t fs_bitmap_isused(FSBITMAP *bp, const BPB *bpb, cluster_t clus, bool_t *used);

#endif
