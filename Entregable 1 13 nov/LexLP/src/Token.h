#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <utility>

// Enum con TODOS los tipos de token de la especificación léxica de LP
// (lista de expresiones regulares del curso, "Aula 5").
//
// Estado por fases:
//   Fase 1: NUM_INT y NUM_DEC (+ ERROR_LEXICO y FIN_ARCHIVO, utilitarios).
//   Fase 2: ID, TEXTO y las 13 palabras reservadas (+ tabla de símbolos).
//   Fase 3: operadores, símbolos especiales y comentarios  -> ¡COMPLETO!
enum class TokenType {
    // --- Fase 1 ---
    NUM_INT,        // D+                -> secuencia de dígitos
    NUM_DEC,        // D+\.D+            -> parte entera . parte decimal

    // --- Fase 2 ---
    ID,             // L(L|D)*
    TEXTO,          // ".*"
    // Palabras reservadas
    INT, FLOAT, CHAR, BOOLEAN, VOID,
    IF, ELSE, FOR, WHILE, SCANF, PRINTLN, MAIN, RETURN,

    // --- Fase 3 ---
    // Operadores
    ASIGNACION,     // =
    SUMA, RESTA, MULT, DIV, MODULO,        // + - * / %
    AND, OR, NOT,                          // && || !
    COMPARACION,                           // COMP = > | >= | < | <= | != | ==
    // Símbolos especiales
    PAR_IZQ, PAR_DER,                      // ( )
    CORCHETE_IZQ, CORCHETE_DER,            // [ ]
    LLAVE_IZQ, LLAVE_DER,                  // { }
    COMA, PUNTO_COMA,                      // , ;
    // Comentarios
    COMENT,                                // //.*\n

    // --- Utilitarios ---
    ERROR_LEXICO,
    FIN_ARCHIVO
};

// Nombre del token según la 2.ª columna de la lista de expresiones regulares
// (p. ej. "NUM_INT", "ID", "+", "&&", "COMP", "(", "COMENT").
std::string tokenTypeToString(TokenType tipo);

// Categoría del token según la 3.ª columna de la lista
// (p. ej. "Operador aritmetico", "Palabra reservada", "Simbolo especial").
std::string categoriaToken(TokenType tipo);

struct Token {
    TokenType tipo;
    int atributo;       // posición en la tabla de símbolos (solo para ID); -1 si no aplica
    std::string lexema;  // texto exacto reconocido en el fuente
    int linea;
    int columna;

    Token(TokenType tipo_, std::string lexema_, int linea_, int columna_, int atributo_ = -1)
        : tipo(tipo_), atributo(atributo_), lexema(std::move(lexema_)), linea(linea_), columna(columna_) {}

    // Representación en el formato de la lista de tokens:
    //   <NUM_INT>   <ID, 0>   <+>   <&&>   <COMP, >=>   <(>   <COMENT>
    // Atributos: ID -> posición en la tabla de símbolos;
    //            COMP -> el operador de comparación concreto (>, >=, <, <=, !=, ==).
    std::string toString() const;
};

#endif // TOKEN_H
