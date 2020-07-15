// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fs_memory.h"
#include "fs_file.h"
#include "mini_filesystem.h"

#ifdef WIN32
#include "windows.h"
#endif

#ifdef WIN32

INT_PTR WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MessageBoxA(NULL, "memory test.", "test 1", MB_OK);
    for(int i=0; i < 10000; ++i) {
        byte_t *ptr = fs_malloc(1024);
        if(ptr) {
            for(int k=0; k < 1024; ++k)
                ptr[k] = (byte_t)rand();
            fs_free(ptr, true_t);
        }
    }
    /*
    {
        byte_t *ptr = fs_malloc(1024);
        for(int k = 0; k < 1025; ++k)
            ptr[k] = (byte_t)rand();
        fs_free(ptr, true_t); // OK assert
    }
    */

    MessageBoxA(NULL, "file test.", "test 2", MB_OK);
    FILE *fp;
    assert(fs_open(&fp, "D:\\fsdisk\\fsindex0001.dat"));
    byte_t *data = fs_malloc(fs_getsize());
    assert(data);
    assert(fs_read(fp, data, fs_getsize()));
    for(int i=0; i < fs_getsize(); ++i)
        assert(data[i] == 0x00);
    fs_free(data, fs_close(fp, true_t));






    MessageBoxA(NULL, "completed all tests.", "success", MB_OK);
    return 0;
}

#else

int main(int argc, char *argv[])
{
    return 0;
}

#endif
