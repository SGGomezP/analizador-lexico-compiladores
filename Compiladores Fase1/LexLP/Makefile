CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
SRC = src/main.cpp src/Lexer.cpp src/Token.cpp
BIN = lexlp

all: $(BIN)

$(BIN): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(SRC)

run: $(BIN)
	./$(BIN) tests/prueba_basica.lp

clean:
	rm -f $(BIN)
	rm -f output/*.txt

.PHONY: all run clean
