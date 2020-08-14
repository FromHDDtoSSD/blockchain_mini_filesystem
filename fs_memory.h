// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_MEMORY
#define SORACHANCOIN_FS_MEMORY

#include "fs_types.h"
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

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

static inline errno_t memset_s(void *_Dst, fsize_t _DstSize, int _Val, fsize_t _SetSize) {
    assert(_DstSize>=_SetSize);
    assert(memcmp((const byte_t *)_Dst+_DstSize, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    memset(_Dst, _Val, _SetSize);
    return 0;
}

# if !defined(COMPILER_MSC)
static inline errno_t memcpy_s(void *_Dst, fsize_t _DstSize, const void *_Src, fsize_t _SrcSize) {
    assert(_DstSize>=_SrcSize);
    assert(memcmp((const byte_t *)_Dst+_DstSize, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    memcpy(_Dst, _Src, _SrcSize);
    return 0;
}
# endif

# if !defined(COMPILER_MSC)
static inline errno_t strcpy_s(const char *_Dst, fsize_t _DstSize, const char *_Src) {
    assert(memcmp((const byte_t *)_Dst+_DstSize, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    strcpy(_Dst, _Src);
    return 0;
}
# endif

# if !defined(COMPILER_MSC)
static inline errno_t sprintf_s(char *_Dst, fsize_t _DstSize, const char *_Format, ...) {
    assert(memcmp((const byte_t *)_Dst+_DstSize, MEMORY_ASSERT_SIGNATURE, MEMORY_ASSERT_SIG_LENGTH)==0);
    va_list va;
    va_start(va, _Format);
    vsprintf(_Dst, (const char *)_Format, va);
    va_end(va);
    return 0;
}
# endif

# if !defined(COMPILER_MSC)
static inline errno_t printf_s(const char *_Format, ...) {
    va_list va;
    va_start(va, _Format);
    vprintf((const char *)_Format, va);
    va_end(va);
    return 0;
}
# endif

#endif
