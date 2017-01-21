C=gcc
CFLAGS=-Wall -g -std=c99 -pedantic -O2 -D_FILE_OFFSET_BITS=64 -fstack-protector-strong -fPIC
LDFLAGS=-Wl,-z,now -pie
INCS=`pkg-config --cflags vips`
LIBS=`pkg-config --libs vips` -lexif -lm

yagp: yagp.c html.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o yagp yagp.c $(INCS) $(LIBS)

clean:
	rm -f yagp
