// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_MEMORY
#define SORACHANCOIN_FS_MEMORY

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "fs_types.h"

#define MEMORY_ASSERT_SIGNATURE "MIKE"
#define MEMORY_ASSERT_SIG_LENGTH 4

static inline byte_t *fs_malloc(fsize_t size) {
    byte_t *ptr = (byte_t *)malloc((size_t)size + sizeof(fsize_t) + MEMORY_ASSERT_SIG_LENGTH);
    if(ptr) {
        *(fsize_t *)ptr = size;
        memcpy(ptr + size + sizeof(fsize_t), MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH);
        return ptr + sizeof(fsize_t);
    } else
        return NULL;
}

static inline bool_t fs_free(void *ptr, bool_t ret) {
    if(ptr) {
        assert(memcmp((byte_t *)ptr + *(fsize_t *)((byte_t *)ptr - sizeof(fsize_t)), MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH) == 0);
        free((byte_t *)ptr - sizeof(fsize_t));
    }
    return ret;
}

#endif
