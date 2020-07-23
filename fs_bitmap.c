// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fs_file.h"
#include "fs_bitmap.h"
#include "fs_cluster.h"

/***************************/
/* fs_file helper function */

bool_t fsbit_file_open(FSFILE **fp, const char *name) {
    bool_t exist = fs_file_isfile(name);
    if(!fs_file_open(fp, name)) return false_t;
    if(!exist) {
        byte_t *blank = fs_malloc(BITMAP_SIZE);
        if(!blank) return false_t;
        memset(blank, 0x00, BITMAP_SIZE);
        return fs_file_seek(*fp, BITMAP_HEADER_OFFSET_BYTE) && fs_free(blank, fs_file_write(*fp, blank, BITMAP_SIZE));
    } else
        return true_t;
}

bool_t fsbit_file_read(FSFILE *fp, byte_t *data, fsize_t size) {
    return fs_file_read(fp, data, size);
}

/*
* Bitmaps are prepared for each chunk.
* It is OK if it records the bitmap with the header cluster as 0.
*/
bool_t fsbit_file_write(FSFILE *fp, const byte_t *data, fsize_t size) {
    index_t current=fs_file_getlastseek(fp);
    byte_t *bitmap = fs_malloc(BITMAP_SIZE);
    if(!bitmap) return fs_file_seterror(fp, FILE_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(!(fs_file_seek(fp, BITMAP_HEADER_OFFSET_BYTE) && fs_file_read(fp, bitmap, BITMAP_SIZE)))
        return fs_free(bitmap, fs_file_seterror(fp, FILE_ERROR_DRIVE_RW_FAILURE));
    for(fsize_t i=current/CLUSTER_SIZE/BITS; i<(current+size)/CLUSTER_SIZE/BITS; ++i)
        bitmap[i]|=(1<<(((current+i)/CLUSTER_SIZE)%BITS));
    if(!(fs_file_seek(fp, BITMAP_HEADER_OFFSET_BYTE) && fs_file_write(fp, bitmap, BITMAP_SIZE)))
        return fs_free(bitmap, fs_file_seterror(fp, FILE_ERROR_DRIVE_RW_FAILURE));
    return fs_free(bitmap, fs_file_seek(fp, current) && fs_file_write(fp, data, size));
}

/**********************/
/* fs_bitmap function */

bool_t fs_bitmap_setsuccess(FSBITMAP *bp) {
    bp->status = BITMAP_SUCCESS;
    return true_t;
}

bool_t fs_bitmap_seterror(FSBITMAP *bp, bitmap_status status) {
    bp->status = status;
    return false_t;
}

bool_t fs_bitmap_open(FSBITMAP **bp, FSDISK *fdp) {
    *bp = (FSBITMAP *)fs_malloc(sizeof(FSBITMAP));
    if(!*bp) return false_t;
    (*bp)->fdp = fdp;
    /* insert fs_file helper function */
    (*bp)->fdp->io.fs_file_open = &fsbit_file_open;
    (*bp)->fdp->io.fs_file_read = &fsbit_file_read;
    (*bp)->fdp->io.fs_file_write = &fsbit_file_write;
    return true_t;
}

bool_t fs_bitmap_close(FSBITMAP *bp, bool_t ret) {
    return fs_free(bp, ret);
}

bool_t fs_bitmap_isused(FSBITMAP *bp, const BPB *bpb, cluster_t clus, bool_t *used) {
    sector_t sector = fs_cluster_getsectot(bpb, clus);
    sector = (sector/(fs_file_getsize()/SECTOR_SIZE))*(fs_file_getsize()/SECTOR_SIZE);
    byte_t *buf = fs_malloc(BITMAP_SIZE);
    if(!buf) return fs_bitmap_seterror(bp, BITMAP_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(!fs_disk_read(bp->fdp, sector, BITMAP_SIZE/SECTOR_SIZE, buf)) return fs_bitmap_seterror(bp, BITMAP_ERROR_DRIVE_RW_FAILURE);
    *used = (bool_t)(buf[clus%(CLUSTER_CAPACITY/BITS)] & (1<<0));
    return fs_free(buf, fs_file_setsuccess(bp));
}
