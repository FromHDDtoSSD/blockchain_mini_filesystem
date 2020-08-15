// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BCR
#define SORACHANCOIN_FS_BCR

#include "fs_const.h"
#include "fs_memory.h"
#include "fs_types.h"
#include "fs_bpb.h"
#include "fs_bitmap.h"

#define SECTORS_PER_BPT 20   /* 512 * 20 = 10240 bytes */
#define SECTORS_PER_BCR 4096 /* SECTORS_PER?CHUNK/2 */
#define BCR_SIGNATURE "SORA" /* SORA is calico cat. */
#define BCR_BEGIN_SECTOR 0

/*
* ** fs_bcr **
*
* BCR is embed in the "fsindex0000.dat", [sector: 0 - 4095]
*
*/

typedef enum _tab_bpt_type {
    bpt_unused = 0x00,
    bpt_ver1 = 0x01,
    bpt_ver2 = 0x02,
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
            sector_t bpb_offset; /* fsimeta range. */
            sector_t sectors; /* variable length, always 0. */
            byte_t reserved[1];
        };
        struct {
            byte_t unused[BYTES_PER_SECTOR*SECTORS_PER_BPT];
        };
    };
} BPT;
typedef struct _tag_BLOCKCHAIN_RECORD {
    union {
        struct {
            byte_t signature[4]; /* SORA */
            BPT table[NUMOF_BPB]; /* It needs as many BPT as there are BPB. */
            byte_t reserved[1];
        };
        struct {
            byte_t unused[BYTES_PER_SECTOR*SECTORS_PER_BCR];
        };
    };
} BCR;
#pragma pack(pop)

typedef enum _tag_bcr_status {
    FS_BCR_SUCCESS = 0,
    FS_BCR_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
    FS_BCR_ERROR_DRIVE_RW_FAILURE = 2,
    FS_BCR_ERROR_NO_EMPTY_BPT = 3,
} bcr_status;

typedef struct _tag_FSBCR {
    FSBITMAP *bp;
    BCR bcr_on_memory;
    bcr_status status;
} FSBCR;

static inline bool_t fs_bcr_setsuccess(FSBCR *fbr) {
    fbr->status = FS_BCR_SUCCESS;
    return b_true;
}

static inline bool_t fs_bcr_seterror(FSBCR *fbr, bcr_status status) {
    fbr->status = status;
    return b_false;
}

static inline BCR *fs_bcr_getbcr(FSBCR *fbr) {
    return &fbr->bcr_on_memory;
}

static inline bool_t fs_bcr_diskread(FSBCR *fbr) {
    return fs_disk_read(fbr->bp->fdp, BCR_BEGIN_SECTOR, SECTORS_PER_BCR, (byte_t *)&fbr->bcr_on_memory)? fs_bcr_setsuccess(fbr): fs_bcr_seterror(fbr, FS_BCR_ERROR_DRIVE_RW_FAILURE);
}

static inline bool_t fs_bcr_diskwrite(FSBCR *fbr) {
    return fs_disk_write(fbr->bp->fdp, BCR_BEGIN_SECTOR, SECTORS_PER_BCR, (const byte_t *)&fbr->bcr_on_memory)? fs_bcr_setsuccess(fbr): fs_bcr_seterror(fbr, FS_BCR_ERROR_DRIVE_RW_FAILURE);
}

static inline bool_t fs_bcr_sigcmp(FSBCR *fbr) {
    return memcmp_s(&fbr->bcr_on_memory.signature, sizeof(fbr->bcr_on_memory.signature), BCR_SIGNATURE, sizeof(fbr->bcr_on_memory.signature))==0;
}

static inline bool_t fs_bcr_initbpt(FSBCR *fbr, BPT *bpt) { /* [SORA] BPT first[0] create. */
    bpt->types = bpt_ver1;
    if(!fs_bitmap_getmask_freesector(fbr->bp, sizeof(BPB)/BYTES_PER_SECTOR, &bpt->bpb_offset)) fs_bcr_seterror(fbr, FS_BCR_ERROR_DRIVE_RW_FAILURE);
    bpt->sectors = 0; /* always 0: variable length */
    
    // BPB create: bpt->bpb_offset



    return fs_bcr_setsuccess(fbr);
}

static inline bool_t fs_bcr_initbcr(FSBCR *fbr) { /* [SORA] BCR create. */
    memset_s(fbr->bcr_on_memory.unused, sizeof(fbr->bcr_on_memory.unused), 0x00, sizeof(fbr->bcr_on_memory.unused)); /* BCR[SORA] all ZERO. */
    memcpy_s(fbr->bcr_on_memory.signature, sizeof(fbr->bcr_on_memory.signature), BCR_SIGNATURE, sizeof(fbr->bcr_on_memory.signature));
    return fs_bcr_initbpt(fbr, &fbr->bcr_on_memory.table[0]);
}

static inline bool_t fs_bcr_open(FSBCR **fbr, FSBITMAP *bp) {
    *fbr = (FSBCR *)fs_malloc(sizeof(FSBCR));
    if(!*fbr) return b_false;
    (*fbr)->bp = bp;
    BCR *buf=&(*fbr)->bcr_on_memory;
    if(!fs_bcr_diskread(*fbr)) return fs_bcr_seterror(*fbr, FS_BCR_ERROR_DRIVE_RW_FAILURE);
    if(fs_bcr_sigcmp(*fbr)) {
        fs_bcr_initbcr(*fbr);
        return fs_bcr_diskwrite(*fbr)? fs_bcr_setsuccess(*fbr): fs_bcr_seterror(*fbr, FS_BCR_ERROR_DRIVE_RW_FAILURE);
    } else
        return fs_bcr_setsuccess(*fbr);
}

static inline bool_t fs_bcr_close(FSBCR *fbr, bool_t ret) {
    return fs_free(fbr, ret);
}

/*
static inline bool_t fs_bcr_addbpt(FSBCR *fbr, BCR *bcr) {
    BPT *np=NULL;
    for(index_t i=0; i<ARRAYLEN(bcr->table); ++i) {
        np = &bcr->table[i];
        if(np->types==bpt_unused) break;
    }
    if(np==NULL) return fs_bcr_seterror(fbr, FS_BCR_ERROR_NO_EMPTY_BPT);

}
*/

#endif
