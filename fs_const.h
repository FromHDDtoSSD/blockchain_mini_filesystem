// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_CONST
#define SORACHANCOIN_FS_CONST

#ifndef __STDC_WAIT_LIB_EXT1__
# define __STDC_WAIT_LIB_EXT1__ 1
#endif
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "fs_types.h"

/*
** Note: Must NOT change values below.
*/
#define BYTES_PER_SECTOR 512
#define SECTORS_PER_CLUSTER 8
#define BYTES_PER_CLUSTER (BYTES_PER_SECTOR*SECTORS_PER_CLUSTER)
#define CLUSTERS_PER_CHUNK 1024
#define SECTORS_PER_CHUNK (CLUSTERS_PER_CHUNK*SECTORS_PER_CLUSTER)
#define BYTES_PER_CHUNK (CLUSTERS_PER_CHUNK*BYTES_PER_CLUSTER)

#ifndef MAX_PATH
# define MAX_PATH 260
#endif
#define ARRAYLEN(X) (sizeof(X)/sizeof(X[0]))
#define b_true 1
#define b_false 0

#endif
