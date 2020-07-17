// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_CODE
#define SORACHANCOIN_FS_CODE

typedef enum _tag_attr_type {
    ATTR_NORMAL = 0,
} attr_type;

typedef enum _tag_token_status {
    TOKEN_SUCCESS = 0,
    TOKEN_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
    TOKEN_ERROR_SIGNATURE = 2,
    TOKEN_ERROR_BLOCKCHAIN_CONFIRM = 3,
    TOKEN_ERROR_BLOCKCHAIN_OPCODE = 4,
    TOKEN_ERROR_HASH = 5,
} token_status;

#endif
