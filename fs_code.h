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

typedef enum _tag_io_status {
    IO_SUCCESS = 0,
    IO_ERROR_PARAM = 1,
    IO_ERROR_LOCKED = 2,
    IO_ERROR_MEMORY_ALLOCATE_FAILURE = 3,
    IO_ERROR_DRIVE_RW_FAILURE = 4,
} io_status;

#endif
