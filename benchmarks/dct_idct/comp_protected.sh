#~/bin/bash
if [ $# -eq 0 ]
then
  CC=gcc
else
  CC=$1
fi
make clean
make CFLAGS="-DSANITY -DPROTECT" CC=$CC
