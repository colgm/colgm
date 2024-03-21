STD = c++17

ifndef OS
	OS = $(shell uname)
endif
ifeq ($(OS), Darwin)
	CXXFLAGS = -std=$(STD) -c -O3 -fno-exceptions -fPIC -mmacosx-version-min=10.15 -Wall
else
	CXXFLAGS = -std=$(STD) -c -O3 -fno-exceptions -fPIC -Wall
endif

HEADER = \
	src/report.h\
	src/lexer.h\
	src/colgm.h\
	src/parse.h\
	src/semantic.h\
	src/ast/ast.h\
	src/ast/decl.h\
	src/ast/expr.h\
	src/ast/stmt.h\
	src/ast/visitor.h\
	src/ast/dumper.h\
	src/code/context.h\
	src/code/hir.h\
	src/code/sir.h\
	src/code/value.h\
	src/code/ir_gen.h\
	src/package/package.h\
	src/sema/context.h\
	src/sema/symbol.h

OBJECT = \
	build/report.o\
	build/lexer.o\
	build/misc.o\
	build/main.o\
	build/parse.o\
	build/semantic.o\
	build/ast.o\
	build/decl.o\
	build/expr.o\
	build/stmt.o\
	build/visitor.o\
	build/dumper.o\
	build/code_context.o\
	build/hir.o\
	build/sir.o\
	build/value.o\
	build/ir_gen.o\
	build/package.o\
	build/sema_context.o\
	build/symbol.o


# for test
colgm: $(OBJECT) | build
	@if [ OS = "Darwin" ]; then\
		$(CXX) $(OBJECT) -O3 -o colgm -ldl -lpthread -stdlib=libc++ -static-libstdc++;\
	else\
		$(CXX) $(OBJECT) -O3 -o colgm -ldl -lpthread;\
	fi

colgm.exe: $(OBJECT) | build
	$(CXX) $(OBJECT) -O3 -o colgm.exe

build:
	@ if [ ! -d build ]; then mkdir build; fi

build/main.o: $(HEADER) src/main.cpp | build
	$(CXX) $(CXXFLAGS) src/main.cpp -o build/main.o -I src

build/misc.o: src/colgm.h src/misc.cpp | build
	$(CXX) $(CXXFLAGS) src/misc.cpp -o build/misc.o -I src

build/report.o: src/colgm.h src/report.h src/report.cpp | build
	$(CXX) $(CXXFLAGS) src/report.cpp -o build/report.o -I src

build/lexer.o: src/colgm.h src/report.h src/lexer.h src/lexer.cpp | build
	$(CXX) $(CXXFLAGS) src/lexer.cpp -o build/lexer.o -I src

build/parse.o: $(HEADER) src/parse.cpp | build
	$(CXX) $(CXXFLAGS) src/parse.cpp -o build/parse.o -I src

build/semantic.o: $(HEADER) src/semantic.cpp | build
	$(CXX) $(CXXFLAGS) src/semantic.cpp -o build/semantic.o -I src

build/ast.o: $(HEADER) src/ast/ast.cpp | build
	$(CXX) $(CXXFLAGS) src/ast/ast.cpp -o build/ast.o -I src

build/decl.o: $(HEADER) src/ast/decl.cpp | build
	$(CXX) $(CXXFLAGS) src/ast/decl.cpp -o build/decl.o -I src

build/expr.o: $(HEADER) src/ast/expr.cpp | build
	$(CXX) $(CXXFLAGS) src/ast/expr.cpp -o build/expr.o -I src

build/stmt.o: $(HEADER) src/ast/stmt.cpp | build
	$(CXX) $(CXXFLAGS) src/ast/stmt.cpp -o build/stmt.o -I src

build/visitor.o: $(HEADER) src/ast/visitor.cpp | build
	$(CXX) $(CXXFLAGS) src/ast/visitor.cpp -o build/visitor.o -I src

build/dumper.o: $(HEADER) src/ast/dumper.cpp | build
	$(CXX) $(CXXFLAGS) src/ast/dumper.cpp -o build/dumper.o -I src

build/code_context.o: $(HEADER) src/code/context.cpp | build
	$(CXX) $(CXXFLAGS) src/code/context.cpp -o build/code_context.o -I src

build/hir.o: $(HEADER) src/code/hir.cpp | build
	$(CXX) $(CXXFLAGS) src/code/hir.cpp -o build/hir.o -I src

build/sir.o: $(HEADER) src/code/sir.cpp | build
	$(CXX) $(CXXFLAGS) src/code/sir.cpp -o build/sir.o -I src

build/value.o: $(HEADER) src/code/value.cpp | build
	$(CXX) $(CXXFLAGS) src/code/value.cpp -o build/value.o -I src

build/ir_gen.o: $(HEADER) src/code/ir_gen.cpp | build
	$(CXX) $(CXXFLAGS) src/code/ir_gen.cpp -o build/ir_gen.o -I src

build/package.o: $(HEADER) src/package/package.cpp | build
	$(CXX) $(CXXFLAGS) src/package/package.cpp -o build/package.o -I src

build/sema_context.o: $(HEADER) src/sema/context.cpp | build
	$(CXX) $(CXXFLAGS) src/sema/context.cpp -o build/sema_context.o -I src

build/symbol.o: $(HEADER) src/sema/symbol.cpp | build
	$(CXX) $(CXXFLAGS) src/sema/symbol.cpp -o build/symbol.o -I src

out.ll: colgm
	./colgm ../src/main.colgm -L ../src -Wt

.PHONY: clean
clean:
	@ echo "[clean] colgm" && if [ -e colgm ]; then rm colgm; fi
	@ echo "[clean] colgm.exe" && if [ -e colgm.exe ]; then rm colgm.exe; fi
	@ rm $(OBJECT)

.PHONY: test
test: colgm
	@ ./colgm test/func.colgm
	@ lli out.ll
	@ ./colgm test/hello.colgm
	@ lli out.ll
