// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_FRAGMENT_VECTOR
#define SORACHANCOIN_FS_FRAGMENT_VECTOR

#include "fs_memory.h"
#include "fs_types.h"
#include "fs_const.h"

typedef unsigned int uindex_t;
#define V_ALIGNMENT sizeof(double)
#define P_TABLE_NUM 128

/*
*
* ** fs_fragment_vector **
*
* vector on memory.
*
* Note:
* It's array that spreads data in a vacant memory with a minimum of effort.
* An address continuity is NOT guaranteed due to fragmentation. (use function, fs_fragvector_getdata(fvp, index))
* Please be careful only there.
*/

/*
* ***** tips: In C++, the following. *****
*
* VECTOR_DATA:
* template<typename T>
* struct VECTOR_DATA<T> {
*     [any data]
*     VECTOR_DATA() {}
*     ~VECTOR_DATA() {}
* };
*
* open:
* FSFRAGVECTOR *fvp;
* fs_fragvector_open(&fvp);
*
* insert:
* VECTOR_DATA<T> *ptr;
* fs_fragvector_insert2(fvp, &ptr);
* new(ptr) VECTOR_DATA;
*
* pop:
* VECTOR_DATA<T> *ptr=fs_fragvector_pop(fvp);
* ptr->~VECTOR_DATA();
*
* close:
* index_t i=0;
* VECTOR_DATA<T> *ite;
* while((ite=fs_fragvector_iterator(fvp, i))!=nullptr)
*     ite->~VECTOR_DATA();
* fs_fragvector_close(fvp);
*
*/

static inline uindex_t fs_fragvector_getnumbits(uindex_t val) {
    val=(val&0x55555555)+((val>>1)&0x55555555);
    val=(val&0x33333333)+((val>>2)&0x33333333);
    val=(val&0x0f0f0f0f)+((val>>4)&0x0f0f0f0f);
    val=(val&0x00ff00ff)+((val>>8)&0x00ff00ff);
    return (val&0x0000ffff)+((val>>16)&0x0000ffff);
}

static inline uindex_t fs_fragvector_getLsb32(uindex_t val) {
    uindex_t tmp=val;
    val|=(val<<1);
    val|=(val<<2);
    val|=(val<<4);
    val|=(val<<8);
    val|=(val<<16);
    return (tmp==0) ? 0: 32-fs_fragvector_getnumbits(val);
}

static inline uindex_t fs_fragvector_getMsb32(uindex_t val) {
    uindex_t tmp=val;
    val|=(val>>1);
    val|=(val>>2);
    val|=(val>>4);
    val|=(val>>8);
    val|=(val>>16);
    return (tmp==0) ? 0: fs_fragvector_getnumbits(val)-1;
}

#pragma pack(push, 1)
typedef struct _tag_VECTOR_DATA {
    byte_t data[CLUSTER_SIZE];
} VECTOR_DATA;
#pragma pack(pop)

typedef enum _tag_fragvector_status {
    FRAGVECTOR_SUCCESS = 0,
    FRAGVECTOR_ERROR_PARAM = 1,
    FRAGVECTOR_ERROR_MEMORY_ALLOCATE_FAILURE = 2,
} fragvector_status;

typedef struct _tag_FS_FRAGMENT_VECTOR {
    index_t numOfBufferAry;
    index_t numOfUsedBufferAry;
    index_t firstArrayInsert;
    index_t reallocArrayInsert;
    byte_t **bufferArray;
    index_t firstArrayShift;
    index_t reallocArrayShift;
    fsize_t maxArraySize;
    fsize_t reallocSize;
    index_t currentIndex;
    fsize_t alignSize;
    fsize_t addSize;
    fragvector_status status;
} FSFRAGVECTOR;

static inline bool_t fs_fragvector_setsuccess(FSFRAGVECTOR *fvp) {
    fvp->status = FRAGVECTOR_SUCCESS;
    return true_t;
}

static inline bool_t fs_fragvector_seterror(FSFRAGVECTOR *fvp, fragvector_status status) {
    fvp->status = status;
    return false_t;
}

