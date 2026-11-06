#!/bin/sh
exec x86_64-w64-mingw32-gcc -O2 -Wall -Wextra -shared -static-libgcc -o version.dll p2.c forward.S version.def -lkernel32
