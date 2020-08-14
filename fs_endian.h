// Copyright (c) 2014-2018 The Bitcoin Core developers
// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_ENDIAN
#define SORACHANCOIN_FS_ENDIAN

#include <stddef.h>
#include "fs_types.h"

static inline uindex_t getnumbits(uindex_t val) {
    val=(val&0x55555555)+((val>>1)&0x55555555);
    val=(val&0x33333333)+((val>>2)&0x33333333);
    val=(val&0x0f0f0f0f)+((val>>4)&0x0f0f0f0f);
    val=(val&0x00ff00ff)+((val>>8)&0x00ff00ff);
    return (val&0x0000ffff)+((val>>16)&0x0000ffff);
}

static inline uindex_t getLsb_32(uindex_t val) {
    uindex_t tmp=val;
    val|=(val<<1);
    val|=(val<<2);
    val|=(val<<4);
    val|=(val<<8);
    val|=(val<<16);
    return (tmp==0) ? 0: 32-getnumbits(val);
}

static inline uindex_t getMsb_32(uindex_t val) {
    uindex_t tmp=val;
    val|=(val>>1);
    val|=(val>>2);
    val|=(val>>4);
    val|=(val>>8);
    val|=(val>>16);
    return (tmp==0) ? 0: getnumbits(val)-1;
}

static inline uint16_t bswap_16(uint16_t x)
{
    return (x >> 8) | (x << 8);
}

static inline uint32_t bswap_32(uint32_t x)
{
    return (((x & 0xff000000U) >> 24) | ((x & 0x00ff0000U) >> 8) |
           ((x & 0x0000ff00U) << 8) | ((x & 0x000000ffU) << 24));
}

static inline uint64_t bswap_64(uint64_t x)
{
    return (((x & 0xff00000000000000ull) >> 56)
         | ((x & 0x00ff000000000000ull) >> 40)
         | ((x & 0x0000ff0000000000ull) >> 24)
         | ((x & 0x000000ff00000000ull) >> 8)
         | ((x & 0x00000000ff000000ull) << 8)
         | ((x & 0x0000000000ff0000ull) << 24)
         | ((x & 0x000000000000ff00ull) << 40)
         | ((x & 0x00000000000000ffull) << 56));
}

#if defined(HAVE_ENDIAN_H)
#include <endian.h>
#elif defined(HAVE_SYS_ENDIAN_H)
#include <sys/endian.h>
#endif

#ifndef HAVE_CONFIG_H
// While not technically a supported configuration, defaulting to defining these
// DECLs when we were compiled without autotools makes it easier for other build
// systems to build things like libbitcoinconsensus for strange targets.
#ifdef htobe16
#define HAVE_DECL_HTOBE16 1
#endif
#ifdef htole16
#define HAVE_DECL_HTOLE16 1
#endif
#ifdef be16toh
#define HAVE_DECL_BE16TOH 1
#endif
#ifdef le16toh
#define HAVE_DECL_LE16TOH 1
#endif

#ifdef htobe32
#define HAVE_DECL_HTOBE32 1
#endif
#ifdef htole32
#define HAVE_DECL_HTOLE32 1
#endif
#ifdef be32toh
#define HAVE_DECL_BE32TOH 1
#endif
#ifdef le32toh
#define HAVE_DECL_LE32TOH 1
#endif

#ifdef htobe64
#define HAVE_DECL_HTOBE64 1
#endif
#ifdef htole64
#define HAVE_DECL_HTOLE64 1
#endif
#ifdef be64toh
#define HAVE_DECL_BE64TOH 1
#endif
#ifdef le64toh
#define HAVE_DECL_LE64TOH 1
#endif

#endif // HAVE_CONFIG_H

#if defined(WORDS_BIGENDIAN)

static inline uindex_t fs_getLsb32(uindex_t val) {
    val=bswap_32(val);
    return getLsb_32(val);
}

static inline uindex_t fs_getMsb32(uindex_t val) {
    val=bswap_32(val);
    return getMsb_32(val);
}

#if HAVE_DECL_HTOBE16 == 0
static inline uint16_t htobe16(uint16_t host_16bits)
{
    return host_16bits;
}
#endif // HAVE_DECL_HTOBE16

#if HAVE_DECL_HTOLE16 == 0
static inline uint16_t htole16(uint16_t host_16bits)
{
    return bswap_16(host_16bits);
}
#endif // HAVE_DECL_HTOLE16

#if HAVE_DECL_BE16TOH == 0
static inline uint16_t be16toh(uint16_t big_endian_16bits)
{
    return big_endian_16bits;
}
#endif // HAVE_DECL_BE16TOH

#if HAVE_DECL_LE16TOH == 0
static inline uint16_t le16toh(uint16_t little_endian_16bits)
{
    return bswap_16(little_endian_16bits);
}
#endif // HAVE_DECL_LE16TOH

