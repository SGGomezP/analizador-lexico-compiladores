#include "Lexer.h"

Lexer::Lexer(const std::string& codigoFuente)
    : fuente(codigoFuente), pos(0), linea(1), columna(1) {}

// ---------------------------------------------------------------------------
// Utilidades de bajo nivel
// ---------------------------------------------------------------------------

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
    } else if ((static_cast<unsigned char>(c) & 0xC0) == 0x80) {
        // Byte de continuación UTF-8 (p. ej. la segunda mitad de 'ñ'):
        // no cuenta como una columna nueva, así la columna refleja caracteres
        // y no bytes.
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

bool Lexer::esDigito(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::esLetra(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

const std::unordered_map<std::string, TokenType>& Lexer::palabrasReservadas() {
    static const std::unordered_map<std::string, TokenType> tabla = {
        {"int",     TokenType::INT},
        {"float",   TokenType::FLOAT},
        {"char",    TokenType::CHAR},
        {"boolean", TokenType::BOOLEAN},
        {"void",    TokenType::VOID},
        {"if",      TokenType::IF},
        {"else",    TokenType::ELSE},
        {"for",     TokenType::FOR},
        {"while",   TokenType::WHILE},
        {"scanf",   TokenType::SCANF},
        {"println", TokenType::PRINTLN},
        {"main",    TokenType::MAIN},
        {"return",  TokenType::RETURN}
    };
    return tabla;
}

Token Lexer::registrarError(int lineaError, int columnaError,
                            const std::string& lexema, const std::string& motivo) {
    errores.emplace_back(lineaError, columnaError, lexema, motivo);
    return Token(TokenType::ERROR_LEXICO, lexema, lineaError, columnaError);
}

// ---------------------------------------------------------------------------
// Fase 1: números
// ---------------------------------------------------------------------------

Token Lexer::reconocerNumero() {
    int lineaInicio = linea;
    int columnaInicio = columna;
    std::string lexema;

    // Parte entera: D+
    while (!finDeArchivo() && esDigito(caracterActual())) {
        lexema += avanzar();
    }

    // ¿Es NUM_DEC? Requiere '.' seguido de al menos un dígito: D+\.D+
    if (!finDeArchivo() && caracterActual() == '.' && esDigito(verSiguiente())) {
        lexema += avanzar(); // consume '.'
        while (!finDeArchivo() && esDigito(caracterActual())) {
            lexema += avanzar();
        }
        return Token(TokenType::NUM_DEC, lexema, lineaInicio, columnaInicio);
    }

    // Si no hay '.' seguido de dígito, es simplemente un NUM_INT.
    // (Un '.' suelto se deja sin consumir; será reportado como error léxico
    // en la siguiente llamada a siguienteToken().)
    return Token(TokenType::NUM_INT, lexema, lineaInicio, columnaInicio);
}

// ---------------------------------------------------------------------------
// Fase 2: identificadores y palabras reservadas
// ---------------------------------------------------------------------------

Token Lexer::reconocerIdentificador() {
    int lineaInicio = linea;
    int columnaInicio = columna;
    std::string lexema;

    // L(L|D)*  -> se toma la coincidencia MÁS LARGA posible ("promedio2" es
    // un solo lexema, no "promedio" + "2").
    lexema += avanzar(); // primera letra (ya verificada por el llamador)
    while (!finDeArchivo() && (esLetra(caracterActual()) || esDigito(caracterActual()))) {
        lexema += avanzar();
    }

    // Una vez leído el lexema completo se decide si es palabra reservada o ID.
    // Las palabras reservadas NO entran a la tabla de símbolos.
    const auto& reservadas = palabrasReservadas();
    auto it = reservadas.find(lexema);
    if (it != reservadas.end()) {
        return Token(it->second, lexema, lineaInicio, columnaInicio);
    }

    // Identificador: se inserta una sola vez en la tabla (o se recupera la
    // posición existente) y el token guarda esa posición como atributo.
    int posicionTabla = tabla.insertar(lexema);
    return Token(TokenType::ID, lexema, lineaInicio, columnaInicio, posicionTabla);
}

// ---------------------------------------------------------------------------
// Fase 2: constantes de texto
// ---------------------------------------------------------------------------

Token Lexer::reconocerTexto() {
    int lineaInicio = linea;
    int columnaInicio = columna;
    std::string lexema;

    lexema += avanzar(); // comilla de apertura '"'

    // Se lee hasta la PRIMERA comilla de cierre. El '.' de la expresión
    // regular ".*" no incluye el salto de línea, por lo que un texto no puede
    // extenderse a la línea siguiente.
    while (!finDeArchivo() && caracterActual() != '"' &&
           caracterActual() != '\n' && caracterActual() != '\r') {
        lexema += avanzar();
    }

    if (!finDeArchivo() && caracterActual() == '"') {
        lexema += avanzar(); // comilla de cierre
        return Token(TokenType::TEXTO, lexema, lineaInicio, columnaInicio);
    }

    // Llegó fin de línea o fin de archivo sin cerrar la comilla: error léxico.
    // Se reporta UNA sola vez desde la comilla hasta el final de la línea, para
    // no generar errores en cascada con el contenido del texto.
    return registrarError(lineaInicio, columnaInicio, lexema, "texto sin cerrar");
}

// ---------------------------------------------------------------------------
// Dispatcher principal
// ---------------------------------------------------------------------------

Token Lexer::siguienteToken() {
    saltarEspacios();

    if (finDeArchivo()) {
        return Token(TokenType::FIN_ARCHIVO, "", linea, columna);
    }

    char c = caracterActual();

    if (esDigito(c)) {
        return reconocerNumero();
    }

    if (esLetra(c)) {
        return reconocerIdentificador();
    }

    if (c == '"') {
        return reconocerTexto();
    }

    // Fase 2: todavía no se reconocen operadores, símbolos especiales ni
    // comentarios (Fase 3). Cualquier otro carácter es, por ahora, un error
    // léxico. Se consume UN carácter para poder continuar el análisis
    // (recuperación de errores). Si es un carácter UTF-8 multibyte (p. ej. 'ñ',
    // 'á') se consume completo para reportarlo como un solo error.
    int lineaError = linea;
    int columnaError = columna;
    std::string lexemaError(1, avanzar());
    while (!finDeArchivo() &&
           (static_cast<unsigned char>(caracterActual()) & 0xC0) == 0x80) {
        lexemaError += avanzar();
    }

    return registrarError(lineaError, columnaError, lexemaError, "caracter no reconocido");
}

std::vector<Token> Lexer::analizar() {
    std::vector<Token> tokens;

    // Se reinicia todo el estado para que analizar() sea repetible.
    pos = 0;
    linea = 1;
    columna = 1;
    errores.clear();
    tabla.limpiar();

    while (true) {
        Token t = siguienteToken();
        if (t.tipo == TokenType::FIN_ARCHIVO) {
            break;
        }
        // Los errores se registran en 'errores' (dentro de siguienteToken) y
        // NO se agregan a la lista final de tokens.
        if (t.tipo != TokenType::ERROR_LEXICO) {
            tokens.push_back(t);
        }
    }

    return tokens;
}
