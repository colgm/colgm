COLGMCC = ../build/colgm

.PHONY: out.ll
out.ll:
	$(COLGMCC) ../src/main.colgm -L ../src -Wt

.PHONY: test
test: $(COLGMCC)
	@ $(COLGMCC) test/func.colgm
	@ lli out.ll
	@ $(COLGMCC) test/hello.colgm
	@ lli out.ll
