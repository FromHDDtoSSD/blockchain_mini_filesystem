// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_TYPES
#define SORACHANCOIN_FS_TYPES

#define __STDC_WAIT_LIB_EXT1__ 1
#include <stdio.h>
#include <stdarg.h>

/*
** Compiler type
**
*/
#ifdef __INTEL_COMPILER
# define WIN32
# define COMPILER_INTEL
#elif _MSC_VER
# define WIN32
# define COMPILER_MSC
#elif __clang__
# define COMPILER_CLANG
#elif __GNUC__
# define COMPILER_GNU
#elif __MINGW32__
# define WIN32
# define COMPILER_MINGW
#else
# define WIN32
# define COMPILER_OTHER
#endif

#ifdef WIN32
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

/*
** Note: Must use types below, because if other types using, may NOT confirm by Blockchain.
**
** OK: fsize_t size, counter_t counter, bool_t flag
** NG: size_t size, int counter, bool flag
*/
typedef unsigned char byte_t;
typedef unsigned char aldstbyte_t; /* when allocated destination buffer. */
typedef int32_t bool_t;
typedef int32_t num_t;
typedef int64_t counter_t;
typedef int32_t index_t;
typedef int32_t fbit_t;
typedef char str_t;
typedef int32_t fsize_t;
typedef int64_t foffset_t;
typedef int64_t llsize_t;
typedef int32_t flag_t;
typedef int64_t sector_t;
typedef int64_t cluster_t;
typedef unsigned int uindex_t;

/* 
** stdio secure functions
**
*/
# if !defined(COMPILER_MSC)
static inline errno_t fopen_s(FILE **fp, const char *_FileName, const char *_Mode) {
    *fp=fopen(_FileName, _Mode);
    return (fp!=NULL)? 0: 1;
}
# endif

/*
** Debug: fs_printf
**
*/
static inline void fs_printf(const str_t *format, ...) {
#ifdef DEBUG
    va_list va;
    va_start(va, format);
    vprintf((const char *)format, va);
    va_end(va);
#endif
}

#endif
