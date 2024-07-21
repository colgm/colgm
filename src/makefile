out.ll: ../bootstrap/build/colgm ast/*.colgm err/*.colgm std/*.colgm ./*.colgm
	../bootstrap/build/colgm -L . main.colgm --mir

.PHONY=clean
clean: out.ll
	rm out.ll