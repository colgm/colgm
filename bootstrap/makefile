COLGMCC = ./build/colgm

.PHONY: test
test: $(COLGMCC)
	@ $(COLGMCC) test/assign.colgm && lli out.ll
	@ $(COLGMCC) test/bitwise.colgm && lli out.ll
	@ $(COLGMCC) test/branch.colgm && lli out.ll
	@ $(COLGMCC) test/cmpnot.colgm && lli out.ll
	@ $(COLGMCC) test/continue_break.colgm && lli out.ll
	@ $(COLGMCC) test/enum_test.colgm && lli out.ll
	@ $(COLGMCC) test/for_test.colgm && lli out.ll
	@ $(COLGMCC) test/func.colgm && lli out.ll
	@ $(COLGMCC) test/hello.colgm && lli out.ll
	@ $(COLGMCC) test/local.colgm && lli out.ll
	@ $(COLGMCC) test/match.colgm && lli out.ll
	@ $(COLGMCC) test/negative.colgm && lli out.ll
	@ $(COLGMCC) test/self-ref-test.colgm && lli out.ll
	@ $(COLGMCC) test/string.colgm && lli out.ll
	@ $(COLGMCC) test/to_str.colgm && lli out.ll
	@ $(COLGMCC) test/type_convert.colgm && lli out.ll
	@ $(COLGMCC) test/void_return.colgm && lli out.ll
