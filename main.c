// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fs_memory.h"
#include "fs_file.h"
#include "fs_disk.h"
#include "mini_filesystem.h"

#ifdef WIN32
#include "windows.h"
#endif

#ifdef WIN32

INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MessageBoxA(NULL, "memory test.", "test 1", MB_OK);
    for(index_t i=0; i < 10000; ++i) {
        byte_t *ptr = fs_malloc(1024);
        if(ptr) {
            for(index_t k=0; k < 1024; ++k)
                ptr[k] = (byte_t)rand();
            fs_free(ptr, true_t);
        }
    }
    /*
    {
        byte_t *ptr = fs_malloc(1024);
        for(index_t k = 0; k < 1025; ++k)
            ptr[k] = (byte_t)rand();
        fs_free(ptr, true_t); // OK assert
    }
    */

    MessageBoxA(NULL, "file test.", "test 2", MB_OK);
    FSFILE *fp;
    assert(fs_file_open(&fp, "D:\\fsdisk\\fsindex0001.dat"));
    byte_t *data = fs_malloc(fs_file_getsize());
    assert(data);
    assert(fs_file_read(fp, data, fs_file_getsize()));
    fs_free(data, fs_file_close(fp, true_t));

    MessageBoxA(NULL, "disk test.", "test 3", MB_OK);
    for(index_t test=0; test < 64; ++test) {
        const sector_t begin = rand() % 10000000;
        const counter_t num  = rand() % 94581920;
        if(num==0) continue;
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

    




    MessageBoxA(NULL, "completed all tests.", "success", MB_OK);
    return 0;
}

#else

int main(int argc, char *argv[])
{
    return 0;
}

#endif
