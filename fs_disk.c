// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "fs_disk.h"

typedef struct _tag_disk_info {
    union {
        struct _in_disk {
            sector_t begin_iterator;
            sector_t end_iterator;
        };
        struct _in_memory {
            byte_t data[1];
        };
    };
} disk_info;

static const str_t *fpath = "D:\\fsdisk";
static const str_t *ffileformat = "fsindex%d.dat"; /* size is fixed: CLUSTER_SIZE * CLUSTER_CAPACITY */
static disk_info **fptr = NULL;
static counter_t fptrbegin = 0;
static counter_t fptrend = 0;
static counter_t fptrallocated = 0;

io_status fs_io_init() {

    return IO_SUCCESS;
}

void fs_io_destroy() {
    if(fptr) {
        for(int i=0; i < fptrend; ++i) {
            fs_free(fptr[i], true_t);
        }
        fs_free(fptr, true_t);
        fptr = NULL;
    }
}

io_status fs_diskread(sector_t begin, sector_t nums) {

    return IO_SUCCESS;
}

io_status fs_diskwrite(sector_t begin, sector_t nums) {

    return IO_SUCCESS;
}
