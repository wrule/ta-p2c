#!/bin/bash
gcc -Ofast -o hello src/hello.c
gcc -Ofast -shared -o hello.so src/hello.c
