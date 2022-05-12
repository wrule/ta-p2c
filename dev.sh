#!/bin/bash
rm pioneer.so
gcc -O3 -shared -o pioneer.so src/jimao.c src/tiamalgamation.c -fPIC
./dev.py
