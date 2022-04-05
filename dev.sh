#!/bin/bash
rm hello.so
gcc -O3 -shared -o hello.so src/hello.c src/tiamalgamation.c -lta_lib -fPIC
./dev.py
