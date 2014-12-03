#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DSANITY -DPROTECT -DRAZOR" CC=$1
