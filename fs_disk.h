// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_DISK
#define SORACHANCOIN_FS_DISK

#include "fs_memory.h"
#include "fs_types.h"
#include "fs_const.h"
#include "fs_code.h"
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
* cluster: The size is SECTOR_SIZE * SECTORS_PER_CLUS and is the access unit, call it with a LBA from mini filesystem.
* chunk: This is one of each file distributed to "fsindex%04d.dat".
*/

#define NO_EXIST_BPB -1

typedef enum _tag_disk_status {
    DISK_SUCCESS = 0,
    DISK_ERROR_PARAM = 1,
    DISK_ERROR_LOCKED = 2,
    DISK_ERROR_MEMORY_ALLOCATE_FAILURE = 3,
    DISK_ERROR_DRIVE_RW_FAILURE = 4,
} disk_status;

typedef struct _tag_DISKIO {
    FSFILE **fp;
    index_t fp_num;
    //index_t fp_current;
    bool_t (*fs_file_open)(FSFILE **fp, const char *name);
    bool_t (*fs_file_read)(FSFILE *fp, byte_t *data, fsize_t size);
    bool_t (*fs_file_write)(FSFILE *fp, const byte_t *data, fsize_t size);
} DISKIO;

typedef struct _tag_FSDISK {
    DISKIO io;
    disk_status status;
} FSDISK;

static inline bool_t fs_disk_setsuccess(FSDISK *fdp) {
    fdp->status = DISK_SUCCESS;
    return true_t;
}

static inline bool_t fs_disk_seterror(FSDISK *fdp, disk_status status) {
    fdp->status = status;
    return false_t;
}

static inline bool_t fs_disk_setf_open(FSDISK *fdp, bool_t (*fs_file_open)(FSFILE **fp, const char *name), bool_t ret) {
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

bool_t fs_disk_open(FSDISK **fdp);
bool_t fs_disk_close(FSDISK *fdp, bool_t ret);
bool_t fs_disk_read(FSDISK *fdp, sector_t begin, counter_t num, byte_t *buf);
bool_t fs_disk_write(FSDISK *fdp, sector_t begin, counter_t num, const byte_t *buf);

#endif
