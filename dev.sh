#!/bin/bash
rm hello.so
gcc -shared -o hello.so src/hello.c
./dev.py
