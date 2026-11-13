#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "Token.h"
#include "AST.h"

// Error sintáctico con su ubicación.
struct ErrorSintactico {
    int linea;
    int columna;
    std::string encontrado;   // lexema del token donde se detectó el error ("<fin de archivo>" si no hay más tokens)
    std::string mensaje;      // qué se esperaba y qué se encontró

    ErrorSintactico(int linea_, int columna_, std::string encontrado_, std::string mensaje_)
        : linea(linea_), columna(columna_), encontrado(std::move(encontrado_)), mensaje(std::move(mensaje_)) {}
};

// Aviso (no es error): construcción válida del lenguaje cuyo análisis corresponde
// a una entrega posterior (if / while / for / scanf / println). Se omite, no se verifica.
struct AvisoSintactico {
    int linea;
    int columna;
    std::string mensaje;

    AvisoSintactico(int linea_, int columna_, std::string mensaje_)
        : linea(linea_), columna(columna_), mensaje(std::move(mensaje_)) {}
};

// Analizador sintáctico de LP: DESCENSO RECURSIVO (predictivo, LL(1) con un caso de LL(2)).
//
// Trabaja directamente con la lista de tokens que produce el Lexer (integración léxico + sintáctico).
// Cada no terminal de la gramática (ver docs/GRAMATICA_LP.md) es un método de esta clase.
//
// ALCANCE DEL ENTREGABLE 1:
//   * estructura del programa  : Programa, Funcion, Parametros, Bloque
//   * declaraciones            : variables, arreglos, inicialización, varias por línea
//   * asignaciones             : variable = expresión ;  /  arreglo[expr] = expresión ;
//   * expresiones completas    : aritméticas, comparación, lógicas, unarias, paréntesis, llamadas
//   * return
// ENTREGABLE 2 (pendiente): if / else / while / for / scanf / println.
//   Por ahora se reconocen y se OMITEN con un aviso (ver sentenciaPendiente()).
//
// Recuperación de errores (modo pánico):
//   * falta de ';' -> se informa y se continúa como si estuviera (si el siguiente token empieza otra sentencia)
//   * otros errores -> se lanza una excepción interna, se salta hasta ';' / '}' / inicio de sentencia y se sigue.
//   Así se reportan VARIOS errores en una sola corrida sin errores en cascada.
class Parser {
public:
    // 'tokensDelLexer' es la salida de Lexer::analizar(). Los comentarios (COMENT) se descartan aquí.
    explicit Parser(const std::vector<Token>& tokensDelLexer);

    // Analiza el programa completo. Siempre devuelve un árbol (posiblemente parcial si hubo errores).
    NodoPtr analizar();

    const std::vector<ErrorSintactico>& getErrores() const { return errores; }
    const std::vector<AvisoSintactico>& getAvisos() const { return avisos; }
    bool exitoso() const { return errores.empty(); }

private:
    // Excepción interna para saltar al punto de sincronización (nunca sale de la clase).
    struct Abortar {};

    std::vector<Token> toks;
    Token fin;                    // token FIN_ARCHIVO ficticio, ubicado justo después del último token
    size_t pos = 0;
    bool tieneMain = false;
    size_t ultimoErrorPos = static_cast<size_t>(-1);   // evita reportar dos errores en el mismo token

    std::vector<ErrorSintactico> errores;
    std::vector<AvisoSintactico> avisos;

    // --- Manejo del flujo de tokens ---
    const Token& actual() const;
    const Token& mirar(size_t desplazamiento) const;   // lookahead sin consumir
    const Token& anterior() const;
    bool es(TokenType t) const { return actual().tipo == t; }
    Token avanzar();                                   // consume y devuelve el token actual
    bool consumirSi(TokenType t);

    static bool esTipoDato(TokenType t);               // int float char boolean
    static bool esTipoRetorno(TokenType t);            // tipos de dato + void
    static bool inicioDeSentencia(TokenType t);
    bool esInicioFuncion() const;                      // tipo (ID|main) '('   -> LL(2)

    // --- Reporte de errores ---
    static std::string describir(const Token& t);      // "el identificador 'x'", "fin de archivo", ...
    static std::string nombreEsperado(TokenType t);    // "';'", "un identificador", ...
    void registrarEn(int linea, int columna, const std::string& encontrado,
                     const std::string& mensaje, bool permitirDuplicado = false);
    [[noreturn]] void fallar(const std::string& mensaje);                          // error en el token actual + salto
    [[noreturn]] void fallarEn(const Token& donde, const std::string& mensaje);    // error en otro token + salto
    Token esperar(TokenType t, const std::string& contexto);
    void esperarPuntoComa(const std::string& contexto);

    // --- Recuperación ---
    void sincronizarSentencia();
    void sincronizarFuncion();

    // --- No terminales: estructura del programa ---
    NodoPtr programa();
    NodoPtr funcion();
    NodoPtr parametro();
    NodoPtr bloque();
    NodoPtr sentencia();

    // --- No terminales: sentencias del Entregable 1 ---
    NodoPtr declaracion();
    NodoPtr declaracionVariable();
    NodoPtr asignacionOLlamada();
    NodoPtr retorno();

    // --- Entregable 2: se omiten con aviso ---
    NodoPtr sentenciaPendiente();
    void omitirControl();
    void omitirSentencia();
    void omitirAgrupado(TokenType abre, TokenType cierra);
    void omitirHastaPuntoComa();

    // --- No terminales: expresiones (de menor a mayor precedencia) ---
    NodoPtr expresion();      // = exprOr
    NodoPtr exprOr();         // ||
    NodoPtr exprAnd();        // &&
    NodoPtr exprComp();       // COMP  (> >= < <= != ==)
    NodoPtr exprSuma();       // + -
    NodoPtr exprMult();       // * / %
    NodoPtr exprUnaria();     // ! -
    NodoPtr primario();       // número, texto, ID, ID[expr], ID(args), (expr)
    NodoPtr variable();       // ID  |  ID '[' expresion ']'
    NodoPtr llamada();        // ID '(' [expresion {',' expresion}] ')'
    NodoPtr nivelBinario(NodoPtr (Parser::*operando)(), bool (*esOperador)(TokenType));

    static NodoPtr nodoID(const Token& t);
};

#endif // PARSER_H
