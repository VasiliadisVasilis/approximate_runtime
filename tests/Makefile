CFLAGS=-g -pthread -Wall -lOpenCL -lm -O3 -DDEBUG
LDFLAGS=-lrtsrel -lm -lrt -ldl 
LIB=..
INCLUDE=-I../include/

PROG=test1.bin taylor.bin

all: ${PROG}

${PROG}: test1.c taylor.c
	make -C ../
	gcc ${CFLAGS} -L${LIB} ${INCLUDE} taylor.c ${LDFLAGS} -o taylor.bin
	gcc ${CFLAGS} -L${LIB} ${INCLUDE} test1.c ${LDFLAGS} -o test1.bin	
	gcc ${CFLAGS} -L${LIB} ${INCLUDE} dependence.c ${LDFLAGS} -o dependence.bin
# %.bin: %.c ../libadamsmp.a
# 	gcc ${CFLAGS} -L${ADAMLIB} ${INCLUDE} $< -o $@ ${LDFLAGS}
# 
# taylor: taylor_series_bddt.c ../libadamsmp.a
# 	gcc ${CFLAGS} -L${ADAMLIB} ${INCLUDE} $< -o $@ ${LDFLAGS}
# 
# taylor_series_bddt.c: taylor_series_scoop.c ../libadamsmp.a
# 	scoop ${INCLUDE} $< --out-name=taylor_series_bddt --verbose --out=log --tpcIncludePath=../include --runtime=bddt --debugSCOOP
# 
clean:
	rm -f ${PROG} #taylor_series_bddt.c
