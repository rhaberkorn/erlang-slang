


ifeq ($(TYPE),debug)
EXTRA_FLAGS = -g
else
EXTRA_FLAGS =
endif

include  ../include.mk
PIC=-fpic
ERL = $(ERLDIR)/usr/include


all:    ../priv/slang_drv.so


slang_drv.o: slang_drv.c
	$(CC) -o slang_drv.o -c $(PIC) $(EXTRA_FLAGS)  -I$(ERL) \
	-I$(SLANG_INCLUDE) -I/usr/local/include slang_drv.c 

../priv/slang_drv.so:	slang_drv.o
	mkdir -p ../priv
	$(CC) -shared -o ../priv/slang_drv.so slang_drv.o \
	-L/usr/local/lib -lslang

clean:
	-rm -f *.o ../priv/*.so *~ 2> /dev/null
