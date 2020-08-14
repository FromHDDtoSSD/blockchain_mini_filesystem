// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_DISK
#define SORACHANCOIN_FS_DISK

#include "fs_const.h"
#include "fs_memory.h"
#include "fs_types.h"
#include "fs_file.h"

/*
* ** fs_disk **
*
* These behave like LBA on disk.
*
* In addition, hash calculation is to include certain "side effects" comes to approve. 
* e.g, Implement this into a Blockchain for cluster approval.
*
* sector: The size is SECTOR_SIZE and is the minimum unit, call it with a LBA from 0.
* cluster: The size is SECTOR_SIZE*SECTORS_PER_CLUS and is the access unit, call it with a LBA from mini filesystem.
* chunk: This is one of each file distributed to "fsindex%04d.dat".
*/

#define DISK_SET_ERROR_BY_FILE(fdp, i) fs_disk_seterror((fdp), (fs_file_getstatus((fdp)->io.fp[(i)]) == FS_FILE_ERROR_DRIVE_RW_FAILURE) ? FS_DISK_ERROR_DRIVE_RW_FAILURE : FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE)

static const str_t *metaformat = "%s\\fsimeta%04d.dat"; /* BCR and BPB, include fsmeta.dat, minus index. */
static const str_t *fileformat = "%s\\fsindex%04d.dat"; /* size is fixed: SECTOR_SIZE * SECTORS_PER_CLUS * CLUSTER_CAPACITY */

typedef enum _tag_disk_status {
    FS_DISK_SUCCESS = 0,
    FS_DISK_ERROR_PARAM = 1,
    FS_DISK_ERROR_LOCKED = 2,
    FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE = 3,
    FS_DISK_ERROR_DRIVE_RW_FAILURE = 4,
} disk_status;

typedef struct _tag_DISKIO {
    FSFILE **fmeta;
    FSFILE **fp;
    num_t fmeta_num;
    num_t fp_num;
    str_t dir[MAX_PATH];
    bool_t (*fs_file_open)(FSFILE **fp, const char *path);
    bool_t (*fs_file_read)(FSFILE *fp, byte_t *data, fsize_t size);
    bool_t (*fs_file_write)(FSFILE *fp, const byte_t *data, fsize_t size);
    bool_t (*fs_fmeta_open)(FSFILE **fp, const char *path);
    bool_t (*fs_fmeta_read)(FSFILE *fp, byte_t *data, fsize_t size);
    bool_t (*fs_fmeta_write)(FSFILE *fp, const byte_t *data, fsize_t size);
} DISKIO;

typedef struct _tag_IO_SET_PARAM {
    sector_t begin;
    counter_t fnum;
    FSFILE **ftarget;
    bool_t(*fop)(FSFILE **fp, const char *path);
    bool_t(*fre)(FSFILE *fp, byte_t *data, fsize_t size);
    bool_t(*fwr)(FSFILE *fp, const byte_t *data, fsize_t size);
} IOSETPARAM;

typedef struct _tag_FSDISK {
    DISKIO io;
    disk_status status;
} FSDISK;

static inline bool_t fs_disk_setsuccess(FSDISK *fdp) {
    fdp->status = FS_DISK_SUCCESS;
    return b_true;
}

static inline bool_t fs_disk_seterror(FSDISK *fdp, disk_status status) {
    fdp->status = status;
    return b_false;
}

static inline disk_status fs_disk_getstatus(FSDISK *fdp) {
    return fdp->status;
}

static inline bool_t fs_disk_setf_open(FSDISK *fdp, bool_t (*fs_file_open)(FSFILE **fp, const char *path), bool_t ret) {
    fdp->io.fs_file_open = fs_file_open;
    return ret;
}

static inline bool_t fs_disk_setf_read(FSDISK *fdp, bool_t (*fs_file_read)(FSFILE *fp, byte_t *data, fsize_t size), bool_t ret) {
    fdp->io.fs_file_read = fs_file_read;
    return ret;
}

static inline bool_t fs_disk_setf_write(FSDISK *fdp, bool_t (*fs_file_write)(FSFILE *fp, const byte_t *data, fsize_t size), bool_t ret) {
    fdp->io.fs_file_write = fs_file_write;
    return ret;
}

