// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <stdio.h>
#include <stdlib.h>
#include "fs_memory.h"
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
        for(int k=0; k < 1024; ++k)
            ptr[k] = (byte_t)rand();
        fs_free(ptr);
    }
    /*
    {
        byte_t *ptr = fs_malloc(1024);
        for(int k = 0; k < 1025; ++k)
            ptr[k] = (byte_t)rand();
        fs_free(ptr); // OK assert
    }
    */



    MessageBoxA(NULL, "completed all tests.", "success", MB_OK);
    return 0;
}

#else

int main(int argc, char *argv[])
{
    return 0;
}

#endif
