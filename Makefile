
DIRS = src c_src demo

all clean:
	@set -e ; \
		for d in $(DIRS) ; do \
		if [ -d $$d ]; then ( cd $$d && $(MAKE) $@ ) || exit 1 ; fi ; \
		done

debug:
	@set -e ; \
		for d in $(DIRS) ; do \
		if [ -d $$d ]; then ( cd $$d && $(MAKE) TYPE=debug  ) || exit 1 ; fi ; \
		done



# possibly with --with-slang-include arg
conf:
	(cd config; ./configure)