static inline bool_t fs_disk_open(FSDISK **fdp, const str_t *dir) {
    num_t num=0, meta=0;
    bool_t one_exist = b_false, meta_exist = b_false;
    {
        str_t path[MAX_PATH];
        sprintf_s(path, ARRAYLEN(path), fileformat, dir, 1);
        one_exist = fs_file_isfile(path);
        sprintf_s(path, ARRAYLEN(path), metaformat, dir, 1);
        meta_exist = fs_file_isfile(path);
    }
    if(one_exist) {
        num = 1;
        for(index_t i=2;;++i) {
            str_t path[MAX_PATH];
            sprintf_s(path, ARRAYLEN(path), fileformat, dir, i);
            if(fs_file_isfile(path)) ++num;
            else break;
        }
    }
    if(meta_exist) {
        meta = 1;
        for(index_t i=2;;++i) {
            str_t path[MAX_PATH];
            sprintf_s(path, ARRAYLEN(path), metaformat, dir, i);
            if(fs_file_isfile(path)) ++meta;
            else break;
        }
    }
    *fdp = (FSDISK *)fs_malloc(sizeof(FSDISK));
    if(!*fdp) return b_false;
    strcpy_s((*fdp)->io.dir, ARRAYLEN((*fdp)->io.dir), dir);
    if(num > 0) {
        (*fdp)->io.fp = (FSFILE **)fs_malloc((fsize_t)(sizeof(FSFILE *) * num));
        if(!(*fdp)->io.fp) return fs_free(*fdp, fs_disk_seterror(*fdp, FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE));
        (*fdp)->io.fp_num = num;
        (*fdp)->io.fs_file_open = &fs_file_open;
        (*fdp)->io.fs_file_read = &fs_file_read;
        (*fdp)->io.fs_file_write = &fs_file_write;
        for(index_t i=0; i < num; ++i) {
            str_t path[MAX_PATH];
            sprintf_s(path, ARRAYLEN(path), fileformat, dir, i + 1);
            if(!(*fdp)->io.fs_file_open(&(*fdp)->io.fp[i], path)) return DISK_SET_ERROR_BY_FILE(*fdp, i);
        }
    } else {
        (*fdp)->io.fp = (FSFILE **)fs_malloc(sizeof(FSFILE *) * 1);
        if(!(*fdp)->io.fp) return fs_free(*fdp, fs_disk_seterror(*fdp, FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE));
        (*fdp)->io.fp_num = 1;
        (*fdp)->io.fs_file_open = &fs_file_open;
        (*fdp)->io.fs_file_read = &fs_file_read;
        (*fdp)->io.fs_file_write = &fs_file_write;
        str_t path[MAX_PATH];
        sprintf_s(path, ARRAYLEN(path), fileformat, dir, 1);
        if(!(*fdp)->io.fs_file_open(&(*fdp)->io.fp[0], path)) return DISK_SET_ERROR_BY_FILE(*fdp, 0);
    }
    if(meta > 0) {
        (*fdp)->io.fmeta = (FSFILE **)fs_malloc((fsize_t)(sizeof(FSFILE *) * meta));
        if(!(*fdp)->io.fmeta) return fs_free(*fdp, fs_disk_seterror(*fdp, FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE));
        (*fdp)->io.fmeta_num = meta;
        (*fdp)->io.fs_fmeta_open = &fs_file_open;
        (*fdp)->io.fs_fmeta_read = &fs_file_read;
        (*fdp)->io.fs_fmeta_write = &fs_file_write;
        for(index_t i=0; i < meta; ++i) {
            str_t path[MAX_PATH];
            sprintf_s(path, ARRAYLEN(path), metaformat, dir, i + 1);
            if(!(*fdp)->io.fs_fmeta_open(&(*fdp)->io.fmeta[i], path)) return DISK_SET_ERROR_BY_FILE(*fdp, i);
        }
    } else {
        (*fdp)->io.fmeta = (FSFILE **)fs_malloc(sizeof(FSFILE *) * 1);
        if(!(*fdp)->io.fmeta) return fs_free(*fdp, fs_disk_seterror(*fdp, FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE));
        (*fdp)->io.fmeta_num = 1;
        (*fdp)->io.fs_fmeta_open = &fs_file_open;
        (*fdp)->io.fs_fmeta_read = &fs_file_read;
        (*fdp)->io.fs_fmeta_write = &fs_file_write;
        str_t path[MAX_PATH];
        sprintf_s(path, ARRAYLEN(path), metaformat, dir, 1);
        if(!(*fdp)->io.fs_fmeta_open(&(*fdp)->io.fmeta[0], path)) return DISK_SET_ERROR_BY_FILE(*fdp, 0);
    }
    return fs_disk_setsuccess(*fdp);
}

static inline bool_t fs_disk_close(FSDISK *fdp, bool_t ret) {
    for(index_t i=0; i < fdp->io.fmeta_num; ++i)
        fs_file_close(fdp->io.fmeta[i], b_true);
    for(index_t i=0; i < fdp->io.fp_num; ++i)
        fs_file_close(fdp->io.fp[i], b_true);
    return fs_free(fdp, fs_free(fdp->io.fp, fs_free(fdp->io.fmeta, ret)));
}

