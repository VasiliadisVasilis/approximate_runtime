#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DNON_SIGNIFICANT_TASKS -DPROTECT" CC=$1
