#!/bin/bash
rm hello.so
gcc -Ofast -shared -o hello.so src/hello.c src/tiamalgamation.c -lta_lib
./dev.py
