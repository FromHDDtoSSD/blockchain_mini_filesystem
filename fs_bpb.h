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

/*
*
* ** fs_bpb **
*
* SORA blockchain-mini-filesystem is a variable length file system that doesn't care about partition size.
* Now that you don't matter how much the block or data grows.
* 
* SORA blockchain-mini-filesystem structure:
* [BITMAP(sector 0)][BCR][BPB(cluster 0)] --- [MFT][LOG][INDEX][MERKLE][TOKEN] --- [XOR][xor dara] --- [data area] --- [XOR][xor data]
*/

typedef struct _tag_BLOCKCHAIN_PARAMETER_BLOCK {
    union {
        struct {
            byte_t signature[4]; /* IWAB */
            union {
                byte_t hash[32]; /* ver1 SHA256 */
                byte_t qhash[32 * 256]; /* later ver2 QHASH65536 */
            };
            byte_t reserved1[20480];
            sector_t bpb_offset;
            sector_t partition_size; /* always 0. because, variable length file system. */
            cluster_t mft_offset; /* [FILE] master file record table offset */
            cluster_t log_offset; /* [LOGF] logfile table offset */
            cluster_t index_root_offset; /* [INDX] index buffer root offset */
            cluster_t merkle_root_offset; /* [HASH] hash tree root offset */
            cluster_t tft_offset; /* [TOKN] token file record table offset */
            cluster_t meta_xor_offset; /* [SXOR] meta-data xor table record offset */
            cluster_t data_xor_offset; /* [SXOR] data-area xor table record offset */
            byte_t reserved2[1];
        };
        struct {
            byte_t unused[SECTOR_SIZE * SECTORS_PER_BPB];
        };
    };
} BPB;

typedef enum _tag_bpb_status {
    BPB_SUCCESS = 0,
    BPB_ERROR_MEMORY_ALLOCATE_FAILURE = 1, 
} bpb_status;

#endif
