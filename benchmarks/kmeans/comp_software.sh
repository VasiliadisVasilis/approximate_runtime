#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DSANITY -DPROTECT" CC=$1

