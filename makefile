COLGMCC = ./build/colgm

colgm.ll: $(COLGMCC) src/ast/*.colgm src/err/*.colgm src/sema/*.colgm src/*.colgm src/std/*.colgm
	$(COLGMCC) --library src src/main.colgm -o colgm.ll

TEST_LIB = --library test/test_lib

.PHONY: test
test: $(COLGMCC) test/*
	@ $(COLGMCC) test/assign.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/bitwise.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/branch.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/cmpnot.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/continue_break.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/enum_test.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/for_test.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/func.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/hello.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/initializer.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/local.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/match.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/negative.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/self-ref-test.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/string.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/to_str.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/type_convert.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/void_return.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/generic.colgm $(TEST_LIB) && lli out.ll
	@ rm out.ll

.PHONY: clean
clean: out.ll
	rm out.ll