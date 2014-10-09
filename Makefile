C=gcc
CFLAGS=-Wall -std=c99 -O2 -D_FILE_OFFSET_BITS=64
INCS=`GraphicsMagickWand-config --cppflags`
LIBS=`GraphicsMagickWand-config --libs` -lexif -lm

yagp: yagp.c html.h
	$(CC) $(CFLAGS) -o yagp yagp.c $(INCS) $(LIBS)

clean:
	rm -f yagp
