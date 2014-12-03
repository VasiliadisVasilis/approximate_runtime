#!/bin/bash
make clean
make CFLAGS="-DGEMFI -DALLFEATURES" CC=$1
