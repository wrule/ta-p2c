#!/bin/bash
rm hello.so
gcc -shared -o hello.so src/hello.c -L /opt/homebrew/opt/ta-lib/lib
./dev.py
