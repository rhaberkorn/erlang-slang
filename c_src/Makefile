ERL_CPPFLAGS := $(shell erl -noinput -eval \
			'io:format("-I~s/erts-~s/include", [code:root_dir(), erlang:system_info(version)]), halt(0)')

override CFLAGS += -fPIC
override CPPFLAGS += $(ERL_CPPFLAGS)
override LDFLAGS += -shared -lslang

all : ../priv/slang_drv.so

../priv/slang_drv.so : slang_drv.o
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	$(RM) -f *.o ../priv/slang_drv.so
