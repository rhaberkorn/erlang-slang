LIBS := -lslang
ERL_CPPFLAGS := $(shell erl -noinput -eval \
			'io:format("-I~s/erts-~s/include", [code:root_dir(), erlang:system_info(version)]), halt(0)')

override CFLAGS += -Wall -fpic
override CPPFLAGS += $(ERL_CPPFLAGS)

all : ../priv/slang_drv.so

../priv/slang_drv.so : slang_drv.o
	$(CC) -shared $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) -f *.o ../priv/slang_drv.so
