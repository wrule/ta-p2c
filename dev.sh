#!/bin/bash
rm pioneer.so
gcc -Ofast -shared -o pioneer.so src/pioneer.c src/tiamalgamation.c -lta_lib
./dev.py
