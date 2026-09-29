#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "Token.h"
#include "SymbolTable.h"

// Estructura simple para reportar un error léxico con su ubicación.
struct ErrorLexico {
    int linea;
    int columna;
    std::string lexema;
    std::string motivo;   // p. ej. "caracter no reconocido", "texto sin cerrar"

    ErrorLexico(int linea_, int columna_, std::string lexema_, std::string motivo_)
        : linea(linea_), columna(columna_), lexema(std::move(lexema_)), motivo(std::move(motivo_)) {}
};

// Analizador léxico para LP.
//
// Fase 1: lectura carácter por carácter (con línea/columna) y
//         NUM_INT = D+ ,  NUM_DEC = D+\.D+
// Fase 2: ID = L(L|D)* , TEXTO = ".*" , 13 palabras reservadas y
//         tabla de símbolos (cada ID se inserta UNA sola vez: <ID, posicion>).
// Fase 3 (esta entrega): TODOS los tokens de la lista del curso.
//         Operadores:  =  + - * / %  &&  ||  !  y COMP = > >= < <= != ==
//         Símbolos:    ( ) [ ] { } , ;
//         Comentarios: //.*\n   (token COMENT)
//         Regla general: coincidencia MÁS LARGA (">=" es un token, no ">" + "=").
//         Un '&' o '|' solitario NO es token de LP: se reporta como error léxico.
class Lexer {
public:
    // Construye el lexer a partir del contenido completo del archivo fuente.
    explicit Lexer(const std::string& codigoFuente);

    // Recorre todo el código fuente y devuelve la lista secuencial de tokens
    // reconocidos (sin incluir FIN_ARCHIVO ni los errores). Los errores léxicos
    // se acumulan aparte (getErrores()) y la tabla de símbolos se llena a medida
    // que aparecen identificadores (getTablaSimbolos()).
    std::vector<Token> analizar();

    // Si es 'false', los comentarios se reconocen pero NO se agregan a la lista
    // de tokens (por defecto 'true': se emite <COMENT>, tal como pide la lista).
    void setIncluirComentarios(bool incluir) { incluirComentarios = incluir; }

    // Errores léxicos detectados durante la última llamada a analizar().
    const std::vector<ErrorLexico>& getErrores() const { return errores; }

    // Tabla de símbolos generada durante la última llamada a analizar().
    const SymbolTable& getTablaSimbolos() const { return tabla; }

private:
    std::string fuente;
    size_t pos;      // índice actual dentro de 'fuente'
    int linea;
    int columna;

    std::vector<ErrorLexico> errores;
    SymbolTable tabla;
    bool incluirComentarios = true;

    // --- Utilidades de bajo nivel sobre el flujo de caracteres ---
    bool finDeArchivo() const;
    char caracterActual() const;
    char verSiguiente(int offset = 1) const; // lookahead sin consumir
    char avanzar();                          // consume y devuelve el carácter actual, actualiza línea/columna

    void saltarEspacios();

    // Clases de caracteres de la especificación (independientes del locale).
    static bool esDigito(char c);   // D = [0-9]
    static bool esLetra(char c);    // L = [a-zA-Z_]

    // Palabras reservadas: lexema -> token. Es sensible a mayúsculas (int != Int).
    static const std::unordered_map<std::string, TokenType>& palabrasReservadas();

    // Reconocedores de token individuales. Cada uno asume que
    // caracterActual() ya fue verificado por el llamador.
    Token reconocerNumero();
    Token reconocerIdentificador();   // ID o palabra reservada
    Token reconocerTexto();           // TEXTO (o error si no se cierra)

    // Fase 3
    Token reconocerComentario();      // //.*\n   (asume que ve "//")
    Token reconocerOperador();        // operadores y símbolos especiales (o error de '&' / '|' solitarios)
    Token crearToken(TokenType tipo, int longitud); // consume 'longitud' caracteres y arma el token

    // Registra un error léxico (consumiendo 'lexema' ya leído) y devuelve su token de error.
    Token registrarError(int lineaError, int columnaError, const std::string& lexema, const std::string& motivo);

    // Obtiene el siguiente token del flujo (usado internamente por analizar()).
    Token siguienteToken();
};

#endif // LEXER_H
