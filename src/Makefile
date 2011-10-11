


ifeq ($(TYPE),debug)
DEBUG_FLAGS = -Ddebug
else
DEBUG_FLAGS =
endif

include ../include.mk
include ../vsn.mk

ERLC_FLAGS+=-W $(DEBUG_FLAGS)
MODULES = slang

TARGETS = $(MODULES:%=../ebin/%.beam) ../ebin/slang.app


all debug:	$(TARGETS)
clean:
	rm -f $(TARGETS)
../ebin/slang.app:	slang.app.src
	cat slang.app.src | sed 's/%VSN%/${SLANG_VSN}/' > ../ebin/slang.app
