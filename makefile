COLGMCC = ./build/colgm

colgm.ll: $(COLGMCC) src/**/*.colgm src/*.colgm
	$(COLGMCC) --library src --pass-info src/main.colgm -o colgm.ll
	$(COLGMCC) --library src --pass-info src/test/test.colgm -o test.ll

TEST_LIB = --library test/test_lib

.PHONY: test
test: $(COLGMCC) test/*
	@ $(COLGMCC) test/assign.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/bitwise.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/branch.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/cmpnot.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/complex_generics.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/continue_break.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/enum_test.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/for_test.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/func.colgm $(TEST_LIB) && lli out.ll
	@ $(COLGMCC) test/generic.colgm $(TEST_LIB) && lli out.ll
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
	@ $(COLGMCC) test/warn_on_left_call.colgm $(TEST_LIB) && lli out.ll
	@ rm out.ll

.PHONY: clean
clean: out.ll
	rm out.ll