/* Note: There is NO problem even if used first_size to 0. */
static inline bool_t fs_fragvector_open(FSFRAGVECTOR **fvp, fsize_t first_size, fsize_t realloc_size) {
    *fvp = (FSFRAGVECTOR *)fs_malloc(sizeof(FSFRAGVECTOR));
    if(!*fvp) return false_t;
    (*fvp)->numOfBufferAry = 0;
    (*fvp)->numOfUsedBufferAry = 0;
    (*fvp)->firstArrayInsert = 0;
    (*fvp)->reallocArrayInsert = 0;
    (*fvp)->bufferArray = NULL;
    (*fvp)->firstArrayShift = 0;
    (*fvp)->reallocArrayShift = 0;
    (*fvp)->maxArraySize = 0;
    (*fvp)->reallocSize = 0;
    (*fvp)->currentIndex = 0;
    (*fvp)->alignSize = 0;
    (*fvp)->addSize = 0;

    (*fvp)->alignSize = V_ALIGNMENT-(sizeof(VECTOR_DATA)&(V_ALIGNMENT-1));
    ((*fvp)->alignSize==V_ALIGNMENT)? (*fvp)->alignSize=0:0;
    (*fvp)->addSize = sizeof(VECTOR_DATA)+(*fvp)->alignSize;
    uindex_t addIndex = fs_fragvector_getMsb32((*fvp)->addSize);
    uindex_t compIndex = fs_fragvector_getLsb32((*fvp)->addSize);
    (*fvp)->addSize = (addIndex!=compIndex)? 1<<++addIndex: 1<<addIndex;
    if(realloc_size<(*fvp)->addSize) return fs_fragvector_seterror(*fvp, FRAGVECTOR_ERROR_PARAM);
    (*fvp)->numOfBufferAry = P_TABLE_NUM;
    (*fvp)->bufferArray = (byte_t **)fs_malloc(sizeof(byte_t *)*(*fvp)->numOfBufferAry);
    if(!(*fvp)->bufferArray) return fs_fragvector_seterror(*fvp, FRAGVECTOR_ERROR_MEMORY_ALLOCATE_FAILURE);
    if(0<first_size) {
        uindex_t firstIndex = fs_fragvector_getMsb32((uindex_t)first_size);
        uindex_t reallocIndex = fs_fragvector_getMsb32((uindex_t)realloc_size);
        uindex_t compFirst = fs_fragvector_getLsb32((uindex_t)first_size);
        uindex_t compRealloc = fs_fragvector_getLsb32((uindex_t)realloc_size);
        first_size = (firstIndex!=compFirst)? (fsize_t)1<<(firstIndex+1): (fsize_t)1<<firstIndex;
        realloc_size = (reallocIndex!=compRealloc)? (fsize_t)1<<(reallocIndex+1): (fsize_t)1<<reallocIndex;
        (*fvp)->bufferArray[0] = (byte_t *)fs_malloc(first_size);
        if(!(*fvp)->bufferArray[0]) return fs_fragvector_seterror(*fvp, FRAGVECTOR_ERROR_MEMORY_ALLOCATE_FAILURE);
        (*fvp)->firstArrayInsert = (index_t)first_size>>addIndex;
        (*fvp)->firstArrayShift = fs_fragvector_getMsb32((*fvp)->firstArrayInsert);
        (*fvp)->reallocArrayInsert = (index_t)realloc_size>>addIndex;
        (*fvp)->reallocArrayShift = fs_fragvector_getMsb32((*fvp)->reallocArrayInsert);
    } else {
        unsigned int reallocIndex = fs_fragvector_getMsb32((unsigned int)realloc_size);
        unsigned int compRealloc = fs_fragvector_getLsb32((unsigned int)realloc_size);
        realloc_size = (reallocIndex!=compRealloc)? (fsize_t)1<<(reallocIndex+1): (fsize_t)1<<reallocIndex;
        (*fvp)->bufferArray[0] = NULL;
        (*fvp)->firstArrayInsert = 0;
        (*fvp)->firstArrayShift = 0;
        (*fvp)->reallocArrayInsert = (index_t)realloc_size>>addIndex;
        (*fvp)->reallocArrayShift = fs_fragvector_getMsb32((*fvp)->reallocArrayInsert);
    }
    (*fvp)->numOfUsedBufferAry = 1;
    (*fvp)->maxArraySize = first_size;
    (*fvp)->reallocSize = realloc_size;
    (*fvp)->currentIndex = 0;
    assert((*fvp)->reallocSize>0);
    return fs_fragvector_setsuccess(*fvp);
}

