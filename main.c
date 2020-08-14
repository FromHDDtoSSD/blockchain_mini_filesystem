// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma warning(disable: 4100)
#include "fs_const.h"
#include "fs_memory.h"
#include "fs_file.h"
#include "fs_disk.h"
#include "fs_bitmap.h"
#include "fs_fragment_vector.h"
#include "fs_datastream.h"
#include "fs_btree.h"

//[OK]#define FS_TEST1
//[OK]#define FS_TEST2
//[OK]#define FS_TEST3
//[OK]#define FS_TEST4
#define FS_TEST5

#ifdef WIN32
#include <windows.h>
#endif

static const str_t *target_dir = "D:\\fsdisk";

int main(int argc, char *argv[]) {
#ifdef FS_TEST1
# ifdef WIN32
    MessageBoxA(NULL, "memory test.", "test 1", MB_OK);
# else
    printf("test1: memory test.\n");
# endif
    for(index_t i=0; i < 10000; ++i) {
        byte_t *ptr = fs_malloc(1024);
        byte_t *ptr2 = fs_malloc(1024);
        if(ptr) {
            for(index_t k=0; k < 1024; ++k)
                ptr[k] = (byte_t)rand();
            memcpy_s(ptr2, 1024, ptr, 1024);
            assert(memcmp_s(ptr, 1024, ptr2, 1024)==0);
            fs_free(ptr2, fs_free(ptr, b_true));
        }
    }
    /*
    {
        byte_t *ptr = fs_malloc(1024);
        for(index_t k = 0; k < 1025; ++k)
            ptr[k] = (byte_t)rand();
        fs_free(ptr, b_true); // OK assert
    }
    */
#endif

#ifdef FS_TEST2
# ifdef WIN32
    MessageBoxA(NULL, "file test.", "test 2", MB_OK);
# else
    printf("test2: file test.\n");
# endif
    FSFILE *fp;
    assert(fs_file_open(&fp, "D:\\fsdisk\\fsindex0001.dat"));
    byte_t *data = fs_malloc(fs_file_getsize());
    assert(data);
    assert(fs_file_read(fp, data, fs_file_getsize()));
    fs_free(data, fs_file_close(fp, b_true));
#endif

#ifdef FS_TEST3
# ifdef WIN32
    MessageBoxA(NULL, "disk test.", "test 3", MB_OK);
# else
    printf("test3: disk test.\n");
# endif
    for(index_t test=0; test < 10; ++test) {
        const sector_t begin = rand() % 10000000;
        const counter_t num  = rand() % 94581920;
        if(num==0) assert(0);
        const fsize_t bsize = num * BYTES_PER_SECTOR;
        byte_t *wbuf = fs_malloc(bsize);
        assert(wbuf);
        {
            FSDISK *fdp1;
            assert(fs_disk_open(&fdp1, target_dir));
            for(index_t i = 0; i < bsize; ++i) wbuf[i] = (byte_t)rand();
            assert(fs_disk_write(fdp1, begin, num, wbuf));
            fs_disk_close(fdp1, b_true);
        }
        {
            FSDISK *fdp2;
            assert(fs_disk_open(&fdp2, target_dir));
            byte_t *rbuf = fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdp2, begin, num, rbuf));
            assert(memcmp(wbuf, rbuf, bsize)==0);
            fs_free(rbuf, fs_disk_close(fdp2, b_true));
        }
        {
            FSDISK *fdpA;
            assert(fs_disk_open(&fdpA, target_dir));
            byte_t *rbuf = fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdpA, begin, num, rbuf));
            assert(memcmp(wbuf, rbuf, bsize)==0);
            fs_free(rbuf, fs_disk_close(fdpA, b_true));
        }
        {
            FSDISK *fdpB;
            assert(fs_disk_open(&fdpB, target_dir));
            for(index_t i = 0; i < bsize; ++i) wbuf[i] = (byte_t)rand();
            assert(fs_disk_write(fdpB, begin, num, wbuf));
            fs_disk_close(fdpB, b_true);
        }
        {
            FSDISK *fdpC;
            assert(fs_disk_open(&fdpC, target_dir));
            byte_t *rbuf = fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdpC, begin, num, rbuf));
            assert(memcmp(wbuf, rbuf, bsize)==0);
            fs_free(rbuf, fs_disk_close(fdpC, b_true));
        }
        {
            FSDISK *fdpD;
            assert(fs_disk_open(&fdpD, target_dir));
            assert(fs_disk_write(fdpD, -1*begin, (num>=600)? 60: num, wbuf));
            assert(fs_disk_write(fdpD, -1*begin, num, wbuf));
            assert(fs_disk_read(fdpD, -1*begin, num, wbuf));
            fs_disk_close(fdpD, b_true);
            assert(fs_disk_open(&fdpD, target_dir));
            byte_t *rbuf=fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdpD, -1*begin, num, rbuf));
            assert(memcmp(rbuf+BYTES_PER_SECTOR, wbuf+BYTES_PER_SECTOR, bsize-BYTES_PER_SECTOR)==0);
            fs_free(rbuf, fs_disk_close(fdpD, b_true));
        }
        {
            FSDISK *fdpD;
            assert(fs_disk_open(&fdpD, target_dir));
            assert(fs_disk_write(fdpD, -1*begin, (num>=600)? 60: num, wbuf));
            assert(fs_disk_write(fdpD, -1*begin, num, wbuf));
            assert(fs_disk_read(fdpD, -1*begin, num, wbuf));
            fs_disk_close(fdpD, b_true);
        }
        {
            FSDISK *fdpF;
            assert(fs_disk_open(&fdpF, target_dir));
            byte_t *rbuf=fs_malloc(bsize);
            assert(rbuf);
            assert(fs_disk_read(fdpF, -1*begin, num, rbuf));
            assert(memcmp(rbuf+BYTES_PER_SECTOR, wbuf+BYTES_PER_SECTOR, bsize-BYTES_PER_SECTOR)==0);
            fs_free(rbuf, fs_disk_close(fdpF, b_true));
        }
        fs_free(wbuf, b_true);
    }
