#!/bin/bash
gcc -Ofast -shared -o hello.so src/hello.c
./dev.py
