CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC = src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp src/Parser.cpp src/AST.cpp
BIN = lexlp

all: $(BIN)

$(BIN): $(SRC) $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN) tests/sintactico/validas/programa_minimo.lp

test: $(BIN)
	sh correr_pruebas.sh
	sh correr_pruebas_sintacticas.sh

clean:
	rm -f $(BIN)
	rm -rf output/*

.PHONY: all run test clean
