export CC := gcc
export ERL := erl
export RM := rm

export CFLAGS ?= -O2
export CPPFLAGS ?=
export LDFLAGS ?=

all:
	$(MAKE) -C c_src $@
	$(ERL) -noinput -eval \
	       "case make:all() of up_to_date -> halt(0); error -> halt(1) end"

install:
	$(MAKE) -C c_src $@

clean:
	$(MAKE) -C c_src $@
	$(RM) -f {ebin,demo}/*.beam
