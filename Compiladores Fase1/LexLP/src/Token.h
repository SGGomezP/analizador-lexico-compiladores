#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// Enum con TODOS los tipos de token contemplados en la especificación léxica
// de LP (secciones 4.1 a 4.4 del documento del proyecto).
//
// IMPORTANTE - Fase 1 (Lectura + NUM_INT + NUM_DEC):
//   En esta primera entrega el Lexer únicamente reconoce NUM_INT y NUM_DEC
//   (además de ERROR_LEXICO y FIN_ARCHIVO, que son necesarios para que el
//   analizador funcione de punta a punta). El resto de los valores del enum
//   ya están declarados para que las próximas fases (identificadores,
//   palabras reservadas, texto, operadores, símbolos especiales y
//   comentarios) solo requieran extender Lexer::siguienteToken() sin tener
//   que modificar la interfaz pública ni el resto del proyecto.
enum class TokenType {
    // --- Fase 1 (implementados) ---
    NUM_INT,        // D+                -> secuencia de dígitos
    NUM_DEC,        // D+\.D+            -> parte entera . parte decimal

    // --- Fase 2 (pendiente) ---
    ID,             // L(L|D)*
    TEXTO,          // ".*"
    // Palabras reservadas
    INT, FLOAT, CHAR, BOOLEAN, VOID,
    IF, ELSE, FOR, WHILE, SCANF, PRINTLN, MAIN, RETURN,

    // --- Fase 3 (pendiente) ---
    // Operadores
    ASIGNACION,     // =
    SUMA, RESTA, MULT, DIV, MODULO,        // + - * %
    AND, OR, NOT,                          // && || !
    COMPARACION,                           // > >= < <= != ==
    // Símbolos especiales
    PAR_IZQ, PAR_DER,                      // ( )
    CORCHETE_IZQ, CORCHETE_DER,            // [ ]
    LLAVE_IZQ, LLAVE_DER,                  // { }
    COMA, PUNTO_COMA,                      // , ;

    // --- Utilitarios (implementados desde la fase 1) ---
    ERROR_LEXICO,
    FIN_ARCHIVO
};

// Devuelve el nombre "legible" del tipo de token, tal como se usa
// en la lista de tokens (p. ej. "<NUM_INT>", "<ID, 0>").
std::string tokenTypeToString(TokenType tipo);

struct Token {
    TokenType tipo;
    int atributo;       // posición en la tabla de símbolos (solo para ID); -1 si no aplica
    std::string lexema;  // texto exacto reconocido en el fuente (útil para reportes/depuración)
    int linea;
    int columna;

    Token(TokenType tipo_, std::string lexema_, int linea_, int columna_, int atributo_ = -1)
        : tipo(tipo_), atributo(atributo_), lexema(std::move(lexema_)), linea(linea_), columna(columna_) {}

    // Representación en el formato de la lista de tokens del documento,
    // p. ej. "<NUM_INT>" o "<ID, 0>"
    std::string toString() const;
};

#endif // TOKEN_H
