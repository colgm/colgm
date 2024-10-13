COLGMCC = ./build/colgm

colgm.ll: $(COLGMCC) src/ast/*.colgm src/err/*.colgm src/sema/*.colgm src/*.colgm src/std/*.colgm
	$(COLGMCC) --library src src/main.colgm -o colgm.ll

.PHONY: test
test: $(COLGMCC) bootstrap/test/*
	@ $(COLGMCC) bootstrap/test/assign.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/bitwise.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/branch.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/cmpnot.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/continue_break.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/enum_test.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/for_test.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/func.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/hello.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/initializer.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/local.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/match.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/negative.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/self-ref-test.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/string.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/to_str.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/type_convert.colgm && lli out.ll
	@ $(COLGMCC) bootstrap/test/void_return.colgm && lli out.ll
	-@ $(COLGMCC) bootstrap/test/generic.colgm --library bootstrap/test/test_lib -s
	@ rm out.ll

.PHONY: clean
clean: out.ll
	rm out.ll