static inline bool_t fs_fragvector_realloc(FSFRAGVECTOR *fvp) {
    if(fvp->numOfBufferAry<fvp->numOfUsedBufferAry+1) {
        int numofOldAry = fvp->numOfBufferAry;
        fvp->numOfBufferAry+=P_TABLE_NUM;
        byte_t **tmp = (byte_t **)fs_malloc(sizeof(byte_t *)*fvp->numOfBufferAry);
        memcpy(tmp, fvp->bufferArray, numofOldAry*sizeof(byte_t *));
        fs_free(fvp->bufferArray, true_t); fvp->bufferArray = tmp;
    }
    fvp->maxArraySize += fvp->reallocSize;
    ++(fvp->numOfUsedBufferAry);
    fvp->bufferArray[fvp->numOfUsedBufferAry-1] = (byte_t *)fs_malloc(fvp->reallocSize);
    return (fvp->bufferArray[fvp->numOfUsedBufferAry-1])? fs_fragvector_setsuccess(fvp): fs_fragvector_seterror(fvp, FRAGVECTOR_ERROR_MEMORY_ALLOCATE_FAILURE);
}

static inline byte_t *fs_fragvector_getaddr(FSFRAGVECTOR *fvp, index_t index) {
    index_t remain = (index+1) - fvp->firstArrayInsert;
    if(remain<=0) {
        remain+=fvp->firstArrayInsert;
        return fvp->bufferArray[0]+fvp->addSize*(remain-1);
    } else {
        remain-=1;
        const index_t split=remain>>fvp->reallocArrayShift;
        remain&=(1<<fvp->reallocArrayShift)-1;
        return fvp->bufferArray[1+split]+fvp->addSize*remain;
    }
}

static inline VECTOR_DATA *fs_fragvector_getdata(FSFRAGVECTOR *fvp, index_t index) {
    return (VECTOR_DATA *)fs_fragvector_getaddr(fvp, index);
}

static inline bool_t fs_fragvector_insert1(FSFRAGVECTOR *fvp, const VECTOR_DATA *data) {
    while(fvp->maxArraySize<(fvp->currentIndex*fvp->addSize)+fvp->addSize)
        if(!fs_fragvector_realloc(fvp)) return fs_fragvector_seterror(fvp, FRAGVECTOR_ERROR_MEMORY_ALLOCATE_FAILURE);
    memcpy(fs_fragvector_getaddr(fvp, fvp->currentIndex++), data, sizeof(VECTOR_DATA));
    return fs_fragvector_setsuccess(fvp);
}

static inline fsize_t fs_fragvector_getsize(FSFRAGVECTOR *fvp) {
    return fvp->currentIndex;
}

static inline VECTOR_DATA *fs_fragvector_iterator(FSFRAGVECTOR *fvp, index_t current) {
    return (current==fvp->currentIndex) ? NULL: (VECTOR_DATA *)fs_fragvector_getaddr(fvp, fvp->currentIndex);
}

static inline bool_t fs_fragvector_insert2(FSFRAGVECTOR *fvp, VECTOR_DATA **create) {
    while(fvp->maxArraySize<(fvp->currentIndex*fvp->addSize) + fvp->addSize)
        if(!fs_fragvector_realloc(fvp)) return fs_fragvector_seterror(fvp, FRAGVECTOR_ERROR_MEMORY_ALLOCATE_FAILURE);
    *create = (VECTOR_DATA *)fs_fragvector_getaddr(fvp, fvp->currentIndex++);
    return fs_fragvector_setsuccess(fvp);
}

static inline VECTOR_DATA *fs_fragvector_pop(FSFRAGVECTOR *fvp) {
    return (VECTOR_DATA *)fs_fragvector_getaddr(fvp, --(fvp->currentIndex));
}

static inline bool_t fs_fragvector_clear(FSFRAGVECTOR *fvp) {
    fvp->currentIndex = 0;
    return fs_fragvector_setsuccess(fvp);
}

static inline bool_t fs_fragvector_close(FSFRAGVECTOR *fvp) {
    fs_fragvector_clear(fvp);
    for(index_t i=0; i<fvp->numOfUsedBufferAry; ++i)
        fs_free(fvp->bufferArray[i], true_t);
    return fs_free(fvp, fs_free(fvp->bufferArray, true_t));
}

#endif
