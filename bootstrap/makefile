COLGMCC = ./build/colgm

.PHONY: out.ll
out.ll:
	$(COLGMCC) ../src/main.colgm -L ../src

.PHONY: test
test: $(COLGMCC)
	@ $(COLGMCC) test/assign.colgm
	@ lli out.ll
	@ $(COLGMCC) test/bitwise.colgm
	@ lli out.ll
	@ $(COLGMCC) test/branch.colgm
	@ lli out.ll
	@ $(COLGMCC) test/continue_break.colgm
	@ lli out.ll
	@ $(COLGMCC) test/func.colgm
	@ lli out.ll
	@ $(COLGMCC) test/hello.colgm
	@ lli out.ll
	@ $(COLGMCC) test/negative.colgm
	@ lli out.ll
	@ $(COLGMCC) test/string.colgm
	@ lli out.ll
	@ $(COLGMCC) test/to_str.colgm
	@ lli out.ll
	@ $(COLGMCC) test/type_convert.colgm
	@ lli out.ll
