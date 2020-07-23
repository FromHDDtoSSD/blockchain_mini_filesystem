// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BPB
#define SORACHANCOIN_FS_BPB

#define SECTORS_PER_BPB 200 /* 512 * 200 = 102400 bytes */
#define BPB_SIGNATURE "IWAB"

#include "fs_memory.h"
#include "fs_types.h"
#include "fs_const.h"

typedef struct _tag_BLOCKCHAIN_PARAMETER_BLOCK {
    union {
        struct {
            byte_t signature[4]; /* IWAB */
            sector_t bpb_offset;
            cluster_t mft_offset;
            cluster_t index_root_offset;
        };
        struct {
            byte_t unused[SECTOR_SIZE * SECTORS_PER_BPB];
        };
    };
} BPB;



#endif
