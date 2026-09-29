#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <utility>

// Enum con TODOS los tipos de token contemplados en la especificación léxica
// de LP (secciones 4.1 a 4.4 del documento del proyecto).
//
// Estado por fases:
//   Fase 1: NUM_INT y NUM_DEC (+ ERROR_LEXICO y FIN_ARCHIVO, utilitarios).
//   Fase 2: ID, TEXTO y las 13 palabras reservadas (+ tabla de símbolos).
//   Fase 3: operadores, símbolos especiales y comentarios (todavía NO
//           implementados: el Lexer los reporta como error léxico).
//   Los valores de la Fase 3 ya están declarados para que solo haya que
//   extender Lexer::siguienteToken() sin tocar la interfaz del resto del proyecto.
enum class TokenType {
    // --- Fase 1 (implementados) ---
    NUM_INT,        // D+                -> secuencia de dígitos
    NUM_DEC,        // D+\.D+            -> parte entera . parte decimal

    // --- Fase 2 (implementados) ---
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
