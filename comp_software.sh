#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DSOFTWARE" CC=$1
