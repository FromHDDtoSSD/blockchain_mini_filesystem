// Copyright (c) 2014-2018 The Bitcoin Core developers
// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_SHA256
#define SORACHANCOIN_FS_SHA256

#include "fs_types.h"
#include "fs_const.h"
#include "fs_memory.h"
#include "fs_endian.h"

typedef enum _tag_sha256_status {
    SHA256_SUCCESS = 0,
    SHA256_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
} sha256_status;

typedef struct _tag_FSSHA256 {
    byte_t hash[32];
    char hashstr[65];
    uint32_t s[8];
    byte_t buf[64];
    uint64_t bytes;
    sha256_status status;
} FSSHA256;

static inline byte_t *fs_sha256_gethash(FSSHA256 *sp) {
    return sp->hash;
}

static inline const char *fs_sha256_gethashstr(FSSHA256 *sp) {
    for(index_t i=0, j=0; i<ARRAYLEN(sp->hash); ++i, j+=2) {
        char tmp[3];
        sprintf_s(tmp, sizeof(tmp), "%x", sp->hash[i]);
        sp->hashstr[j] = tmp[0];
        sp->hashstr[j+1] = tmp[1];
    }
    sp->hashstr[64] = '\0';
    return sp->hashstr;
}

static inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return z ^ (x & (y ^ z)); }
static inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (z & (x | y)); }
static inline uint32_t Sigma0(uint32_t x) { return (x >> 2 | x << 30) ^ (x >> 13 | x << 19) ^ (x >> 22 | x << 10); }
static inline uint32_t Sigma1(uint32_t x) { return (x >> 6 | x << 26) ^ (x >> 11 | x << 21) ^ (x >> 25 | x << 7); }
static inline uint32_t sigma0(uint32_t x) { return (x >> 7 | x << 25) ^ (x >> 18 | x << 14) ^ (x >> 3); }
static inline uint32_t sigma1(uint32_t x) { return (x >> 17 | x << 15) ^ (x >> 19 | x << 13) ^ (x >> 10); }

/** One round of SHA-256. */
static inline void Round(uint32_t a, uint32_t b, uint32_t c, uint32_t *d, uint32_t e, uint32_t f, uint32_t g, uint32_t *h, uint32_t k)
{
    uint32_t t1 = *h + Sigma1(e) + Ch(e, f, g) + k;
    uint32_t t2 = Sigma0(a) + Maj(a, b, c);
    *d += t1;
    *h = t1 + t2;
}

