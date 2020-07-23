// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_CLUSTER
#define SORACHANCOIN_FS_CLUSTER

#include "fs_const.h"
#include "fs_bitmap.h"
#include "fs_bpb.h"

#define CLUSTER_SIZE (SECTOR_SIZE * SECTORS_PER_CLUS)
#define SECTORS_PER_BITMAP (BITMAP_SIZE / SECTOR_SIZE)

/**
* ** fs_cluster **
* 
* from sector to cluster.
*/

static inline sector_t fs_cluster_getsectot(const BPB *bpb, cluster_t clus) {
    fsize_t bitmap_num = ((bpb->bpb_offset%CLUSTER_CAPACITY+clus*SECTORS_PER_CLUS) * SECTOR_SIZE)/fs_file_getsize();
    return clus*SECTORS_PER_CLUS + bpb->bpb_offset + bitmap_num*SECTORS_PER_BITMAP;
}

#endif
