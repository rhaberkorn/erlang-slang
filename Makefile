export CC := gcc
export ERL := erl
export RM := rm

export CFLAGS ?= -O2
export CPPFLAGS ?=
export LDFLAGS ?=

CONFIGURE_VARS :=

all : libslang/Makefile
	$(MAKE) -C libslang $@
	$(MAKE) -C c_src $@
	$(ERL) -noinput -eval \
	       "case make:all() of up_to_date -> halt(0); error -> halt(1) end"

install:
	$(MAKE) -C c_src $@

clean : libslang/Makefile
	$(MAKE) -C libslang $@
	$(RM) -f libslang/Makefile
	$(MAKE) -C c_src $@
	$(RM) -f ebin/*.beam demo/*.beam

libslang/Makefile : libslang/configure
	( \
		cd libslang; \
		CFLAGS="$(CFLAGS) -fpic" \
		 ./configure $(CONFIGURE_ARGS) \
	)
