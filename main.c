// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fs_memory.h"
#include "fs_file.h"
#include "fs_disk.h"
#include "fs_bitmap.h"
#include "fs_cluster.h"
#include "fs_bcr.h"
#include "fs_sha256.h"
#include "fs_fragment_vector.h"
#include "fs_datastream.h"
#include "mini_filesystem.h"

#ifdef WIN32
#include "windows.h"
#endif

#ifdef WIN32

INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    /* [OK]
    MessageBoxA(NULL, "memory test.", "test 1", MB_OK);
    for(index_t i=0; i < 10000; ++i) {
        byte_t *ptr = fs_malloc(1024);
        if(ptr) {
            for(index_t k=0; k < 1024; ++k)
                ptr[k] = (byte_t)rand();
            fs_free(ptr, true_t);
        }
    }
    */
    /*
    {
        byte_t *ptr = fs_malloc(1024);
        for(index_t k = 0; k < 1025; ++k)
            ptr[k] = (byte_t)rand();
        fs_free(ptr, true_t); // OK assert
    }
    */

    /* [OK]
    MessageBoxA(NULL, "file test.", "test 2", MB_OK);
    FSFILE *fp;
    assert(fs_file_open(&fp, "D:\\fsdisk\\fsindex0001.dat"));
    byte_t *data = fs_malloc(fs_file_getsize());
    assert(data);
    assert(fs_file_read(fp, data, fs_file_getsize()));
    fs_free(data, fs_file_close(fp, true_t));
    */

    /* [OK]
    MessageBoxA(NULL, "disk test.", "test 3", MB_OK);
    for(index_t test=0; test < 10; ++test) {
        const sector_t begin = rand() % 10000000;
        const counter_t num  = rand() % 94581920;
        if(num==0) assert(0);
        const fsize_t bsize = num * SECTOR_SIZE;
        byte_t *wbuf = fs_malloc(bsize);
        assert(wbuf);
        {
            FSDISK *fdp;
            assert(fs_disk_open(&fdp));
            for(index_t i = 0; i < bsize; ++i)
                wbuf[i] = (byte_t)rand();
            assert(fs_disk_write(fdp, begin, num, wbuf));
            fs_disk_close(fdp, true_t);
        }
        {
            FSDISK *fdp;
            assert(fs_disk_open(&fdp));
            byte_t *rbuf = fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdp, begin, num, rbuf));
            assert(memcmp(wbuf, rbuf, bsize) == 0);
            fs_free(rbuf, fs_disk_close(fdp, true_t));
        }
        fs_free(wbuf, true_t);
    }
    */

    /* [OK]
    MessageBoxA(NULL, "cluster(bitmap) test", "test4", MB_OK);
    */
    /* [OK]
    {
        FSBITMAP *bitmap;
        FSDISK *fdp;
        assert(fs_disk_open(&fdp));
        assert(fs_bitmap_open(&bitmap, fdp));
        fs_bitmap_close(bitmap, fs_disk_close(fdp, true_t));
    }
    */
    /* [OK assert]
    * Despite the fact that there is a Bitmap(FSBITMAP),
    * tried to write data in units of sectors, so confirmed that the assert would come out normally.
    *
    for(index_t test = 0; test < 100; ++test) {
        const sector_t begin = rand() % 10000000;
        const counter_t num  = rand() % 94581920;
        if(BITMAP_SIZE/SECTOR_SIZE > begin%(fs_file_getsize()/SECTOR_SIZE)) assert(0);
        if(num == 0) assert(0);
        const fsize_t bsize = num * SECTOR_SIZE;
        byte_t *wbuf = fs_malloc(bsize);
        FSBITMAP *bitmap;
        assert(wbuf);
        {
            FSDISK *fdp;
            assert(fs_disk_open(&fdp));
            assert(fs_bitmap_open(&bitmap, fdp));
            for(index_t i = 0; i < bsize; ++i)
                wbuf[i] = (byte_t)rand();
            assert(fs_disk_write(fdp, begin, num, wbuf));
            fs_disk_close(fdp, true_t);
        }
        {
            FSDISK *fdp;
            assert(fs_disk_open(&fdp));
            byte_t *rbuf = fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdp, begin, num, rbuf));
            assert(memcmp(wbuf, rbuf, bsize) == 0);
            BPB bpb;
            bpb.bpb_offset = 0;
            bool_t used = false_t;
            cluster_t clus = begin / SECTORS_PER_CLUS;
            assert(fs_bitmap_isused(bitmap, &bpb, clus, &used));
            assert(used);
            clus = (begin + num - 1)/SECTORS_PER_CLUS;
            assert(fs_bitmap_isused(bitmap, &bpb, clus, &used));
            assert(used);
            fs_free(rbuf, fs_disk_close(fdp, true_t));
        }
        fs_free(wbuf, fs_bitmap_close(bitmap, true_t));
    }
    */
    /* [OK]
    for(index_t test = 0; test < 3; ++test) {
        const cluster_t begin = rand() % 100000;
        const counter_t num   = rand() % 986000;
        if(num == 0) assert(0);
        const fsize_t bsize = num * CLUSTER_SIZE;
        byte_t *wbuf = fs_malloc(bsize);
        assert(wbuf);
        BPB bpb;
        bpb.bpb_offset = (sizeof(BITMAP_INFO)+sizeof(BCR))/SECTOR_SIZE * (rand()%150);
        assert(bpb.bpb_offset%SECTORS_PER_CLUS==0);
        {
            FSDISK *fdp;
            FSBITMAP *bp;
            assert(fs_disk_open(&fdp));
            assert(fs_bitmap_open(&bp, &bpb, fdp));
            for(index_t i = 0; i < bsize; ++i)
                wbuf[i]=(byte_t)rand();
            assert(fs_cluster_diskwrite(fdp, bp, &bpb, begin, num, wbuf));
            fs_disk_close(fdp, fs_bitmap_close(bp, true_t));
        }
        {
            FSDISK *fdp;
            FSBITMAP *bp;
            assert(fs_disk_open(&fdp));
            assert(fs_bitmap_open(&bp, &bpb, fdp));
            byte_t *rbuf = fs_malloc(bsize);
            assert(rbuf);
            assert(fs_cluster_diskread(fdp, bp, &bpb, begin, num, rbuf));
            assert(memcmp(wbuf, rbuf, bsize)==0);
            bool_t used = false_t;
            cluster_t clus = begin;
            assert(fs_bitmap_isused(bp, clus, &used));
            assert(used);
            clus = begin + num - 1;
            assert(fs_bitmap_isused(bp, clus, &used));
            assert(used);
            fs_free(rbuf, fs_disk_close(fdp, fs_bitmap_close(bp, true_t)));
        }
        fs_free(wbuf, true_t);
    }
    */

    /* [OK]
    MessageBox(NULL, "hash(sha256) test", "test 5", MB_OK);
    FSSHA256 *sp;
    assert(fs_sha256_open(&sp));
    fs_sha256_init(sp);
    const str_t *str = "";
    assert(fs_sha256_update(sp, 0, str));
    assert(fs_sha256_final(sp));
    fs_sha256_close(sp, true_t);
    */

    /* [OK]
    MessageBox(NULL, "fragmentvector on memory test", "test 6", MB_OK);
    static VECTOR_DATA cmpAA;
    memset(cmpAA.data, 0xAA, sizeof(cmpAA.data));
    static VECTOR_DATA cmpCC;
    memset(cmpCC.data, 0xCC, sizeof(cmpCC.data));
    static VECTOR_DATA cmpEE;
    memset(cmpEE.data, 0xEE, sizeof(cmpEE.data));
    static VECTOR_DATA cmpFF;
    memset(cmpFF.data, 0xFF, sizeof(cmpFF.data));
    for(index_t test=0; test<30; ++test) {
        FSFRAGVECTOR *fvp;
        assert(fs_fragvector_open(&fvp, (test%2)?0:sizeof(VECTOR_DATA), sizeof(VECTOR_DATA)*(rand()%15)));
        for(index_t i=0; i<51200; ++i) {
            VECTOR_DATA vch;
            if(0<=i&&i<100)
                memset(vch.data, 0xAA, sizeof(vch.data));
            else
                memset(vch.data, 0xCC, sizeof(vch.data));
            assert(fs_fragvector_insert1(fvp, &vch));
        }
        for(index_t i=0; i<51200; ++i) {
            VECTOR_DATA *vch;
            assert(fs_fragvector_insert2(fvp, &vch));
            if(1245<=i&&i<3345)
                memset(vch->data, 0xEE, sizeof(vch->data));
            else
                memset(vch->data, 0xFF, sizeof(vch->data));
        }
        for(index_t i=0; i<45000; ++i)
            fs_fragvector_pop(fvp);
        for(index_t i=67; i<100; ++i)
            assert(memcmp(fs_fragvector_getdata(fvp, i)->data, cmpAA.data, sizeof(cmpAA.data))==0);
        for(index_t i=51200+1245; i<51200+3345; ++i)
            assert(memcmp(fs_fragvector_getdata(fvp, i)->data, cmpEE.data, sizeof(cmpEE.data))==0);
        for(index_t i=51200+1245+3345; i<51200+1245+3345+3000; ++i) // Even when fs_fragvector_pop is executed, the allocated memory remains.
            assert(memcmp(fs_fragvector_getdata(fvp, i)->data, cmpFF.data, sizeof(cmpFF.data))==0);
        fs_fragvector_close(fvp, true_t);
    }
    */

    MessageBox(NULL, "datastream on memory test", "test 7", MB_OK);
    typedef struct _tag_VECTOR_DATA2 {
        VECTOR_DATA data1;
        VECTOR_DATA data2;
    } VECTOR_DATA2;
    static VECTOR_DATA cmpCF;
    static VECTOR_DATA2 cmp55FF;
    memset(cmpCF.data, 0xCF, sizeof(cmpCF.data));
    memset(cmp55FF.data1.data, 0x55, sizeof(cmp55FF.data1.data));
    memset(cmp55FF.data2.data, 0xFF, sizeof(cmp55FF.data2.data));
    FSDATASTREAM *dsp;
    assert(fs_datastream_open(&dsp));
    for(index_t i=0; i<102400; ++i)
        assert(fs_datastream_lshift(dsp, &cmpCF, sizeof(VECTOR_DATA)));
    for(index_t i=0; i<102400; ++i)
        assert(fs_datastream_lshift(dsp, &cmp55FF, sizeof(VECTOR_DATA2)));
    for(index_t i=0; i<102400; ++i) {
        SRND *srnd;
        assert(fs_datastream_rshift(dsp, &srnd, sizeof(VECTOR_DATA)));
        assert(memcmp(fs_datastream_getdata(srnd), &cmpCF, sizeof(VECTOR_DATA))==0);
        fs_datastream_free(srnd, true_t);
    }
    for(index_t i = 0; i<102400; ++i) {
        SRND *srnd;
        assert(fs_datastream_rshift(dsp, &srnd, sizeof(VECTOR_DATA2)));
        assert(memcmp(fs_datastream_getdata(srnd), &cmp55FF, sizeof(VECTOR_DATA2))==0);
        fs_datastream_free(srnd, true_t);
    }
    fs_datastream_close(dsp, true_t);


    




    MessageBoxA(NULL, "completed all tests.", "success", MB_OK);
    return 0;
}

#else

int main(int argc, char *argv[])
{
    return 0;
}

#endif
