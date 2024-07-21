COLGMCC = ./build/colgm

.PHONY: test
test: $(COLGMCC)
	@ $(COLGMCC) --mir test/assign.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/bitwise.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/branch.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/continue_break.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/enum_test.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/func.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/hello.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/negative.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/string.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/to_str.colgm
	@ lli out.ll
	@ $(COLGMCC) --mir test/type_convert.colgm
	@ lli out.ll
