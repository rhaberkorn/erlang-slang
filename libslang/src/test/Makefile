TEST_SCRIPTS = syntax sscanf loops arith array strops bstring \
  pack stdio assoc selfload struct nspace ospath ifeval anytype arrmult \
  nspace2 prep
TEST_PGM = sltest
RUN_TEST_PGM = ./$(TEST_PGM)
SLANGINC = ..
SLANGLIB = ../$(ARCH)objs

run_tests: $(TEST_PGM)
	@echo ""
	@echo "Running tests:"
	@echo ""
	@for X in $(TEST_SCRIPTS); \
	do \
	   $(RUN_TEST_PGM) $$X.sl; \
	done
	touch sltest.c

sltest: sltest.c $(SLANGLIB)/libslang.a
	$(CC) $(CFLAGS) $(LDFLAGS) sltest.c -o sltest -I$(SLANGINC) -L$(SLANGLIB) -lslang $(TCAPLIB) -lm
clean: 
	-/bin/rm -f *~ sltest *.o *.log
