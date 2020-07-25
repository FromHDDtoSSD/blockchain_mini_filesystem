// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_CONST
#define SORACHANCOIN_FS_CONST

/*
** Note: Must NOT change values below.
*/
#define SECTOR_SIZE 512
#define SECTORS_PER_CLUS 8
#define CLUSTER_SIZE (SECTOR_SIZE*SECTORS_PER_CLUS)
#define CLUSTER_CAPACITY 1024
#define SECTOR_CAPACITY (CLUSTER_CAPACITY*SECTORS_PER_CLUS)
#define FS_PATH_LENGTH 32
#ifndef MAX_PATH
# define MAX_PATH 260
#endif

#define ARRAYLEN(X) (sizeof(X)/sizeof(X[0]))
#define true_t 1
#define false_t 0

#endif
