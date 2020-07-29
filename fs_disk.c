// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fs_file.h"
#include "fs_disk.h"

static const str_t *fs_disk_fileformat = "D:\\fsdisk\\fsindex%04d.dat"; /* size is fixed: SECTOR_SIZE * SECTORS_PER_CLUS * CLUSTER_CAPACITY */
#define DISK_SET_ERROR_BY_FILE(fdp, i) fs_disk_seterror((fdp), (fs_file_getstatus((fdp)->io.fp[(i)]) == FILE_ERROR_DRIVE_RW_FAILURE) ? DISK_ERROR_DRIVE_RW_FAILURE : DISK_ERROR_MEMORY_ALLOCATE_FAILURE)

bool_t fs_disk_open(FSDISK **fdp) {
    counter_t num=0;
    bool_t one_exist = false_t;
    {
        str_t path[MAX_PATH];
        sprintf(path, fs_disk_fileformat, 1);
        one_exist = fs_file_isfile(path);
    }
    if(one_exist) {
        num = 1;
        for(index_t i=2;;++i) {
            str_t path[MAX_PATH];
            sprintf(path, fs_disk_fileformat, i);
            if(fs_file_isfile(path)) ++num;
            else break;
        }
    }
    *fdp = (FSDISK *)fs_malloc(sizeof(FSDISK));
    if(!*fdp) return false_t;
    if(num > 0) {
        (*fdp)->io.fp = (FSFILE **)fs_malloc(sizeof(FSFILE *) * num);
        if(!(*fdp)->io.fp) return fs_free(*fdp, fs_disk_seterror(*fdp, DISK_ERROR_MEMORY_ALLOCATE_FAILURE));
        (*fdp)->io.fp_num = num;
        //(*fdp)->io.fp_current = 0;
        (*fdp)->io.fs_file_open = &fs_file_open;
        (*fdp)->io.fs_file_read = &fs_file_read;
        (*fdp)->io.fs_file_write = &fs_file_write;
        counter_t index=0;
        for(index_t i=0; i < num; ++i) {
            str_t path[MAX_PATH];
            sprintf(path, fs_disk_fileformat, i + 1);
            if(!(*fdp)->io.fs_file_open(&(*fdp)->io.fp[i], path))
                return DISK_SET_ERROR_BY_FILE(*fdp, i);
        }
        return fs_disk_setsuccess(*fdp);
    } else {
        (*fdp)->io.fp = (FSFILE **)fs_malloc(sizeof(FSFILE *) * 1);
        if(!(*fdp)->io.fp) return fs_free(*fdp, fs_disk_seterror(*fdp, DISK_ERROR_MEMORY_ALLOCATE_FAILURE));
        (*fdp)->io.fp_num = 1;
        //(*fdp)->io.fp_current = 0;
        (*fdp)->io.fs_file_open = &fs_file_open;
        (*fdp)->io.fs_file_read = &fs_file_read;
        (*fdp)->io.fs_file_write = &fs_file_write;
        str_t path[MAX_PATH];
        sprintf(path, fs_disk_fileformat, 1);
        if(!(*fdp)->io.fs_file_open(&(*fdp)->io.fp[0], path))
            return DISK_SET_ERROR_BY_FILE(*fdp, 0);
        else
            return fs_disk_setsuccess(*fdp);
    }
}

bool_t fs_disk_close(FSDISK *fdp, bool_t ret) {
    for(index_t i=0; i < fdp->io.fp_num; ++i) {
        fs_file_close(fdp->io.fp[i], true_t);
    }
    return fs_free(fdp, fs_free(fdp->io.fp, ret));
}

bool_t fs_disk_read(FSDISK *fdp, sector_t begin, counter_t num, byte_t *buf) {
    const fsize_t fsize = fs_file_getsize();
    const fsize_t rbegin = begin * SECTOR_SIZE;
    fsize_t remain = num * SECTOR_SIZE;
    for(index_t i=rbegin/fsize; i < fdp->io.fp_num; ++i) {
        fsize_t offset = (i==rbegin/fsize) ? rbegin-(((index_t)(rbegin/fsize))*fsize): 0;
        fsize_t rsize = (remain > fsize-offset) ? fsize - offset: remain;
        if(!fs_file_seek(fdp->io.fp[i], offset)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        if(!fdp->io.fs_file_read(fdp->io.fp[i], buf, rsize)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        buf += rsize;
        if((remain -= rsize)==0) break;
        if(i+1==fdp->io.fp_num && remain > 0) return fs_disk_seterror(fdp, DISK_ERROR_PARAM);
    }
    return fs_disk_setsuccess(fdp, DISK_SUCCESS);
}

bool_t fs_disk_write(FSDISK *fdp, sector_t begin, counter_t num, const byte_t *buf) {
    const fsize_t fsize = fs_file_getsize();
    const fsize_t wbegin = begin * SECTOR_SIZE;
    fsize_t remain = num * SECTOR_SIZE;
    const index_t reqfile = (wbegin + remain)/fsize + (((wbegin + remain)%fsize!=0) ? 1: 0);
    index_t prev_fp_num = fdp->io.fp_num;
    for(index_t i=fdp->io.fp_num; i < reqfile; ++i) {
        if(i==fdp->io.fp_num) {
            FSFILE **tmp = (FSFILE **)fs_malloc(sizeof(FSFILE *) * reqfile);
            if(!tmp) return fs_disk_seterror(fdp, DISK_ERROR_MEMORY_ALLOCATE_FAILURE);
            memcpy(tmp, fdp->io.fp, sizeof(FSFILE *) * fdp->io.fp_num);
            fs_free(fdp->io.fp, true_t);
            fdp->io.fp = tmp;
            fdp->io.fp_num = reqfile;
        }
        str_t path[MAX_PATH];
        sprintf(path, fs_disk_fileformat, i + 1);
        if(!fdp->io.fs_file_open(&fdp->io.fp[i], path)) return DISK_SET_ERROR_BY_FILE(fdp, i);
    }
    for(index_t i=prev_fp_num; i < reqfile; ++i) {
        FSFILE *fpn = fdp->io.fp[i];
        fpn->bpb_offset = NO_EXIST_BPB;
    }
    for(index_t i=wbegin/fsize; i < fdp->io.fp_num; ++i) {
        fsize_t offset = (i==wbegin/fsize) ? wbegin-(((index_t)(wbegin/fsize))*fsize): 0;
        fsize_t wsize = (remain > fsize-offset) ? fsize - offset: remain;
        assert(wsize<=fsize);
        assert(offset<fsize);
        assert(offset+wsize<=fsize);
        if(!fs_file_seek(fdp->io.fp[i], offset)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        if(!fdp->io.fs_file_write(fdp->io.fp[i], buf, wsize)) return DISK_SET_ERROR_BY_FILE(fdp, i);
        buf += wsize;
        if((remain -= wsize)==0) break;
        if(i+1==fdp->io.fp_num && remain > 0) return fs_disk_seterror(fdp, DISK_ERROR_PARAM);
    }
    return fs_disk_setsuccess(fdp, DISK_SUCCESS);
}
