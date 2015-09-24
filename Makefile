C=gcc
CFLAGS=-Wall -g -std=c99 -O2 -D_FILE_OFFSET_BITS=64
INCS=`pkg-config --cflags vips`
LIBS=`pkg-config --libs vips` -lexif -lm

yagp: yagp.c html.h
	$(CC) $(CFLAGS) -o yagp yagp.c $(INCS) $(LIBS)

clean:
	rm -f yagp
