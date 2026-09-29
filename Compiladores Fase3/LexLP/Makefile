CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC = src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp
BIN = lexlp

all: $(BIN)

$(BIN): $(SRC) $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN) tests/validas/prueba_programa.lp

test: $(BIN)
	sh correr_pruebas.sh

clean:
	rm -f $(BIN)
	rm -rf output/*

.PHONY: all run test clean
