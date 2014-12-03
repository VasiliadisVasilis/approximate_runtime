#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DPROTECT -DSANITY" CC=$1
