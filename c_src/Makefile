PRIV := ../priv
LIBSLANG := ../libslang
LIBS := $(LIBSLANG)/src/objs/libslang.a

ERL_CPPFLAGS := $(shell erl -noinput -eval \
			'io:format("-I~s/erts-~s/include", [code:root_dir(), erlang:system_info(version)]), halt(0)')

override CFLAGS += -Wall -fpic
override CPPFLAGS += -I$(LIBSLANG)/src $(ERL_CPPFLAGS)
override LDFLAGS += -fpic

all : $(PRIV)/slang_drv.so

$(PRIV)/slang_drv.so : slang_drv.o
	$(CC) -shared $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) -f *.o $(PRIV)/slang_drv.so
