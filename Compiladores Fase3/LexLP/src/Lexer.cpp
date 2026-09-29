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

    bool esDecimal = false;

    // ¿Es NUM_DEC? Requiere '.' seguido de al menos un dígito: D+\.D+
    if (!finDeArchivo() && caracterActual() == '.' && esDigito(verSiguiente())) {
        esDecimal = true;
        lexema += avanzar(); // consume '.'
        while (!finDeArchivo() && esDigito(caracterActual())) {
            lexema += avanzar();
        }
    }

    // NUMERO MAL FORMADO: si después del número viene otro '.', el lexema
    // completo (dígitos y puntos) NO es un número válido. Ejemplos:
    //   12.3.4   1.2.3.4   5..3   10.
    // Se consume TODO el bloque de dígitos y puntos y se reporta UN solo error
    // (en vez de aceptar "12.3" como NUM_DEC y dejar ".4" suelto).
    if (!finDeArchivo() && caracterActual() == '.') {
        while (!finDeArchivo() && (esDigito(caracterActual()) || caracterActual() == '.')) {
            lexema += avanzar();
        }
        return registrarError(lineaInicio, columnaInicio, lexema, motivoNumeroMalFormado(lexema));
    }

    return Token(esDecimal ? TokenType::NUM_DEC : TokenType::NUM_INT,
                 lexema, lineaInicio, columnaInicio);
}

std::string Lexer::motivoNumeroMalFormado(const std::string& lexema) {
    int puntos = 0;
    for (char ch : lexema) if (ch == '.') puntos++;
    if (lexema.empty() || lexema[0] == '.') {
        return "numero decimal invalido (falta la parte entera)";
    }
    if (puntos == 1 && lexema.back() == '.') {
        return "numero decimal invalido (falta la parte decimal despues del punto)";
    }
    return "numero decimal invalido (mas de un punto decimal)";
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
    return registrarError(lineaInicio, columnaInicio, lexema, "texto sin cerrar (falta la comilla de cierre)");
}

// ---------------------------------------------------------------------------
// Fase 3: comentarios, operadores y símbolos especiales
// ---------------------------------------------------------------------------

// Consume 'longitud' caracteres (todos de una línea) y devuelve el token.
Token Lexer::crearToken(TokenType tipo, int longitud) {
    int lineaInicio = linea;
    int columnaInicio = columna;
    std::string lexema;
    for (int i = 0; i < longitud; ++i) {
        lexema += avanzar();
    }
    return Token(tipo, lexema, lineaInicio, columnaInicio);
}

Token Lexer::reconocerComentario() {
    int lineaInicio = linea;
    int columnaInicio = columna;
    std::string lexema;

    // COMENT = //.*\n  -> desde "//" hasta el fin de la línea.
    // El '\n' final forma parte del token en la expresión regular; aquí se
    // consume pero NO se guarda en el lexema (ni tampoco un '\r' de Windows).
    // Si el archivo termina sin '\n' el comentario también es válido.
    while (!finDeArchivo() && caracterActual() != '\n') {
        lexema += avanzar();
    }
    if (!finDeArchivo()) {
        avanzar(); // consume el '\n'
    }
    if (!lexema.empty() && lexema.back() == '\r') {
        lexema.pop_back();
    }
    return Token(TokenType::COMENT, lexema, lineaInicio, columnaInicio);
}

Token Lexer::reconocerOperador() {
    char c = caracterActual();
    char s = verSiguiente();

    switch (c) {
        // Operadores que pueden ser de 1 o 2 caracteres: se aplica la
        // coincidencia más larga con UN carácter de lookahead.
        case '=':
            if (s == '=') return crearToken(TokenType::COMPARACION, 2);   // ==
            return crearToken(TokenType::ASIGNACION, 1);                  // =
        case '!':
            if (s == '=') return crearToken(TokenType::COMPARACION, 2);   // !=
            return crearToken(TokenType::NOT, 1);                         // !
        case '<':
            if (s == '=') return crearToken(TokenType::COMPARACION, 2);   // <=
            return crearToken(TokenType::COMPARACION, 1);                 // <
        case '>':
            if (s == '=') return crearToken(TokenType::COMPARACION, 2);   // >=
            return crearToken(TokenType::COMPARACION, 1);                 // >

        // '&' y '|' solo son válidos duplicados (&& y ||).
        case '&':
            if (s == '&') return crearToken(TokenType::AND, 2);
            {
                int l = linea, col = columna;
                std::string lex(1, avanzar());
                return registrarError(l, col, lex, "operador incompleto (se esperaba '&&')");
            }
        case '|':
            if (s == '|') return crearToken(TokenType::OR, 2);
            {
                int l = linea, col = columna;
                std::string lex(1, avanzar());
                return registrarError(l, col, lex, "operador incompleto (se esperaba '||')");
            }

        // Operadores aritméticos (el '/' se distingue de "//" en siguienteToken()).
        case '+': return crearToken(TokenType::SUMA, 1);
        case '-': return crearToken(TokenType::RESTA, 1);
        case '*': return crearToken(TokenType::MULT, 1);
        case '/': return crearToken(TokenType::DIV, 1);
        case '%': return crearToken(TokenType::MODULO, 1);

        // Símbolos especiales.
        case '(': return crearToken(TokenType::PAR_IZQ, 1);
        case ')': return crearToken(TokenType::PAR_DER, 1);
        case '[': return crearToken(TokenType::CORCHETE_IZQ, 1);
        case ']': return crearToken(TokenType::CORCHETE_DER, 1);
        case '{': return crearToken(TokenType::LLAVE_IZQ, 1);
        case '}': return crearToken(TokenType::LLAVE_DER, 1);
        case ',': return crearToken(TokenType::COMA, 1);
        case ';': return crearToken(TokenType::PUNTO_COMA, 1);
        default:  break;
    }

    // No pertenece a ningún token de LP: error léxico. Se consume UN carácter
    // para poder continuar (recuperación de errores). Si es UTF-8 multibyte
    // (p. ej. 'ñ', 'á') se consume completo para reportarlo como un solo error.
    int lineaError = linea;
    int columnaError = columna;
    std::string lexemaError(1, avanzar());
    while (!finDeArchivo() &&
           (static_cast<unsigned char>(caracterActual()) & 0xC0) == 0x80) {
        lexemaError += avanzar();
    }
    return registrarError(lineaError, columnaError, lexemaError, "caracter no reconocido");
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

    // Número que empieza con '.' (p. ej. .5): no cumple D+\.D+ -> un solo error.
    if (c == '.' && esDigito(verSiguiente())) {
        int lineaError = linea;
        int columnaError = columna;
        std::string lex;
        while (!finDeArchivo() && (esDigito(caracterActual()) || caracterActual() == '.')) {
            lex += avanzar();
        }
        return registrarError(lineaError, columnaError, lex, motivoNumeroMalFormado(lex));
    }

    // Fase 3: comentario "//" (debe probarse ANTES que el operador '/').
    if (c == '/' && verSiguiente() == '/') {
        return reconocerComentario();
    }

    // Fase 3: operadores, símbolos especiales y, si no coincide nada,
    // error léxico por carácter no reconocido.
    return reconocerOperador();
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
        if (t.tipo == TokenType::ERROR_LEXICO) {
            continue;
        }
        if (t.tipo == TokenType::COMENT && !incluirComentarios) {
            continue;
        }
        tokens.push_back(t);
    }

    return tokens;
}
