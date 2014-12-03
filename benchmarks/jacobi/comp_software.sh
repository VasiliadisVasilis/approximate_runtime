#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DPROTECT" CC=$1
