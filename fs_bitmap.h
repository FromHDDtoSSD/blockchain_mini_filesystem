// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BITMAP
#define SORACHANCOIN_FS_BITMAP

#include "fs_const.h"
#include "fs_types.h"
#include "fs_memory.h"
#include "fs_disk.h"

/*
* ** fs_bitmap **
*
* The Bitmap records position to the used sector.
*
*/

#define BITS_PER_BYTE 8
#define _BITS_PER_SECTOR (BYTES_PER_SECTOR*BITS_PER_BYTE)

typedef enum _tag_bitmap_status {
    FS_BITMAP_SUCCESS = 0,
    FS_BITMAP_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
    FS_BITMAP_ERROR_DRIVE_RW_FAILURE = 2,
    FS_BITMAP_ERROR_OUT_OF_RANGE=3, /* Note: No write bitmap, 0 - 4095 */
} bitmap_status;

typedef struct _tag_FSBITMAP {
    FSDISK *fdp;
    bitmap_status status;
} FSBITMAP;

static inline bool_t fs_bitmap_setsuccess(FSBITMAP *bp) {
    bp->status = FS_BITMAP_SUCCESS;
    return b_true;
}

static inline bool_t fs_bitmap_seterror(FSBITMAP *bp, bitmap_status status) {
    bp->status = status;
    return b_false;
}

static inline bool_t fs_bitmap_open(FSBITMAP **bp, FSDISK *fdp) {
    *bp = (FSBITMAP *)fs_malloc(sizeof(FSBITMAP));
    if(!*bp) return b_false;
    (*bp)->fdp = fdp;
    return fs_bitmap_setsuccess(*bp);
}

static inline bool_t fs_bitmap_close(FSBITMAP *bp, bool_t ret) {
    return fs_free(bp, ret);
}

static inline bool_t fs_diskwith_bitmap_func(FSBITMAP *bp, sector_t begin, counter_t num, void (*mask_func)(sector_t begin, counter_t num, aldstbyte_t *buf, fsize_t bufsize)) {
    if(0<=begin&&begin<_BITS_PER_SECTOR) return fs_bitmap_setsuccess(bp); /* Note: No write bitmap, 0 - 4095 */
    counter_t rwsec=num/_BITS_PER_SECTOR;
    rwsec+=2;
    byte_t *rwbuf=fs_malloc((fsize_t)(rwsec*BYTES_PER_SECTOR));
    if(!rwbuf) return fs_bitmap_seterror(bp, FS_BITMAP_ERROR_MEMORY_ALLOCATE_FAILURE);
    const sector_t metabegin=-1*(begin/_BITS_PER_SECTOR);
    if(!fs_disk_read(bp->fdp, metabegin, rwsec, rwbuf)) return fs_free(rwbuf, fs_bitmap_seterror(bp, FS_BITMAP_ERROR_DRIVE_RW_FAILURE));
    mask_func(begin, num, rwbuf, (fsize_t)(rwsec*BYTES_PER_SECTOR));
    return fs_free(rwbuf, fs_disk_write(bp->fdp, metabegin, rwsec, rwbuf)? fs_bitmap_setsuccess(bp): fs_bitmap_seterror(bp, FS_BITMAP_ERROR_DRIVE_RW_FAILURE));
}

static inline bool_t fs_diskwith_bitmap_read(FSBITMAP *bp, sector_t begin, counter_t num, byte_t *buf) {
    return fs_disk_read(bp->fdp, begin, num, buf)? fs_bitmap_setsuccess(bp): fs_bitmap_seterror(bp, FS_BITMAP_ERROR_DRIVE_RW_FAILURE);
}

static inline void fs_bitmap_setmask(sector_t begin, counter_t num, aldstbyte_t *buf, fsize_t bufsize) { /* from right to left. */
    const foffset_t x=begin;
    const counter_t y=num;
    const fbit_t    z=BITS_PER_BYTE-x%BITS_PER_BYTE; /* z: front bit flags */
    const fbit_t    a=(y-z)%BITS_PER_BYTE; /* a: back bit flags */
    const counter_t b=(y-z)/BITS_PER_BYTE; /* b: inner byte flags */
    const index_t   r=(x%_BITS_PER_SECTOR)/BITS_PER_BYTE; /* r: front byte index */
    const foffset_t q=r+1+b; /* q: back byte index */
    const byte_t    reverse=0xFF>>(BITS_PER_BYTE-z);
    buf[r]|=(reverse<<(BITS_PER_BYTE-z));
    fs_printf("bitmap_setmask: bufsize:%d x:%I64d y:%I64d z:%d a:%d b:%I64d r:%d q:%I64d\n", bufsize, x, y, z, a, b, r, q);
    memset_s(&buf[r+1], bufsize-(r+1), 0xFF, (fsize_t)b);
    buf[q]|=0xFF>>(BITS_PER_BYTE-a);
    assert(b*BITS_PER_BYTE+z+a==num);
}

static inline bool_t fs_diskwith_bitmap_write(FSBITMAP *bp, sector_t begin, counter_t num, const byte_t *buf) {
    if(!fs_disk_write(bp->fdp, begin, num, buf)) return fs_bitmap_seterror(bp, FS_BITMAP_ERROR_DRIVE_RW_FAILURE);
    return fs_diskwith_bitmap_func(bp, begin, num, fs_bitmap_setmask);
}