#if HAVE_DECL_HTOBE32 == 0
static inline uint32_t htobe32(uint32_t host_32bits)
{
    return host_32bits;
}
#endif // HAVE_DECL_HTOBE32

#if HAVE_DECL_HTOLE32 == 0
static inline uint32_t htole32(uint32_t host_32bits)
{
    return bswap_32(host_32bits);
}
#endif // HAVE_DECL_HTOLE32

#if HAVE_DECL_BE32TOH == 0
static inline uint32_t be32toh(uint32_t big_endian_32bits)
{
    return big_endian_32bits;
}
#endif // HAVE_DECL_BE32TOH

#if HAVE_DECL_LE32TOH == 0
static inline uint32_t le32toh(uint32_t little_endian_32bits)
{
    return bswap_32(little_endian_32bits);
}
#endif // HAVE_DECL_LE32TOH

#if HAVE_DECL_HTOBE64 == 0
static inline uint64_t htobe64(uint64_t host_64bits)
{
    return host_64bits;
}
#endif // HAVE_DECL_HTOBE64

#if HAVE_DECL_HTOLE64 == 0
static inline uint64_t htole64(uint64_t host_64bits)
{
    return bswap_64(host_64bits);
}
#endif // HAVE_DECL_HTOLE64

#if HAVE_DECL_BE64TOH == 0
static inline uint64_t be64toh(uint64_t big_endian_64bits)
{
    return big_endian_64bits;
}
#endif // HAVE_DECL_BE64TOH

#if HAVE_DECL_LE64TOH == 0
static inline uint64_t le64toh(uint64_t little_endian_64bits)
{
    return bswap_64(little_endian_64bits);
}
#endif // HAVE_DECL_LE64TOH

#else // WORDS_BIGENDIAN

static inline uindex_t fs_getLsb32(uindex_t val) {
    return getLsb_32(val);
}

static inline uindex_t fs_getMsb32(uindex_t val) {
    return getMsb_32(val);
}

#if HAVE_DECL_HTOBE16 == 0
static inline uint16_t htobe16(uint16_t host_16bits)
{
    return bswap_16(host_16bits);
}
#endif // HAVE_DECL_HTOBE16

#if HAVE_DECL_HTOLE16 == 0
static inline uint16_t htole16(uint16_t host_16bits)
{
    return host_16bits;
}
#endif // HAVE_DECL_HTOLE16

#if HAVE_DECL_BE16TOH == 0
static inline uint16_t be16toh(uint16_t big_endian_16bits)
{
    return bswap_16(big_endian_16bits);
}
#endif // HAVE_DECL_BE16TOH

#if HAVE_DECL_LE16TOH == 0
static inline uint16_t le16toh(uint16_t little_endian_16bits)
{
    return little_endian_16bits;
}
#endif // HAVE_DECL_LE16TOH

#if HAVE_DECL_HTOBE32 == 0
static inline uint32_t htobe32(uint32_t host_32bits)
{
    return bswap_32(host_32bits);
}
#endif // HAVE_DECL_HTOBE32

#if HAVE_DECL_HTOLE32 == 0
static inline uint32_t htole32(uint32_t host_32bits)
{
    return host_32bits;
}
#endif // HAVE_DECL_HTOLE32

#if HAVE_DECL_BE32TOH == 0
static inline uint32_t be32toh(uint32_t big_endian_32bits)
{
    return bswap_32(big_endian_32bits);
}
#endif // HAVE_DECL_BE32TOH

#if HAVE_DECL_LE32TOH == 0
static inline uint32_t le32toh(uint32_t little_endian_32bits)
{
    return little_endian_32bits;
}
#endif // HAVE_DECL_LE32TOH

#if HAVE_DECL_HTOBE64 == 0
static inline uint64_t htobe64(uint64_t host_64bits)
{
    return bswap_64(host_64bits);
}
#endif // HAVE_DECL_HTOBE64

#if HAVE_DECL_HTOLE64 == 0
static inline uint64_t htole64(uint64_t host_64bits)
{
    return host_64bits;
}
#endif // HAVE_DECL_HTOLE64

#if HAVE_DECL_BE64TOH == 0
static inline uint64_t be64toh(uint64_t big_endian_64bits)
{
    return bswap_64(big_endian_64bits);
}
#endif // HAVE_DECL_BE64TOH

#if HAVE_DECL_LE64TOH == 0
static inline uint64_t le64toh(uint64_t little_endian_64bits)
{
    return little_endian_64bits;
}
#endif // HAVE_DECL_LE64TOH

#endif // WORDS_BIGENDIAN

#endif // SORACHANCOIN_FS_ENDIAN
