// Copyright (c) 2014-2018 The Bitcoin Core developers
// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_SHA256
#define SORACHANCOIN_FS_SHA256

#include "fs_types.h"
#include "fs_const.h"

typedef enum _tag_sha256_status {
    SHA256_SUCCESS = 0,
    SHA256_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
} sha256_status;

typedef struct _tag_FSSHA256 {
    byte_t hash[32];
    char hashstr[33];
    uint32_t s[8];
    byte_t buf[64];
    uint64_t bytes;
    sha256_status status;
} FSSHA256;

static inline byte_t *fs_sha256_gethash(FSSHA256 *sp) {
    return sp->hash;
}

static inline const char *fs_sha256_gethashstr(FSSHA256 *sp) {
    for(index_t i=0; i<sizeof(sp->hash); ++i)
        sp->hashstr[i] = sp->hash[i];
    sp->hashstr[32] = '\0';
    return sp->hashstr;
}

bool_t fs_sha256_open(FSSHA256 **sp);
bool_t fs_sha256_close(FSSHA256 *sp, bool_t ret);
bool_t fs_sha256_init(FSSHA256 *sp);
bool_t fs_sha256_update(FSSHA256 *sp, counter_t num, const byte_t *data);
bool_t fs_sha256_final(FSSHA256 *sp);

#endif
