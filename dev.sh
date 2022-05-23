#!/bin/bash
rm pioneer.so
gcc -O3 -shared -o pioneer.so src/t07.c src/tiamalgamation.c -fPIC
./dev.py
