#!/bin/bash
rm hello.so
gcc -shared -o hello.so src/hello.c -lta_lib
./dev.py
