#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DNON_SIGNIFICANT_TASKS -DSANITY -DPROTECT -DTIMER" CC=$1