#endif

#ifdef FS_TEST4
# ifdef WIN32
    MessageBoxA(NULL, "bitmap sector test.", "test 4", MB_OK);
# else
    printf("test4: bitmap test.\n");
# endif
    for(index_t test = 0; test < 30; ++test) {
        const sector_t begin = rand() % 100000;
        const sector_t num   = rand() % 9860000;
        if(num == 0) assert(0);
        const llsize_t bsize=num*BYTES_PER_SECTOR;
        byte_t *wbuf = fs_malloc((fsize_t)bsize);
        assert(wbuf);
        {
            FSDISK *fdp;
            FSBITMAP *bp;
            assert(fs_disk_open(&fdp, target_dir));
            assert(fs_bitmap_open(&bp, fdp));
            for(index_t i=0; i<bsize; ++i) wbuf[i]=(byte_t)rand();
            assert(fs_diskwith_bitmap_write(bp, begin, num, wbuf));
            if(_BITS_PER_SECTOR<=begin) { /* Note: No write bitmap, 0 - 4095 */
                bool_t used=b_false;
                assert(fs_bitmap_getmask(bp, begin, &used));
                assert(used);
                assert(fs_bitmap_getmask_someusedrange(bp, begin, num, &used));
                assert(used);
                assert(fs_bitmap_getmask_allusedrange(bp, begin, num, &used));
                assert(used);
                sector_t nsec;
                assert(fs_bitmap_getmask_freesector(bp, rand()%154367, &nsec));
                fs_printf("new free sector: %I64d\n", nsec);
                assert(fs_diskwith_bitmap_erase(bp, begin, num/2));
                assert(fs_bitmap_getmask_allusedrange(bp, begin+num/2, num/2, &used));
                assert(used);
                assert(fs_bitmap_getmask_allusedrange(bp, begin, num/2, &used));
                assert(!used);
                fs_disk_close(fdp, fs_bitmap_close(bp, b_true));
            } else {
                bool_t used=b_false;
                assert(fs_bitmap_getmask(bp, begin, &used)==b_false);
                fs_disk_close(fdp, fs_bitmap_close(bp, b_true));
            }
        }
        fs_free(wbuf, b_true);
    }
#endif

#ifdef FS_TEST5
# ifdef WIN32
    MessageBoxA(NULL, "fragment vector, datastream, btree on memory test.", "test 5", MB_OK);
