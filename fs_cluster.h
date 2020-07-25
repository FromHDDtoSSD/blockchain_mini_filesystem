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
* If you adopt to use the Bitmap, use to these function.
*
**********************************************************
* NOTE:
* BITMAP should be considered from the second chunks.
* This is because the chunk with BPB has before cluster 0.
*
* [BITMAP][BPB(cluster 0)][cluster], [BITMAP][cluster] ...
**********************************************************
*
*/

static inline counter_t fs_cluster_getusedbitmap(const BPB *bpb, cluster_t clus) {
    //return ((bpb->bpb_offset + clus*SECTORS_PER_CLUS)*SECTOR_SIZE)/fs_file_getsize();
    return (clus/CLUSTER_CAPACITY + 1) - (bpb->bpb_offset/SECTOR_CAPACITY + 1);
}

static inline cluster_t fs_cluster_getrelativeclus(const BPB *bpb, cluster_t clus) {
    sector_t headsector = SECTOR_CAPACITY - bpb->bpb_offset%SECTOR_CAPACITY;
    cluster_t headclus  = headsector/SECTORS_PER_CLUS;
    if(clus<headclus) return clus;
    clus-=headclus;
    return clus%(CLUSTER_CAPACITY-CLUSTER_PER_BITMAP);
}

static inline sector_t fs_cluster_getsector(const BPB *bpb, cluster_t clus) {
    counter_t bitmap_num = fs_cluster_getusedbitmap(bpb, clus);
    return clus*SECTORS_PER_CLUS + bpb->bpb_offset + bitmap_num*SECTORS_PER_BITMAP;
}

static inline bool_t fs_cluster_diskrw(FSDISK *fdp, const FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num, byte_t *buf, 
                                       bool_t (*funcrw)(FSDISK *fp, sector_t begin, counter_t num, byte_t *buf)) {
    if(!bp) fs_disk_seterror(fdp, DISK_ERROR_PARAM);
    sector_t sector=fs_cluster_getsector(bpb, begin);
    counter_t remain=num*SECTORS_PER_CLUS;
    for(index_t i=0; remain>0; ++i) {
        counter_t rwsec=(i==0 && remain>SECTOR_CAPACITY-sector%SECTOR_CAPACITY) ? SECTOR_CAPACITY-sector%SECTOR_CAPACITY: remain;
        rwsec=(rwsec > SECTOR_CAPACITY) ? SECTOR_CAPACITY: rwsec;
        if(rwsec==SECTOR_CAPACITY) rwsec-=SECTORS_PER_BITMAP;
        if(i>0) sector+=SECTORS_PER_BITMAP;
        if(!funcrw(fdp, sector, rwsec, buf)) return fs_disk_seterror(fdp, DISK_ERROR_DRIVE_RW_FAILURE);
        if((remain-=rwsec)==0) break;
        buf+=rwsec*SECTOR_SIZE;
        sector+=rwsec;
    }
    return fs_disk_setsuccess(fdp);
}

static inline bool_t fs_cluster_diskread(FSDISK *fdp, const FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num, byte_t *buf) {
    return fs_cluster_diskrw(fdp, bp, bpb, begin, num, buf, &fs_disk_read);
}

static inline bool_t fs_cluster_diskwrite(FSDISK *fdp, const FSBITMAP *bp, const BPB *bpb, cluster_t begin, counter_t num, const byte_t *buf) {
    return fs_cluster_diskrw(fdp, bp, bpb, begin, num, buf, &fs_disk_write);
}

#endif