static inline bool_t fs_bitmap_getmask(FSBITMAP *bp, sector_t sector, bool_t *used) {
    if(0<=sector&&sector<_BITS_PER_SECTOR) {*used=b_true; return fs_bitmap_seterror(bp,FS_BITMAP_ERROR_OUT_OF_RANGE);} /* Note: No write bitmap, 0 - 4095 */
    const foffset_t x=sector;
    const counter_t y=1;
    const sector_t  metabegin=-1*(x/_BITS_PER_SECTOR);
    const index_t   r=(x%_BITS_PER_SECTOR)/BITS_PER_BYTE;
    const fbit_t    a=x%BITS_PER_BYTE;
    byte_t rbuf[BYTES_PER_SECTOR]; /* y=1,[y*BYTES_PER_SECTOR] */
    if(!fs_disk_read(bp->fdp, metabegin, y, rbuf)) return fs_bitmap_seterror(bp,FS_BITMAP_ERROR_DRIVE_RW_FAILURE);
    *used = (bool_t)(rbuf[r]&(1<<a));
    return fs_bitmap_setsuccess(bp);
}

static inline bool_t fs_bitmap_getmask_someusedrange(FSBITMAP *bp, sector_t begin, counter_t num, bool_t *used) {
    if(0<=begin&&begin<_BITS_PER_SECTOR) {*used=b_true; return fs_bitmap_seterror(bp,FS_BITMAP_ERROR_OUT_OF_RANGE);} /* Note: No write bitmap, 0 - 4095 */
    for(sector_t ite=begin; ite<begin+num; ++ite) {
        if(!fs_bitmap_getmask(bp, ite, used)) return b_false;
        if(*used) return fs_bitmap_setsuccess(bp);
    }
    return fs_bitmap_setsuccess(bp);
}

static inline bool_t fs_bitmap_getmask_allusedrange(FSBITMAP *bp, sector_t begin, counter_t num, bool_t *used) {
    if(0<=begin&&begin<_BITS_PER_SECTOR) {*used=b_true; return fs_bitmap_seterror(bp,FS_BITMAP_ERROR_OUT_OF_RANGE);} /* Note: No write bitmap, 0 - 4095 */
    for(sector_t ite=begin; ite<begin+num; ++ite) {
        if(!fs_bitmap_getmask(bp, ite, used)) return b_false;
        if(!*used) return fs_bitmap_setsuccess(bp);
    }
    return fs_bitmap_setsuccess(bp);
}

static inline bool_t fs_bitmap_getmask_freesector(FSBITMAP *bp, counter_t num, sector_t *begin) {
    *begin=_BITS_PER_SECTOR; /* after 4096. */
    sector_t ite=_BITS_PER_SECTOR;
    for(;;ite+=num) {
        bool_t used=b_false;
        if(!fs_bitmap_getmask_someusedrange(bp, ite, num, &used)) {
            disk_status status=fs_disk_getstatus(bp->fdp);
            if(status==FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE) return fs_bitmap_seterror(bp, FS_BITMAP_ERROR_MEMORY_ALLOCATE_FAILURE);
            else break;
        }
        if(!used) {*begin=ite; return fs_bitmap_setsuccess(bp);}
    }
    ite+=num;
    const sector_t n_sector= ite;
    const byte_t empty[BYTES_PER_SECTOR]={0};
    if(!fs_disk_write(bp->fdp, n_sector, 1, empty)) return fs_bitmap_seterror(bp, FS_BITMAP_ERROR_DRIVE_RW_FAILURE);
    *begin=n_sector;
    return fs_bitmap_setsuccess(bp);
}

static inline void fs_bitmap_erasemask(sector_t begin, counter_t num, aldstbyte_t *buf, fsize_t bufsize) { /* from right to left. */
    const foffset_t x=begin;
    const counter_t y=num;
    const fbit_t    z=BITS_PER_BYTE-x%BITS_PER_BYTE; /* z: front bit flags */
    const fbit_t    a=(y-z)%BITS_PER_BYTE; /* a: back bit flags */
    const counter_t b=(y-z)/BITS_PER_BYTE; /* b: inner byte flags */
    const index_t   r=(x%_BITS_PER_SECTOR)/BITS_PER_BYTE; /* r: front byte index */
    const foffset_t q=r+1+b; /* q: back byte index */
    const byte_t    reverse=0xFF>>(BITS_PER_BYTE-z);
    buf[r]&=~(reverse<<(BITS_PER_BYTE-z));
    fs_printf("bitmap_erasemask: bufsize:%d x:%I64d y:%I64d z:%d a:%d b:%I64d r:%d q:%I64d\n", bufsize, x, y, z, a, b, r, q);
    memset_s(&buf[r+1], bufsize-(r+1), 0x00, (fsize_t)b);
    buf[q]&=~(0xFF>>(BITS_PER_BYTE-a));
}

static inline bool_t fs_diskwith_bitmap_erase(FSBITMAP *bp, sector_t begin, counter_t num) {
    return fs_diskwith_bitmap_func(bp, begin, num, fs_bitmap_erasemask);
}

#endif
