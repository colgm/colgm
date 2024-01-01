STD = c++17

ifndef OS
	OS = $(shell uname)
endif
ifeq ($(OS), Darwin)
	CXXFLAGS = -std=$(STD) -c -O3 -fno-exceptions -fPIC -mmacosx-version-min=10.15
else
	CXXFLAGS = -std=$(STD) -c -O3 -fno-exceptions -fPIC
endif

HEADER = \
	src/err.h\
	src/lexer.h\
	src/colgm.h\
	src/parse.h\
	src/codegen.h\
	src/semantic.h\
	src/ast/ast.h\
	src/ast/decl.h\
	src/ast/expr.h\
	src/ast/stmt.h\
	src/ast/visitor.h\
	src/ast/dumper.h

OBJECT = \
	build/err.o\
	build/lexer.o\
	build/misc.o\
	build/main.o\
	build/parse.o\
	build/codegen.o\
	build/semantic.o\
	build/ast.o\
	build/decl.o\
	build/expr.o\
	build/stmt.o\
	build/visitor.o\
	build/dumper.o


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

build/err.o: src/colgm.h src/err.h src/err.cpp | build
	$(CXX) $(CXXFLAGS) src/err.cpp -o build/err.o -I src

build/lexer.o: src/colgm.h src/err.h src/lexer.h src/lexer.cpp | build
	$(CXX) $(CXXFLAGS) src/lexer.cpp -o build/lexer.o -I src

build/parse.o: $(HEADER) src/parse.cpp | build
	$(CXX) $(CXXFLAGS) src/parse.cpp -o build/parse.o -I src

build/codegen.o: $(HEADER) src/codegen.cpp | build
	$(CXX) $(CXXFLAGS) src/codegen.cpp -o build/codegen.o -I src

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

.PHONY: clean
clean:
	@ echo "[clean] colgm" && if [ -e colgm ]; then rm colgm; fi
	@ echo "[clean] colgm.exe" && if [ -e colgm.exe ]; then rm colgm.exe; fi
	@ rm $(OBJECT)

.PHONY: test
test: colgm
	./colgm ../test/example.colgm -a
	./colgm ../test/operator.colgm -a
	./colgm ../test/use_stmt.colgm -a