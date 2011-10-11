
CC=gcc
ERLDIR=/usr/lib/erlang
SLANG_INCLUDE=/usr/include/slang
LD_SHARED=ld -shared

ERLC = erlc
ERLC_FLAGS+=-W $(DEBUG_FLAGS)

../ebin/%.beam: %.erl
	$(ERLC) -b beam $(ERLC_FLAGS) -o ../ebin $<

%.beam: %.erl
	$(ERLC) -b beam $(ERLC_FLAGS)  $<

