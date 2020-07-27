// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_TYPES
#define SORACHANCOIN_FS_TYPES

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
** BAD: size_t size, int counter, bool flag
*/
typedef unsigned char byte_t;
typedef int32_t bool_t;
typedef int32_t counter_t;
typedef int32_t index_t;
typedef unsigned char str_t;
typedef int32_t fsize_t;
typedef int32_t flag_t;
typedef int64_t sector_t;
typedef int64_t cluster_t;

#endif
