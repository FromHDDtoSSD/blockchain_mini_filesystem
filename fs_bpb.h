// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BPB
#define SORACHANCOIN_FS_BPB

#define SECTORS_PER_BPB 32 /* 512 * 32 = 16384 bytes, 32 * 128 = 4096 sectors. */
#define BPB_SIGNATURE "IWAB"

#include "fs_const.h"
#include "fs_memory.h"
#include "fs_types.h"
#include "fs_bitmap.h"

/*
*
* ** fs_bpb **
*
* SORA blockchain-mini-filesystem is a variable length file system that doesn't care about partition size.
* Now that you don't matter how much the block or data grows.
*
*/

#pragma pack(push, 1)
typedef struct _tag_BLOCKCHAIN_PARAMETER_BLOCK {
    union {
        struct {
            byte_t signature[4]; /* IWAB */
            union {
                byte_t hash[32]; /* ver1 SHA256 */
                byte_t qhash[32 * 256]; /* later ver2 QHASH65536 */
            };
            byte_t reserved1[2048];
            sector_t bpb_offset; /* fsindex range. */
            sector_t partition_size; /* always 0. because, variable length file system. */
            cluster_t mft_offset; /* [FILE] master file record table offset */
            cluster_t log_offset; /* [LOGF] logfile table offset */
            cluster_t index_root_offset; /* [INDX] index buffer root offset */
            cluster_t merkle_root_offset; /* [HASH] hash tree root offset */
            cluster_t tft_offset; /* [TOKN] token file record table offset */
            cluster_t bad_clus_offset; /* [BADC] bad cluster list offset */
            cluster_t bad_allocate_offset; /* [BADA] bad allocate chain list offset */
            cluster_t meta_xor_offset; /* [SXOR] meta-data xor table record offset */
            cluster_t data_xor_offset; /* [SXOR] data-area xor table record offset */
            byte_t reserved2[1];
        };
        struct {
            byte_t unused[BYTES_PER_SECTOR*SECTORS_PER_BPB];
        };
    };
} BPB;
#pragma pack(pop)

typedef enum _tag_bpb_status {
    FS_BPB_SUCCESS = 0,
    FS_BPB_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
    FS_BPB_ERROR_DRIVE_RW_FAILURE = 2,
    FS_BPB_ERROR_NO_EMPTY_BPT = 3,
} bpb_status;

typedef struct _tag_FSBPB {
    FSBITMAP *bp;
    BPB bpb_on_memory[NUMOF_BPB];
    bpb_status status;
} FSBPB;

static inline bool_t fs_bpb_setsuccess(FSBPB *bpb) {
    bpb->status=FS_BPB_SUCCESS;
    return b_true;
}

static inline bool_t fs_bpb_seterror(FSBPB *bpb, bpb_status status) {
    bpb->status=status;
    return b_false;
}

static inline bool_t fs_bpb_sigcmp(const BPB *bpb) {
    return memcmp_s(&bpb->signature, sizeof(bpb->signature), BPB_SIGNATURE, sizeof(bpb->signature))==0;
}

static inline bool_t fs_bpb_open(FSBPB **bpb, FSBITMAP *bp) {
    *bpb=(FSBPB *)fs_malloc(sizeof(FSBPB));
    if(!*bpb) return b_false;
    return fs_bpb_setsuccess(bpb);
}

static inline bool_t fs_bpb_close(FSBPB *bpb, bool_t ret) {
    return fs_free(bpb, ret);
}

#endif
