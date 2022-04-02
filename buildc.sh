#!/bin/bash
gcc -o hello src/hello.c
gcc -shared -o hello.so src/hello.c
