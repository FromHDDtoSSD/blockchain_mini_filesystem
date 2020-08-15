// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_CLUSTER
#define SORACHANCOIN_FS_CLUSTER

#include "fs_const.h"
#include "fs_disk.h"
#include "fs_bitmap.h"
#include "fs_bpb.h"

/**
* ** fs_cluster **
* 
* 1, From cluster to sector.
*
* 2, Supports reading and writing in cluster units.
* If you adopt to use the "Bitmap", use to these function.
*
* 3, Use fs_bitmap(FSBITMAP) for error status.
*
*/

static inline sector_t fs_cluster_getsector(const BPB *bpb, cluster_t clus) {
    assert(clus>=0);
    return bpb->bpb_offset+clus*SECTORS_PER_CLUSTER;
}

static inline cluster_t fs_cluster_getcluster(const BPB *bpb, sector_t sector) {
    assert(sector>=0);
    return (sector-bpb->bpb_offset)/SECTORS_PER_CLUSTER;
}

static inline bool_t fs_cluster_diskread(FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num, byte_t *buf) {
    return fs_diskwith_bitmap_read(bp, fs_cluster_getsector(bpb, begin), num*SECTORS_PER_CLUSTER, buf);
}

static inline bool_t fs_cluster_diskwrite(FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num, const byte_t *buf) {
    return fs_diskwith_bitmap_write(bp, fs_cluster_getsector(bpb, begin), num*SECTORS_PER_CLUSTER, buf);
}

static inline bool_t fs_cluster_erasebitmap(FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num) {
    return fs_diskwith_bitmap_erase(bp, fs_cluster_getsector(bpb, begin), num*SECTORS_PER_CLUSTER);
}

static inline bool_t fs_cluster_someusedrange(FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num, bool_t *used) {
    return fs_bitmap_getmask_someusedrange(bp, fs_cluster_getsector(bpb, begin), num*SECTORS_PER_CLUSTER, used);
}

static inline bool_t fs_cluster_getfreecluster(FSBITMAP *bp, const BPB *bpb, counter_t num, cluster_t *clus_begin) {
    sector_t begin;
    if(!fs_bitmap_getmask_freesector(bp, num*SECTORS_PER_CLUSTER, bpb->bpb_offset, &begin)) return b_false;
    *clus_begin=fs_cluster_getcluster(bpb, begin);
    assert(*clus_begin>=0);
    return b_true;
}

#endif
