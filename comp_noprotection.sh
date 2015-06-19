#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DNOPROTECT" CC=$1
