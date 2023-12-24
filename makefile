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
	src/colgm.h

OBJECT = \
	build/err.o\
	build/lexer.o\
	build/misc.o\
	build/main.o


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
	$(CXX) $(CXXFLAGS) src/main.cpp -o build/main.o

build/misc.o: src/colgm.h src/misc.cpp | build
	$(CXX) $(CXXFLAGS) src/misc.cpp -o build/misc.o

build/err.o: src/colgm.h src/err.h src/err.cpp | build
	$(CXX) $(CXXFLAGS) src/err.cpp -o build/err.o

build/lexer.o: src/colgm.h src/err.h src/lexer.h src/lexer.cpp | build
	$(CXX) $(CXXFLAGS) src/lexer.cpp -o build/lexer.o

.PHONY: clean
clean:
	@ echo "[clean] colgm" && if [ -e colgm ]; then rm colgm; fi
	@ echo "[clean] colgm.exe" && if [ -e colgm.exe ]; then rm colgm.exe; fi
	@ rm $(OBJECT)