/** Perform a number of SHA-256 transformations, processing 64-byte chunks. */
static inline void Transform(uint32_t *s, const byte_t *chunk, size_t blocks)
{
    while(blocks--) {
        uint32_t a = s[0], b = s[1], c = s[2], d = s[3], e = s[4], f = s[5], g = s[6], h = s[7];
        uint32_t w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10, w11, w12, w13, w14, w15;

        Round(a, b, c, &d, e, f, g, &h, 0x428a2f98 + (w0 = ReadBE32(chunk + 0)));
        Round(h, a, b, &c, d, e, f, &g, 0x71374491 + (w1 = ReadBE32(chunk + 4)));
        Round(g, h, a, &b, c, d, e, &f, 0xb5c0fbcf + (w2 = ReadBE32(chunk + 8)));
        Round(f, g, h, &a, b, c, d, &e, 0xe9b5dba5 + (w3 = ReadBE32(chunk + 12)));
        Round(e, f, g, &h, a, b, c, &d, 0x3956c25b + (w4 = ReadBE32(chunk + 16)));
        Round(d, e, f, &g, h, a, b, &c, 0x59f111f1 + (w5 = ReadBE32(chunk + 20)));
        Round(c, d, e, &f, g, h, a, &b, 0x923f82a4 + (w6 = ReadBE32(chunk + 24)));
        Round(b, c, d, &e, f, g, h, &a, 0xab1c5ed5 + (w7 = ReadBE32(chunk + 28)));
        Round(a, b, c, &d, e, f, g, &h, 0xd807aa98 + (w8 = ReadBE32(chunk + 32)));
        Round(h, a, b, &c, d, e, f, &g, 0x12835b01 + (w9 = ReadBE32(chunk + 36)));
        Round(g, h, a, &b, c, d, e, &f, 0x243185be + (w10 = ReadBE32(chunk + 40)));
        Round(f, g, h, &a, b, c, d, &e, 0x550c7dc3 + (w11 = ReadBE32(chunk + 44)));
        Round(e, f, g, &h, a, b, c, &d, 0x72be5d74 + (w12 = ReadBE32(chunk + 48)));
        Round(d, e, f, &g, h, a, b, &c, 0x80deb1fe + (w13 = ReadBE32(chunk + 52)));
        Round(c, d, e, &f, g, h, a, &b, 0x9bdc06a7 + (w14 = ReadBE32(chunk + 56)));
        Round(b, c, d, &e, f, g, h, &a, 0xc19bf174 + (w15 = ReadBE32(chunk + 60)));

        Round(a, b, c, &d, e, f, g, &h, 0xe49b69c1 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, &c, d, e, f, &g, 0xefbe4786 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, &b, c, d, e, &f, 0x0fc19dc6 + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, &a, b, c, d, &e, 0x240ca1cc + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, &h, a, b, c, &d, 0x2de92c6f + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, &g, h, a, b, &c, 0x4a7484aa + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, &f, g, h, a, &b, 0x5cb0a9dc + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, &e, f, g, h, &a, 0x76f988da + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, &d, e, f, g, &h, 0x983e5152 + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, &c, d, e, f, &g, 0xa831c66d + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, &b, c, d, e, &f, 0xb00327c8 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, &a, b, c, d, &e, 0xbf597fc7 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, &h, a, b, c, &d, 0xc6e00bf3 + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, &g, h, a, b, &c, 0xd5a79147 + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, &f, g, h, a, &b, 0x06ca6351 + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, &e, f, g, h, &a, 0x14292967 + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        Round(a, b, c, &d, e, f, g, &h, 0x27b70a85 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, &c, d, e, f, &g, 0x2e1b2138 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, &b, c, d, e, &f, 0x4d2c6dfc + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, &a, b, c, d, &e, 0x53380d13 + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, &h, a, b, c, &d, 0x650a7354 + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, &g, h, a, b, &c, 0x766a0abb + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, &f, g, h, a, &b, 0x81c2c92e + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, &e, f, g, h, &a, 0x92722c85 + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, &d, e, f, g, &h, 0xa2bfe8a1 + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, &c, d, e, f, &g, 0xa81a664b + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, &b, c, d, e, &f, 0xc24b8b70 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, &a, b, c, d, &e, 0xc76c51a3 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, &h, a, b, c, &d, 0xd192e819 + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, &g, h, a, b, &c, 0xd6990624 + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, &f, g, h, a, &b, 0xf40e3585 + (w14 += sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, &e, f, g, h, &a, 0x106aa070 + (w15 += sigma1(w13) + w8 + sigma0(w0)));

        Round(a, b, c, &d, e, f, g, &h, 0x19a4c116 + (w0 += sigma1(w14) + w9 + sigma0(w1)));
        Round(h, a, b, &c, d, e, f, &g, 0x1e376c08 + (w1 += sigma1(w15) + w10 + sigma0(w2)));
        Round(g, h, a, &b, c, d, e, &f, 0x2748774c + (w2 += sigma1(w0) + w11 + sigma0(w3)));
        Round(f, g, h, &a, b, c, d, &e, 0x34b0bcb5 + (w3 += sigma1(w1) + w12 + sigma0(w4)));
        Round(e, f, g, &h, a, b, c, &d, 0x391c0cb3 + (w4 += sigma1(w2) + w13 + sigma0(w5)));
        Round(d, e, f, &g, h, a, b, &c, 0x4ed8aa4a + (w5 += sigma1(w3) + w14 + sigma0(w6)));
        Round(c, d, e, &f, g, h, a, &b, 0x5b9cca4f + (w6 += sigma1(w4) + w15 + sigma0(w7)));
        Round(b, c, d, &e, f, g, h, &a, 0x682e6ff3 + (w7 += sigma1(w5) + w0 + sigma0(w8)));
        Round(a, b, c, &d, e, f, g, &h, 0x748f82ee + (w8 += sigma1(w6) + w1 + sigma0(w9)));
        Round(h, a, b, &c, d, e, f, &g, 0x78a5636f + (w9 += sigma1(w7) + w2 + sigma0(w10)));
        Round(g, h, a, &b, c, d, e, &f, 0x84c87814 + (w10 += sigma1(w8) + w3 + sigma0(w11)));
        Round(f, g, h, &a, b, c, d, &e, 0x8cc70208 + (w11 += sigma1(w9) + w4 + sigma0(w12)));
        Round(e, f, g, &h, a, b, c, &d, 0x90befffa + (w12 += sigma1(w10) + w5 + sigma0(w13)));
        Round(d, e, f, &g, h, a, b, &c, 0xa4506ceb + (w13 += sigma1(w11) + w6 + sigma0(w14)));
        Round(c, d, e, &f, g, h, a, &b, 0xbef9a3f7 + (w14 + sigma1(w12) + w7 + sigma0(w15)));
        Round(b, c, d, &e, f, g, h, &a, 0xc67178f2 + (w15 + sigma1(w13) + w8 + sigma0(w0)));

        s[0] += a;
        s[1] += b;
        s[2] += c;
        s[3] += d;
        s[4] += e;
        s[5] += f;
        s[6] += g;
        s[7] += h;
        chunk += 64;
    }
}

static inline bool_t fs_sha256_setsuccess(FSSHA256 *sp) {
    sp->status = SHA256_SUCCESS;
    return b_true;
}

static inline bool_t fs_sha256_seterror(FSSHA256 *sp, sha256_status status) {
    sp->status = status;
    return b_false;
}

static inline bool_t fs_sha256_open(FSSHA256 **sp) {
    *sp = (FSSHA256 *)fs_malloc(sizeof(FSSHA256));
    if(!*sp) return b_false;
    memset_s((*sp)->hash, sizeof(FSSHA256), 0x00, sizeof((*sp)->hash));
    return fs_sha256_setsuccess(*sp);
}

static inline bool_t fs_sha256_close(FSSHA256 *sp, bool_t ret) {
    return fs_free(sp, ret);
}

static inline bool_t fs_sha256_init(FSSHA256 *sp) {
    sp->s[0] = 0x6a09e667ul;
    sp->s[1] = 0xbb67ae85ul;
    sp->s[2] = 0x3c6ef372ul;
    sp->s[3] = 0xa54ff53aul;
    sp->s[4] = 0x510e527ful;
    sp->s[5] = 0x9b05688cul;
    sp->s[6] = 0x1f83d9abul;
    sp->s[7] = 0x5be0cd19ul;
    memset(sp->hash, 0x00, sizeof(sp->hash));
    sp->bytes=0;
    return fs_sha256_setsuccess(sp);
}

static inline bool_t fs_sha256_update(FSSHA256 *sp, counter_t num, const byte_t *data) {
    const byte_t *end = data + num;
    size_t bufsize = sp->bytes % 64;
    if(bufsize && bufsize + num >= 64) {
        // Fill the buffer, and process it.
        memcpy(sp->buf + bufsize, data, 64 - bufsize);
        sp->bytes += 64 - bufsize;
        data += 64 - bufsize;
        Transform(sp->s, sp->buf, 1);
        bufsize = 0;
    }
    if(end - data >= 64) {
        size_t blocks = (end - data) / 64;
        Transform(sp->s, data, blocks);
        data += 64 * blocks;
        sp->bytes += 64 * blocks;
    }
    if(end > data) {
        // Fill the buffer with what remains.
        memcpy(sp->buf + bufsize, data, end - data);
        sp->bytes += end - data;
    }
    return fs_sha256_setsuccess(sp);
}

static inline bool_t fs_sha256_final(FSSHA256 *sp) {
    static const byte_t pad[64] = { 0x80 };
    byte_t sizedesc[8];
    WriteBE64(sizedesc, sp->bytes << 3);
    if(!fs_sha256_update(sp, 1 + ((119 - (sp->bytes % 64)) % 64), pad)) return b_false;
    if(!fs_sha256_update(sp, 8, sizedesc)) return b_false;
    WriteBE32(sp->hash, sp->s[0]);
    WriteBE32(sp->hash + 4, sp->s[1]);
    WriteBE32(sp->hash + 8, sp->s[2]);
    WriteBE32(sp->hash + 12, sp->s[3]);
    WriteBE32(sp->hash + 16, sp->s[4]);
    WriteBE32(sp->hash + 20, sp->s[5]);
    WriteBE32(sp->hash + 24, sp->s[6]);
    WriteBE32(sp->hash + 28, sp->s[7]);
    return fs_sha256_setsuccess(sp);
}

/* [OK] */
# ifdef DEBUG
static inline void fs_sha256_test() {
    // Input state (equal to the initial SHA256 state)
    static const uint32_t init[8] = {
        0x6a09e667ul, 0xbb67ae85ul, 0x3c6ef372ul, 0xa54ff53aul, 0x510e527ful, 0x9b05688cul, 0x1f83d9abul, 0x5be0cd19ul
    };
    // Some random input data to test with
    static const unsigned char data[641] = "-" // Intentionally not aligned
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
        "eiusmod tempor incididunt ut labore et dolore magna aliqua. Et m"
        "olestie ac feugiat sed lectus vestibulum mattis ullamcorper. Mor"
        "bi blandit cursus risus at ultrices mi tempus imperdiet nulla. N"
        "unc congue nisi vita suscipit tellus mauris. Imperdiet proin fer"
        "mentum leo vel orci. Massa tempor nec feugiat nisl pretium fusce"
        " id velit. Telus in metus vulputate eu scelerisque felis. Mi tem"
        "pus imperdiet nulla malesuada pellentesque. Tristique magna sit.";
    // Expected output state for hashing the i*64 first input bytes above (excluding SHA256 padding).
    static const uint32_t result[9][8] = {
        { 0x6a09e667ul, 0xbb67ae85ul, 0x3c6ef372ul, 0xa54ff53aul, 0x510e527ful, 0x9b05688cul, 0x1f83d9abul, 0x5be0cd19ul },
        { 0x91f8ec6bul, 0x4da10fe3ul, 0x1c9c292cul, 0x45e18185ul, 0x435cc111ul, 0x3ca26f09ul, 0xeb954caeul, 0x402a7069ul },
        { 0xcabea5acul, 0x374fb97cul, 0x182ad996ul, 0x7bd69cbful, 0x450ff900ul, 0xc1d2be8aul, 0x6a41d505ul, 0xe6212dc3ul },
        { 0xbcff09d6ul, 0x3e76f36eul, 0x3ecb2501ul, 0x78866e97ul, 0xe1c1e2fdul, 0x32f4eafful, 0x8aa6c4e5ul, 0xdfc024bcul },
        { 0xa08c5d94ul, 0x0a862f93ul, 0x6b7f2f40ul, 0x8f9fae76ul, 0x6d40439ful, 0x79dcee0cul, 0x3e39ff3aul, 0xdc3bdbb1ul },
        { 0x216a0895ul, 0x9f1a3662ul, 0xe99946f9ul, 0x87ba4364ul, 0x0fb5db2cul, 0x12bed3d3ul, 0x6689c0c7ul, 0x292f1b04ul },
        { 0xca3067f8ul, 0xbc8c2656ul, 0x37cb7e0dul, 0x9b6b8b0ful, 0x46dc380bul, 0xf1287f57ul, 0xc42e4b23ul, 0x3fefe94dul },
        { 0x3e4c4039ul, 0xbb6fca8cul, 0x6f27d2f7ul, 0x301e44a4ul, 0x8352ba14ul, 0x5769ce37ul, 0x48a1155ful, 0xc0e1c4c6ul },
        { 0xfe2fa9ddul, 0x69d0862bul, 0x1ae0db23ul, 0x471f9244ul, 0xf55c0145ul, 0xc30f9c3bul, 0x40a84ea0ul, 0x5b8a266cul },
    };
    // Test Transform() for 0 through 8 transformations.
    for(size_t i = 0; i <= 8; ++i) {
        uint32_t state[8];
        memcpy(state, init, sizeof(state));
        Transform(state, data + 1, i);
        assert(memcmp(state, result[i], sizeof(state))==0);
    }
}
# endif

#endif
