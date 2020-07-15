// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_BLOCKCHAIN_MINI_FILESYSTEM
#define SORACHANCOIN_BLOCKCHAIN_MINI_FILESYSTEM

#include "fs_types.h"
#include "fs_const.h"
#include "fs_memory.h"
#include "fs_code.h"

#pragma pack(push, 1)
typedef struct _tag_fs_header {
    byte_t signature[4]; /* SORA */
    byte_t qhash[8192]; /* CQHASH65536 */
} fs_header;

typedef struct _tag_attr_record {
    byte_t signature[4]; /* ATTR */
    flag_t flags;
    byte_t reserve[20];
} attr_record;

typedef struct _tag_p2p_record {
    byte_t signature[4]; /* P2PR */
    counter_t connect;
    
} p2p_record;

typedef struct _tag_fragment_info {
    byte_t run[8];
    byte_t reserve[16];
} fragment_info;

typedef struct _tag_hash_record { /* CSHA256 */
    byte_t signature[4]; /* HASH */
    byte_t myself[32];
    byte_t top[32];
    byte_t parent[32];
    byte_t fragment[32];
} hash_record;

typedef struct _tag_file_record {
    byte_t signature[4]; /* FILE */
    counter_t reuse_counter;
    bool_t enable;
    bool_t resident;
    attr_record attr;
    fragment_info fragment;
    hash_record hash;
    fsize_t size;
    str_t name[MAX_PATH_DEFAULT];
    byte_t data[1];
} file_record;
#pragma pack(pop)

#endif
