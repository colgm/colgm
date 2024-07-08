out.ll: ast/*.colgm err/*.colgm std/*.colgm cli.colgm main.colgm
	../bootstrap/build/colgm -L . main.colgm

.PHONY=clean
clean: out.ll
	rm out.ll