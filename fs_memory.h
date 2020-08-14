// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_MEMORY
#define SORACHANCOIN_FS_MEMORY

# ifndef __STDC_WAIT_LIB_EXT1__
#define __STDC_WAIT_LIB_EXT1__ 1
# endif
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

static inline int memcmp_s(const void *_Src1, fsize_t _Src1Size, const void *_Src2, fsize_t _Src2Size) {
    assert(_Src1Size==_Src2Size);
    assert(memcmp((const byte_t *)_Src1+_Src1Size, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    assert(memcmp((const byte_t *)_Src2+_Src2Size, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    return memcmp(_Src1, _Src2, _Src1Size);
}

# ifdef WIN32
static inline errno_t memset_s(void *_Dst, fsize_t _DstSize, int _Val, fsize_t _SetSize) {
    assert(_DstSize>=_SetSize);
    assert(memcmp((const byte_t *)_Dst+_DstSize, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    memset(_Dst, _Val, _SetSize);
    return 0;
}
# endif

#endif
