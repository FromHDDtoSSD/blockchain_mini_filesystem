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
* ** fs_bitmap **
*
* The Bitmap records position to the used cluster.
* The first 1st cluster of each chunk will be used for management to the Bitmap area.
*
* It is generated at the beginning of each chunk.
* 0x00: unused, 0x01: used
*
* Note, The link between sectors and clusters strictly adheres to the following.
* sector: include Bitmap area. It is assigned as an absolute position from the 0th chunk.
* cluster: NO include Bitmap area. It is assigned as a relative position by BPB.
*/

#define BITMAP_HEADER_OFFSET_BYTE 0 /* This cannot be changed. */
#define CLUSTER_PER_BITMAP 1
#define SECTORS_PER_BITMAP (SECTORS_PER_CLUS*CLUSTER_PER_BITMAP)
#define BITMAP_SIZE (SECTORS_PER_BITMAP*SECTOR_SIZE)

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
    const BPB *bpb;
    bitmap_status status;
} FSBITMAP;

bool_t fs_bitmap_open(FSBITMAP **bp, const BPB *bpb, FSDISK *fdp);
bool_t fs_bitmap_close(FSBITMAP *bp, bool_t ret);
bool_t fs_bitmap_isused(FSBITMAP *bp, cluster_t clus, bool_t *used);

#endif