static inline bool_t fs_disk_read(FSDISK *fdp, const sector_t begin, counter_t num, aldstbyte_t *buf) {
    IOSETPARAM param;
    param.ftarget=(begin>=0)? fdp->io.fp: fdp->io.fmeta;
    param.fnum=(begin>=0)? fdp->io.fp_num: fdp->io.fmeta_num;
    param.begin=(begin>=0)? begin: -1*begin;
    /* param.fop=(begin>=0)? fdp->io.fs_file_open: fdp->io.fs_fmeta_open; */
    param.fre=(begin>=0)? fdp->io.fs_file_read: fdp->io.fs_fmeta_read;
    /* param.fwr=(begin>=0)? fdp->io.fs_file_write: fdp->io.fs_fmeta_write; */
    const fsize_t fsize = fs_file_getsize();
    const llsize_t rbegin = param.begin * BYTES_PER_SECTOR;
    llsize_t remain = num * BYTES_PER_SECTOR;
    for(index_t i=(index_t)(rbegin/fsize); i < param.fnum; ++i) {
        foffset_t offset = (i==rbegin/fsize) ? rbegin-(((index_t)(rbegin/fsize))*fsize): 0;
        fsize_t rsize = (fsize_t)((remain > fsize-offset) ? fsize - offset: remain);
        if(!fs_file_seek(param.ftarget[i], (index_t)offset)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        if(!param.fre(param.ftarget[i], buf, rsize)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        buf += rsize;
        if((remain -= rsize)==0) break;
        if(i+1==param.fnum && remain > 0) return fs_disk_seterror(fdp, FS_DISK_ERROR_PARAM);
    }
    return fs_disk_setsuccess(fdp);
}

static inline bool_t fs_disk_write(FSDISK *fdp, const sector_t begin, counter_t num, const byte_t *buf) {
    IOSETPARAM param;
    param.ftarget=(begin>=0)? fdp->io.fp: fdp->io.fmeta;
    param.fnum=(begin>=0)? fdp->io.fp_num: fdp->io.fmeta_num;
    param.begin=(begin>=0)? begin: -1*begin;
    param.fop=(begin>=0)? fdp->io.fs_file_open: fdp->io.fs_fmeta_open;
    /* param.fre=(begin>=0)? fdp->io.fs_file_read: fdp->io.fs_fmeta_read; */
    param.fwr=(begin>=0)? fdp->io.fs_file_write: fdp->io.fs_fmeta_write;
    const fsize_t fsize = fs_file_getsize();
    const llsize_t wbegin = param.begin * BYTES_PER_SECTOR;
    llsize_t remain = num * BYTES_PER_SECTOR;
    const index_t reqfile = (index_t)((wbegin + remain)/fsize + (((wbegin + remain)%fsize!=0) ? 1: 0));
    for(index_t i=(index_t)param.fnum; i < reqfile; ++i) {
        if(i==param.fnum) {
            const fsize_t alsize=sizeof(FSFILE *) * reqfile;
            FSFILE **tmp = (FSFILE **)fs_malloc(alsize);
            if(!tmp) return fs_disk_seterror(fdp, FS_DISK_ERROR_MEMORY_ALLOCATE_FAILURE);
            if(begin>=0) {
                memcpy_s(tmp, alsize, fdp->io.fp, sizeof(FSFILE *) * fdp->io.fp_num);
                fs_free(fdp->io.fp, b_true);
                fdp->io.fp = tmp;
                fdp->io.fp_num = reqfile;
                param.ftarget = fdp->io.fp;
                param.fnum = fdp->io.fp_num;
            } else {
                memcpy_s(tmp, alsize, fdp->io.fmeta, sizeof(FSFILE *) * fdp->io.fmeta_num);
                fs_free(fdp->io.fmeta, b_true);
                fdp->io.fmeta = tmp;
                fdp->io.fmeta_num = reqfile;
                param.ftarget = fdp->io.fmeta;
                param.fnum = fdp->io.fmeta_num;
            }
        }
        if(begin>=0) {
            str_t path[MAX_PATH];
            sprintf_s(path, ARRAYLEN(path), fileformat, fdp->io.dir, i + 1);
            if(!param.fop(&param.ftarget[i], path)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        } else {
            str_t path[MAX_PATH];
            sprintf_s(path, ARRAYLEN(path), metaformat, fdp->io.dir, i + 1);
            if(!param.fop(&param.ftarget[i], path)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        }
    }
    for(index_t i=(index_t)(wbegin/fsize); i < param.fnum; ++i) {
        foffset_t offset = (i==wbegin/fsize) ? wbegin-(((index_t)(wbegin/fsize))*fsize): 0;
        fsize_t wsize = (fsize_t)((remain > fsize-offset) ? fsize - offset: remain);
        assert(wsize<=fsize);
        assert(offset<fsize);
        assert(offset+wsize<=fsize);
        if(!fs_file_seek(param.ftarget[i], (index_t)offset)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        if(!param.fwr(param.ftarget[i], buf, wsize)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        buf += wsize;
        if((remain -= wsize)==0) break;
        if(i+1==param.fnum && remain > 0) return fs_disk_seterror(fdp, FS_DISK_ERROR_PARAM);
    }
    return fs_disk_setsuccess(fdp);
}

#endif
