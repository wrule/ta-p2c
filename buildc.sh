#!/bin/bash
gcc -shared -o hello.so src/hello.c
gcc -o hello src/hello.c
