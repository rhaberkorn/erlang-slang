

ifeq ($(TYPE),debug)
DEBUG_FLAGS = -Ddebug
else
DEBUG_FLAGS =
endif

include ../include.mk

ERLC_FLAGS+=-W $(DEBUG_FLAGS)
MODULES = ex2 ex1 


TARGETS = $(MODULES:%=%.beam) 

all debug:	$(TARGETS) 

clean:
	rm -f $(TARGETS)
