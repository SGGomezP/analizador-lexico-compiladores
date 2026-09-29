#include "Lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& codigoFuente)
    : fuente(codigoFuente), pos(0), linea(1), columna(1) {}

bool Lexer::finDeArchivo() const {
    return pos >= fuente.size();
}

char Lexer::caracterActual() const {
    if (finDeArchivo()) return '\0';
    return fuente[pos];
}

char Lexer::verSiguiente(int offset) const {
    size_t idx = pos + static_cast<size_t>(offset);
    if (idx >= fuente.size()) return '\0';
    return fuente[idx];
}

char Lexer::avanzar() {
    char c = fuente[pos];
    pos++;
    if (c == '\n') {
        linea++;
        columna = 1;
    } else {
        columna++;
    }
    return c;
}

void Lexer::saltarEspacios() {
    while (!finDeArchivo()) {
        char c = caracterActual();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            avanzar();
        } else {
            break;
        }
    }
}

Token Lexer::reconocerNumero() {
    int lineaInicio = linea;
    int columnaInicio = columna;
    std::string lexema;

    // Parte entera: D+
    while (!finDeArchivo() && std::isdigit(static_cast<unsigned char>(caracterActual()))) {
        lexema += avanzar();
    }

    // ¿Es NUM_DEC? Requiere '.' seguido de al menos un dígito: D+\.D+
    if (!finDeArchivo() && caracterActual() == '.' &&
        std::isdigit(static_cast<unsigned char>(verSiguiente()))) {

        lexema += avanzar(); // consume '.'
        while (!finDeArchivo() && std::isdigit(static_cast<unsigned char>(caracterActual()))) {
            lexema += avanzar();
        }
        return Token(TokenType::NUM_DEC, lexema, lineaInicio, columnaInicio);
    }

    // Si no hay '.' seguido de dígito, es simplemente un NUM_INT.
    // (Un '.' suelto que no cumpla D+\.D+ se deja sin consumir; el propio
    // carácter '.' será reportado como error léxico en la siguiente llamada
    // a siguienteToken(), ya que no forma parte de ningún token de esta fase.)
    return Token(TokenType::NUM_INT, lexema, lineaInicio, columnaInicio);
}

Token Lexer::siguienteToken() {
    saltarEspacios();

    if (finDeArchivo()) {
        return Token(TokenType::FIN_ARCHIVO, "", linea, columna);
    }

    char c = caracterActual();

    if (std::isdigit(static_cast<unsigned char>(c))) {
        return reconocerNumero();
    }

    // Fase 1: todavía no se reconocen letras, operadores ni símbolos
    // especiales. Cualquier carácter que llegue hasta aquí es, por ahora,
    // un error léxico. Se reporta y se consume un carácter para poder
    // continuar el análisis (recuperación de errores carácter a carácter).
    int lineaError = linea;
    int columnaError = columna;
    std::string lexemaError(1, avanzar());

    errores.emplace_back(lineaError, columnaError, lexemaError);
    return Token(TokenType::ERROR_LEXICO, lexemaError, lineaError, columnaError);
}

std::vector<Token> Lexer::analizar() {
    std::vector<Token> tokens;
    errores.clear();

    while (true) {
        Token t = siguienteToken();
        if (t.tipo == TokenType::FIN_ARCHIVO) {
            break;
        }
        // Los tokens de error se registran en 'errores' (dentro de
        // siguienteToken) y NO se agregan a la lista final de tokens,
        // tal como indica la sección 4.4 del documento para los comentarios
        // (se reconocen pero no forman parte del flujo principal de tokens).
        if (t.tipo != TokenType::ERROR_LEXICO) {
            tokens.push_back(t);
        }
    }

    return tokens;
}
