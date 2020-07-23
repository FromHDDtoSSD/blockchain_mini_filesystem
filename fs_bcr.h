// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BCR
#define SORACHANCOIN_FS_BCR

#include "fs_memory.h"
#include "fs_disk.h"
#include "fs_types.h"
#include "fs_const.h"
#include "fs_bitmap.h"

#define SECTORS_PER_BPT 20 /* 512 * 20 = 10240 bytes */
#define SECTORS_PER_BCR 200 /* 512 * 200 = 102400 bytes */
#define BCR_SIGNATURE "SORA" /* SORA is calico cat. */
#define BCR_BEGIN_SECTOR (BITMAP_SIZE/SECTOR_SIZE) /* BITMAP,BCR ... */

/*
* ** fs_bcr **
*
* BCR is embed in the first chunk.
*/

typedef enum _tab_bpt_type {
    bpt_blank = (byte_t)0x00,
    bpt_ver1 = (byte_t)0x01,
} bpt_type;

#pragma pack(push, 1)
typedef struct _tag_BLOCKCHAIN_PARTITION_TABLE {
    union {
        struct {
            union {
                byte_t hash[32]; /* ver1 SHA256 */
                byte_t qhash[32 * 256]; /* later ver2 QHASH65536 */
            };
            bpt_type types;
            byte_t padding[sizeof(byte_t)*4 - sizeof(byte_t)];
            sector_t bpb_offset;
            sector_t sectors;
            byte_t reserved[1];
        };
        struct {
            byte_t unused[SECTOR_SIZE * SECTORS_PER_BPT];
        };
    };
} BPT;

typedef struct _tag_BLOCKCHAIN_RECORD {
    union {
        struct {
            byte_t signature[4]; /* SORA */
            BPT table[4]; /* ver1 is only using [0]. */
            byte_t reserved[1];
        };
        struct {
            byte_t unused[SECTOR_SIZE * SECTORS_PER_BCR];
        };
    };
} BCR;
#pragma pack(pop)

typedef enum _tag_bcr_status {
    BCR_SUCCESS = 0,
    BCR_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
    BCR_ERROR_DRIVE_RW_FAILURE = 2,
} bcr_status;

typedef struct _tag_FSBCR {
    FSDISK *fdp;
    bcr_status status;
} FSBCR;

static inline bool_t fs_bcr_setsuccess(FSBCR *fbr) {
    fbr->status = BCR_SUCCESS;
    return true_t;
}

static inline bool_t fs_bcr_seterror(FSBCR *fbr, bcr_status status) {
    fbr->status = status;
    return false_t;
}

static inline void fs_bcr_initbpt(BPT *p) { /* ver1 */
    memset(p->unused, 0x00, sizeof(p->unused));
    p->types = bpt_ver1;
    p->bpb_offset = sizeof(BCR) * sizeof(byte_t) / SECTOR_SIZE;
    p->sectors = 0; /* variable length */
}

static inline void fs_bcr_initbcr(BCR *p) {
    memset(p->unused, 0x00, sizeof(p->unused));
    memcpy(p->signature, BCR_SIGNATURE, sizeof(p->signature));
    fs_bcr_initbpt(&p->table[0]); /* ver1 */
}

static inline bool_t fs_bcr_open(FSBCR **fbr, FSDISK *fdp) {
    *fbr = (FSBCR *)fs_malloc(sizeof(FSBCR));
    if(!*fbr) return false_t;
    (*fbr)->fdp = fdp;
    byte_t *buf = fs_malloc(sizeof(BCR));
    if(!buf) return fs_bcr_seterror(*fbr, BCR_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(!fs_disk_read((*fbr)->fdp, BCR_BEGIN_SECTOR, sizeof(BCR)/SECTOR_SIZE, buf)) return fs_bcr_seterror(*fbr, BCR_ERROR_DRIVE_RW_FAILURE);
    if(memcmp(((const BCR *)buf)->signature, BCR_SIGNATURE, sizeof((const BCR *)buf)->signature * sizeof(byte_t)) != 0) {
        fs_bcr_initbcr((BCR *)buf);
        return fs_free(buf, fs_disk_write((*fbr)->fdp, BCR_BEGIN_SECTOR, sizeof(BCR)/SECTOR_SIZE, buf) ? fs_bcr_setsuccess(*fbr): fs_bcr_seterror(*fbr, BCR_ERROR_DRIVE_RW_FAILURE));
    } else
        return fs_free(buf, fs_bcr_setsuccess(*fbr));
}

static inline bool_t fs_bcr_close(FSBCR *fbr, bool_t ret) {
    return fs_free(fbr, ret);
}

static inline bool_t fs_bcr_read(FSBCR *fbr, BCR *buf) {
    return fs_disk_read(fbr->fdp, BCR_BEGIN_SECTOR, sizeof(BCR)/SECTOR_SIZE, (byte_t *)buf) ? fs_bcr_setsuccess(fbr): fs_bcr_seterror(fbr, BCR_ERROR_DRIVE_RW_FAILURE);
}

#endif
