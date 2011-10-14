export CC ?= gcc
export ERL ?= erl
export RM ?= rm

CFLAGS ?= -O2
CPPFLAGS ?=
LDFLAGS ?=

override CFLAGS += -Wall

export CFLAGS CPPFLAGS LDFLAGS

all:
	$(MAKE) -C c_src $@
	$(ERL) -noinput -eval \
	       "case make:all() of up_to_date -> halt(0); error -> halt(1) end"

install:
	$(MAKE) -C c_src $@

clean:
	$(MAKE) -C c_src $@
	$(RM) -f {ebin,demo}/*.beam
