// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_DISK
#define SORACHANCOIN_FS_DISK

#include "fs_memory.h"
#include "fs_types.h"
#include "fs_const.h"
#include "fs_code.h"
#include "fs_file.h"

io_status fs_diskread(sector_t begin, sector_t num);
io_status fs_diskwrite(sector_t begin, sector_t num);

#endif
