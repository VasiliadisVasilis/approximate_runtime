#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DSANITY -DPROTECT -DTIMER" CC=$1
