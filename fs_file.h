// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_FILE
#define SORACHANCOIN_FS_FILE

#include <stdio.h>
#include "fs_const.h"
#include "fs_memory.h"
#include "fs_types.h"

typedef enum _tag_file_status {
    FS_FILE_SUCCESS = 0,
    FS_FILE_ERROR_PARAM = 1,
    FS_FILE_ERROR_MEMORY_ALLOCATE_FAILURE = 2,
    FS_FILE_ERROR_DRIVE_RW_FAILURE = 3,
} file_status;

typedef struct _tag_FSFILE {
    FILE *file_ptr;
    index_t seek_last_pos;
    file_status status;
} FSFILE;

static inline bool_t fs_file_setsuccess(FSFILE *fp) {
    fp->status = FS_FILE_SUCCESS;
    return b_true;
}

static inline bool_t fs_file_seterror(FSFILE *fp, file_status status) {
    fp->status = status;
    return b_false;
}

static inline file_status fs_file_getstatus(FSFILE *fp) {
    return fp->status;
}

static inline fsize_t fs_file_getsize() {
    return BYTES_PER_CHUNK;
}

static inline FILE *fs_file_securefopen(const str_t *_FileName, const str_t *_Mode) {
    FILE *fp;
    if(fopen_s(&fp, _FileName, _Mode)!=0) return NULL;
    else return fp;
}

static inline bool_t fs_file_seek_top(FSFILE *fp) {
    return fseek(fp->file_ptr, 0, SEEK_SET) == 0;
}

static inline bool_t fs_file_isfile(const char *path) {
    FILE *fp;
    fp = fs_file_securefopen(path, "rb");
    if(fp != NULL) {fclose(fp); return b_true;}
    else return b_false;
}

static inline bool_t fs_file_read(FSFILE *fp, byte_t *data, fsize_t size) {
    fp->status = (fread(data, sizeof(byte_t), (size_t)size, fp->file_ptr) == (size_t)size) ? FS_FILE_SUCCESS: FS_FILE_ERROR_DRIVE_RW_FAILURE;
    return fp->status == FS_FILE_SUCCESS;
}

static inline bool_t fs_file_write(FSFILE *fp, const byte_t *data, fsize_t size) {
    fp->status = (fwrite(data, sizeof(byte_t), (size_t)size, fp->file_ptr) == (size_t)size) ? FS_FILE_SUCCESS : FS_FILE_ERROR_DRIVE_RW_FAILURE;
    return fp->status == FS_FILE_SUCCESS;
}

static inline bool_t fs_file_update(FSFILE *fp, const byte_t *update, bool_t exist) {
    const fsize_t size = fs_file_getsize();
    byte_t *buf = fs_malloc(size);
    if(!buf) return fs_file_seterror(fp, FS_FILE_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(exist) {
        if(update==NULL) return fs_free(buf, fs_file_seterror(fp, FS_FILE_ERROR_PARAM));
        if(!fs_file_read(fp, buf, size)) return fs_free(buf, fs_file_seterror(fp, FS_FILE_ERROR_DRIVE_RW_FAILURE));
        else
            for(int i=0; i < size; ++i)
                buf[i] |= update[i];
    } else
        memset(buf, 0x00, size);
    return (fs_free(buf, fs_file_write(fp, buf, size)) && fs_file_seek_top(fp)) ? fs_file_setsuccess(fp): fs_file_seterror(fp, FS_FILE_ERROR_DRIVE_RW_FAILURE);
}

static inline bool_t fs_file_open(FSFILE **fp, const char *path) {
    *fp = (FSFILE *)fs_malloc(sizeof(FSFILE));
    if(!*fp) return b_false;
    (*fp)->file_ptr = NULL;
    (*fp)->seek_last_pos = 0;
    bool_t exist = fs_file_isfile(path);
    if(exist) (*fp)->file_ptr = fs_file_securefopen(path, "rb+");
    else (*fp)->file_ptr = fs_file_securefopen(path, "wb+");
    if((*fp)->file_ptr && !exist) return fs_file_update(*fp, NULL, exist);
    else return ((*fp)->file_ptr != NULL) ? fs_file_setsuccess(*fp): fs_file_seterror(*fp, FS_FILE_ERROR_DRIVE_RW_FAILURE);
}

static inline bool_t fs_file_close(FSFILE *fp, bool_t ret) {
    if(fp->file_ptr) fclose(fp->file_ptr);
    return fs_free(fp, ret);
}

static inline bool_t fs_file_seek(FSFILE *fp, index_t pos) {
    fp->seek_last_pos = pos;
    return fseek(fp->file_ptr, (long)pos, SEEK_SET) == 0;
}

static inline index_t fs_file_getlastseek(FSFILE *fp) {
    return fp->seek_last_pos;
}

#endif
