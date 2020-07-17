// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fs_file.h"
#include "fs_disk.h"

static const str_t *fs_disk_fileformat = "D:\\fsdisk\\fsindex%04d.dat"; /* size is fixed: CLUSTER_SIZE * CLUSTER_CAPACITY */

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
    if(num > 0) {
        (*fdp)->io.fp = (FSFILE **)fs_malloc(sizeof(FSFILE *) * num);
        if(!(*fdp)->io.fp) return fs_disk_seterror(*fdp, FILE_ERROR_MEMORY_ALLOCATE_FAILURE);
        (*fdp)->io.fp_num = num;
        (*fdp)->io.fp_current = 0;
        counter_t index=0;
        for(index_t i=0; i < num; ++i) {
            str_t path[MAX_PATH];
            sprintf(path, fs_disk_fileformat, i + 1);
            if(!fs_file_open(&(*fdp)->io.fp[i], path))
                return fs_disk_seterror(*fdp, (fs_file_getstatus((*fdp)->io.fp[i]) == FILE_ERROR_DRIVE_RW_FAILURE) ? DISK_ERROR_DRIVE_RW_FAILURE : DISK_ERROR_MEMORY_ALLOCATE_FAILURE);
        }
        return fs_disk_setsuccess(*fdp);
    } else {
        (*fdp)->io.fp = (FSFILE **)fs_malloc(sizeof(FSFILE *) * 1);
        if(!(*fdp)->io.fp) return fs_disk_seterror(*fdp, FILE_ERROR_MEMORY_ALLOCATE_FAILURE);
        (*fdp)->io.fp_num = 1;
        (*fdp)->io.fp_current = 0;
        str_t path[MAX_PATH];
        sprintf(path, fs_disk_fileformat, 1);
        if(!fs_file_open(&(*fdp)->io.fp[0], path))
            return fs_disk_seterror(*fdp, (fs_file_getstatus((*fdp)->io.fp[0]) == FILE_ERROR_DRIVE_RW_FAILURE) ? DISK_ERROR_DRIVE_RW_FAILURE: DISK_ERROR_MEMORY_ALLOCATE_FAILURE);
        else
            return fs_disk_setsuccess(*fdp);
    }
}

bool_t fs_disk_close(FSDISK *fdp, bool_t ret) {
    for(index_t i=0; i < fdp->io.fp_num; ++i) {
        fs_file_close(fdp->io.fp[i], true_t);
    }
    return fs_free(fdp->io.fp, ret);
}

bool_t fs_disk_read(FSDISK *fdp, sector_t begin, counter_t num, byte_t *buf) {

    return DISK_SUCCESS;
}

bool_t fs_disk_write(FSDISK *fdp, sector_t begin, counter_t num, const byte_t *buf) {

    return DISK_SUCCESS;
}
