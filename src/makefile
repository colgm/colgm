out.ll: ../bootstrap/build/colgm ast/*.colgm err/*.colgm std/*.colgm ./*.colgm
	../bootstrap/build/colgm -L . main.colgm

.PHONY=clean
clean: out.ll
	rm out.ll