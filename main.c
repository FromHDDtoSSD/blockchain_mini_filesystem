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
    //for(index_t i=0; i < fs_file_getsize(); ++i)
    //    assert(data[i] == 0x00);
    fs_free(data, fs_file_close(fp, true_t));

    MessageBoxA(NULL, "file disk.", "test 3", MB_OK);
    {
        FSDISK *fdp;
        assert(fs_disk_open(&fdp));
        sector_t begin = 100;
        counter_t num = 28192;
        byte_t *buf = fs_malloc(num * SECTOR_SIZE);
        assert(buf);
        for(index_t i = 0; i < num; ++i)
            buf[i] = 0x00; // (byte_t)rand();
        assert(fs_disk_write(fdp, begin, num, buf));
        fs_free(buf, fs_disk_close(fdp, true_t));
    }
    {
        FSDISK *fdp;
        assert(fs_disk_open(&fdp));
        sector_t begin = 0;
        counter_t num = 8192;
        byte_t *buf = fs_malloc(num * SECTOR_SIZE);
        assert(buf);
        assert(fs_disk_read(fdp, begin, num, buf));
        for(index_t i=0; i < num; ++i)
            assert(buf[i] == 0x00);
        fs_free(buf, fs_disk_close(fdp, true_t));
    }
    {
        FSDISK *fdp;
        assert(fs_disk_open(&fdp));
        sector_t begin = 100;
        counter_t num = 8092;
        byte_t *buf = fs_malloc(num * SECTOR_SIZE);
        assert(buf);
        assert(fs_disk_read(fdp, begin, num, buf));
        for(index_t i = 0; i < num; ++i)
            assert(buf[i] == 0x00);
        fs_free(buf, fs_disk_close(fdp, true_t));
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
