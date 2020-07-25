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

bool_t fsbit_file_read(FSFILE *fp, byte_t *const data, fsize_t size) {
    return fs_file_read(fp, data, size);
}

/*
* Bitmaps are prepared for each chunk.
*/
bool_t fsbit_file_write(FSFILE *fp, const byte_t *const data, fsize_t size) {
    const index_t current=fs_file_getlastseek(fp);
    byte_t *bitmap = fs_malloc(BITMAP_SIZE);
    if(!bitmap) return fs_file_seterror(fp, FILE_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(!(fs_file_seek(fp, BITMAP_HEADER_OFFSET_BYTE) && fs_file_read(fp, bitmap, BITMAP_SIZE)))
        return fs_free(bitmap, fs_file_seterror(fp, FILE_ERROR_DRIVE_RW_FAILURE));
    assert(current-BITMAP_SIZE>=0); /* Note: The assertion here if you include a Bitmap (When use FSBITMAP). If you stop here, the Bitmap is corrupted. */
    const cluster_t bpb_offset = (fp->bpb_offset!=NO_EXIST_BPB) ? fp->bpb_offset/SECTORS_PER_CLUS: 0;
    const cluster_t begin_clus = (current-((fp->bpb_offset==NO_EXIST_BPB) ? BITMAP_SIZE: 0))/CLUSTER_SIZE - bpb_offset;
    const cluster_t end_clus = begin_clus+size/CLUSTER_SIZE+((size%CLUSTER_SIZE>0)?1:0);
    for(fsize_t i=begin_clus; i<=end_clus; ++i)
        bitmap[i]=(byte_t)0x01;
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

bool_t fs_bitmap_open(FSBITMAP **bp, const BPB *bpb, FSDISK *fdp) {
    *bp = (FSBITMAP *)fs_malloc(sizeof(FSBITMAP));
    if(!*bp) return false_t;
    (*bp)->fdp = fdp;
    (*bp)->bpb = bpb;
    for(index_t i=0; i<(*bp)->fdp->io.fp_num; ++i) {
        FSFILE *fp=(*bp)->fdp->io.fp[i];
        fp->bpb_offset = NO_EXIST_BPB;
    }
    index_t bpb_file_target = bpb->bpb_offset/SECTOR_CAPACITY;
    FSFILE *fs_bpb_insert = (*bp)->fdp->io.fp[bpb_file_target];
    fs_bpb_insert->bpb_offset = bpb->bpb_offset;
    /* insert fs_file helper function */
    (*bp)->fdp->io.fs_file_open = &fsbit_file_open;
    (*bp)->fdp->io.fs_file_read = &fsbit_file_read;
    (*bp)->fdp->io.fs_file_write = &fsbit_file_write;
    return fs_bitmap_setsuccess(*bp);
}

bool_t fs_bitmap_close(FSBITMAP *bp, bool_t ret) {
    return fs_free(bp, ret);
}

bool_t fs_bitmap_isused(FSBITMAP *bp, cluster_t clus, bool_t *used) {
    sector_t begin_sector = fs_cluster_getsector(bp->bpb, clus);
    begin_sector /= SECTOR_CAPACITY;
    begin_sector *= SECTOR_CAPACITY;
    byte_t *buf = fs_malloc(BITMAP_SIZE);
    if(!buf) return fs_bitmap_seterror(bp, BITMAP_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(!fs_disk_read(bp->fdp, begin_sector, SECTORS_PER_BITMAP, buf)) return fs_free(buf, fs_bitmap_seterror(bp, BITMAP_ERROR_DRIVE_RW_FAILURE));
    cluster_t pos=fs_cluster_getrelativeclus(bp->bpb, clus);
    assert(pos>=0);
    *used = (bool_t)buf[pos];
    return fs_free(buf, fs_bitmap_setsuccess(bp));
}
