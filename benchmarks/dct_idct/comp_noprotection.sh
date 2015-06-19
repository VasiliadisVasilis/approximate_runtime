#!/bin/bash
make clean
make CFLAGS="-DGEMFI" CC=$1
