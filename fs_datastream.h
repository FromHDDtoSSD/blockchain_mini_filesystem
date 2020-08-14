// Copyright (c) 2018-2020 The SorachanCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_DATASTREAM
#define SORACHANCOIN_FS_DATASTREAM

/*
* ** fs_datastream **
*
* Load or store data sequentially, cluster by cluster.
* 
* There are lshift(like <<) and rshift(like >>). 
* 
*/

#include <stdlib.h>
#include "fs_memory.h"
#include "fs_fragment_vector.h"

typedef enum _tag_datastream_status {
    DATASTREAM_SUCCESS = 0,
    DATASTREAM_ERROR_MEMORY_ALLOCATE_FAILURE = 1,
} datastream_status;

typedef struct _tag_FSDATASTREAM {
    FSFRAGVECTOR *vch;
    fsize_t current_size;
    index_t dest_index;
    datastream_status status;
} FSDATASTREAM;

typedef struct _tag_STREAM_HANDLE {
    fsize_t size;
    byte_t *dest;
} SRND;

static inline bool_t fs_datastream_setsuccess(FSDATASTREAM *dsp) {
    dsp->status = DATASTREAM_SUCCESS;
    return b_true;
}

static inline bool_t fs_datastream_seterror(FSDATASTREAM *dsp, datastream_status status) {
    dsp->status = status;
    return b_false;
}

static inline bool_t fs_datastream_open(FSDATASTREAM **dsp) {
    *dsp = (FSDATASTREAM *)fs_malloc(sizeof(FSDATASTREAM));
    if(!*dsp) return b_false;
    if(!fs_fragvector_open(&(*dsp)->vch, 0, sizeof(VECTOR_DATA))) return fs_free(*dsp, fs_datastream_seterror(*dsp, DATASTREAM_ERROR_MEMORY_ALLOCATE_FAILURE));
    (*dsp)->current_size=0;
    (*dsp)->dest_index=0;
    return fs_datastream_setsuccess(*dsp);
}

static inline bool_t fs_datastream_lshift(FSDATASTREAM *dsp, const byte_t *data, fsize_t size) {
    dsp->current_size+=size;
    while(size>0){
        VECTOR_DATA *vch;
        if(!fs_fragvector_insert2(dsp->vch, &vch)) return fs_datastream_seterror(dsp, DATASTREAM_ERROR_MEMORY_ALLOCATE_FAILURE);
        fsize_t cpsize=(size>sizeof(vch->data))? sizeof(vch->data): size;
        memcpy(vch->data, data, cpsize);
        size-=cpsize;
        data+=cpsize;
        assert(size>=0);
    }
    return fs_datastream_setsuccess(dsp);
}

static inline bool_t fs_datastream_rstream(FSDATASTREAM *dsp, SRND **srnd, fsize_t size, index_t *index) {
    if(!index) index=&dsp->dest_index;
    *srnd=(SRND *)fs_malloc(sizeof(SRND));
    if(!*srnd) return fs_datastream_seterror(dsp, DATASTREAM_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(size<=sizeof(((VECTOR_DATA *)NULL)->data)) {
        (*srnd)->size=size;
        (*srnd)->dest=(byte_t *)fs_fragvector_getdata(dsp->vch, (*index)++)+sizeof(byte_t)*offsetof(VECTOR_DATA,data);
        dsp->current_size-=size;
        return fs_datastream_setsuccess(dsp);
    } else {
        (*srnd)->size=size;
        (*srnd)->dest=(byte_t *)fs_malloc(size);
        if(!(*srnd)->dest) return fs_datastream_seterror(dsp, DATASTREAM_ERROR_MEMORY_ALLOCATE_FAILURE);
        byte_t *buf=(*srnd)->dest;
        while(size>0) {
            fsize_t cpsize=(size>sizeof(((VECTOR_DATA *)NULL)->data))? sizeof(((VECTOR_DATA *)NULL)->data): size;
            memcpy(buf, fs_fragvector_getdata(dsp->vch, (*index)++), cpsize);
            size-=cpsize;
            buf+=cpsize;
            assert(size>=0);
        }
        dsp->current_size-=size;
        return fs_datastream_setsuccess(dsp);
    }
}

static inline bool_t fs_datastream_rshift(FSDATASTREAM *dsp, SRND **srnd, fsize_t size) {
    return fs_datastream_rstream(dsp, srnd, size, NULL);
}

/* Note: The following function can be used when size is evenly stacked. */
static inline bool_t fs_datastream_rgetdata(FSDATASTREAM *dsp, SRND **srnd, fsize_t size, index_t index) {
    dsp->current_size+=size; /* It is not a ">> shift" by unload, so add the size beforehand. */
    return fs_datastream_rstream(dsp, srnd, size, &index);
}
static inline index_t fs_datastream_rgetsize(FSDATASTREAM *dsp, fsize_t size) {
    return fs_fragvector_getsize(dsp->vch)/(size/sizeof(VECTOR_DATA)+((size%sizeof(VECTOR_DATA)==0)?0:1));
}

static inline byte_t *fs_datastream_getdata(SRND *srnd) {
    return srnd->dest;
}

static inline bool_t fs_datastream_free(SRND *srnd, bool_t ret) {
    return (srnd->size<=sizeof(((VECTOR_DATA *)NULL)->data))? ret: fs_free(srnd->dest,ret);
}

static inline bool_t fs_datastream_close(FSDATASTREAM *dsp, bool_t ret) {
    return fs_free(dsp, fs_fragvector_close(dsp->vch, ret));
}

#endif
