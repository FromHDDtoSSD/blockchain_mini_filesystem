// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_FILE
#define SORACHANCOIN_FS_FILE

#include <stdio.h>
#include "fs_memory.h"
#include "fs_types.h"
#include "fs_const.h"

static inline bool_t fs_open(FILE **fp, const char *name) {
    bool_t exist = fs_isfile(name);
    if(exist)
        *fp = fopen(name, "rb+");
    else
        *fp = fopen(name, "wb+");
    if(*fp && !exist)
        return fs_update(*fp, NULL, exist);
    else
        return *fp != NULL;
}

static inline bool_t fs_close(FILE *fp, bool_t ret) {
    fclose(fp);
    return ret;
}

static inline bool_t fs_read(FILE *fp, byte_t *data, fsize_t size) {
    return fread(data, sizeof(byte_t), (size_t)size, fp) == (size_t)size;
}

static inline bool_t fs_write(FILE *fp, const byte_t *data, fsize_t size) {
    return fwrite(data, sizeof(byte_t), (size_t)size, fp) == (size_t)size;
}

static inline bool_t fs_update(FILE *fp, const byte_t *update, bool_t exist) {
    const fsize_t size = fs_getsize();
    byte_t *buf = fs_malloc(size);
    if(!buf) return false_t;
    if(exist) {
        if(update == NULL) return fs_free(buf, false_t);
        if(!fs_read(fp, buf, size))
            return fs_free(buf, true_t);
        else
            for(int i=0; i < size; ++i)
                buf[i] |= update[i];
    } else
        memset(buf, 0x00, size);
    return fs_free(buf, fs_write(fp, buf, size)) && fs_seek_top(fp);
}

static inline bool_t fs_isfile(const char *name) {
    FILE *fp = fopen(name, "rb");
    if(fp != NULL) return fs_close(fp, true_t);
    else return false_t;
}

static inline bool_t fs_getpos(FILE *fp, fpos_t *pos) {
    return fgetpos(fp, pos) == 0;
}

static inline bool_t fs_setpos(FILE *fp, const fpos_t *pos) {
    return fsetpos(fp, pos) == 0;
}

static inline bool_t fs_seek_top(FILE *fp) {
    return fseek(fp, 0, SEEK_SET) == 0;
}

static inline fsize_t fs_getsize() {
    return CLUSTER_SIZE * CLUSTER_CAPACITY;
}

#endif
