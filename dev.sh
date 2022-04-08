#!/bin/bash
rm pioneer.so
gcc -Ofast -shared -o pioneer.so src/t07.c src/tiamalgamation.c
./dev.py