# else
    printf("test5: fragment vector, datastream, btree on memory test.\n");
# endif
    FSBTREE *fbp;
    assert(fs_btree_open(&fbp, 18, sizeof(VECTOR_DATA), sizeof(VECTOR_DATA))); /* ksize: key max size, dsize: data max size */
    static const char key[][sizeof(VECTOR_DATA)] = {
        "dog",
        "cat",
        "drive",
        "Quantum computer",
        "What is Cryptocurrency?",
    };
    static const char data[][sizeof(VECTOR_DATA)] = {
        "DOGE wo nageru no desu.",
        "MIKE neko.",
        "SORA",
        "When we asked a specialized field acquaintance in Japan, he said that the half century would be impossible to realize. \
Is that true? We always don't get reply that much, but ... this time was so early reply!",
        "Oh ... I don't know even a flash memory. haha, this is a joke.",
    };
    for(index_t i=10000; 0<=i; --i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        assert(fs_btree_insert(fbp, _key, _data));
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    }
    for(index_t i=0; i<3; ++i) {
        assert(fs_btree_insert(fbp, key[i], (const byte_t *)data[i]));
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    }
    for(index_t i=10001; i<25000; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        assert(fs_btree_insert(fbp, _key, _data));
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    }
    assert(fs_btree_insert(fbp, key[3], (const byte_t *)data[3]));
    assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    for(index_t i=8000; i<12000; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        assert(fs_btree_insert(fbp, _key, _data));
        assert(fs_btree_getstatus(fbp)==BTREE_NO_ACCEPT); /* No double insert */
    }
    assert(fs_btree_insert(fbp, key[4], (const byte *)data[4]));
    assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);

    {
        SRND *srnd;
        assert(fs_btree_getdata(fbp, key[2], &srnd));
        assert(strcmp(data[2], (const char *)fs_datastream_getdata(srnd))==0);
        fs_btree_free(srnd, b_true);
    }
    for(index_t i=0; i<12000; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        SRND *srnd;
        assert(fs_btree_getdata(fbp, _key, &srnd));
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS); /* exist data */
        assert(strcmp((const str_t *)_data, (const char *)fs_datastream_getdata(srnd))==0);
        fs_btree_free(srnd, b_true);
    }
    for(index_t i=25000; i<32300; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        SRND *srnd;
        assert(fs_btree_getdata(fbp, _key, &srnd));
        assert(fs_btree_getstatus(fbp)==BTREE_NO_DATA); /* no data */
    }

    assert(fs_btree_remove(fbp, key[0]));
    assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    assert(fs_btree_remove(fbp, key[1]));
    assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    assert(fs_btree_remove(fbp, key[2]));
    assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    {
        SRND *srnd;
        assert(fs_btree_getdata(fbp, key[3], &srnd));
        assert(strcmp(data[3], (const char *)fs_datastream_getdata(srnd))==0);
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
        fs_btree_free(srnd, b_true);
    }
    for(index_t i=9300; i<17600; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        assert(fs_btree_remove(fbp, _key));
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS);
    }
    for(index_t i=9800; i<17600; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        SRND *srnd;
        assert(fs_btree_getdata(fbp, _key, &srnd));
        assert(fs_btree_getstatus(fbp)==BTREE_NO_DATA); /* no data */
    }
    for(index_t i=17600; i<25000; ++i) {
        str_t _key[sizeof(VECTOR_DATA)]={0}; byte_t _data[sizeof(VECTOR_DATA)]={0};
        sprintf_s(_key, ARRAYLEN(_key), "%d__key", i);
        sprintf_s((str_t *)_data, ARRAYLEN(_data), "%d__data", i);
        SRND *srnd;
        assert(fs_btree_getdata(fbp, _key, &srnd));
        assert(fs_btree_getstatus(fbp)==BTREE_SUCCESS); /* exist data */
        assert(strcmp((const str_t *)_data, (const char *)fs_datastream_getdata(srnd))==0);
        fs_btree_free(srnd, b_true);
    }

    fs_btree_close(fbp, b_true);
#endif





#ifdef WIN32
    MessageBoxA(NULL, "all test.", "complete success.", MB_OK);
#else
    printf("all test: complete success.\n");
#endif
    return 0;
}
