#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Token.h"

// Estructura simple para reportar un error léxico con su ubicación.
struct ErrorLexico {
    int linea;
    int columna;
    std::string lexema;

    ErrorLexico(int linea_, int columna_, std::string lexema_)
        : linea(linea_), columna(columna_), lexema(std::move(lexema_)) {}
};

// Analizador léxico para LP.
//
// Fase 1: el lexer sabe LEER el archivo fuente carácter por carácter,
// llevando la cuenta de línea/columna, ignorar espacios en blanco, y
// reconocer las expresiones regulares:
//   NUM_INT = D+
//   NUM_DEC = D+\.D+          (con D = [0-9])
// Cualquier otro carácter todavía no reconocido (letras, símbolos,
// operadores, etc. -- que se implementarán en fases siguientes) se reporta
// como error léxico, carácter por carácter.
class Lexer {
public:
    // Construye el lexer a partir del contenido completo del archivo fuente.
    explicit Lexer(const std::string& codigoFuente);

    // Recorre todo el código fuente y devuelve la lista secuencial de tokens
    // reconocidos (sin incluir FIN_ARCHIVO). Los errores léxicos encontrados
    // se acumulan por separado y se pueden consultar con getErrores().
    std::vector<Token> analizar();

    // Errores léxicos detectados durante la última llamada a analizar().
    const std::vector<ErrorLexico>& getErrores() const { return errores; }

private:
    std::string fuente;
    size_t pos;      // índice actual dentro de 'fuente'
    int linea;
    int columna;

    std::vector<ErrorLexico> errores;

    // --- Utilidades de bajo nivel sobre el flujo de caracteres ---
    bool finDeArchivo() const;
    char caracterActual() const;
    char verSiguiente(int offset = 1) const; // lookahead sin consumir
    char avanzar();                          // consume y devuelve el carácter actual, actualiza línea/columna

    void saltarEspacios();

    // Reconocedores de token individuales. Cada uno asume que
    // caracterActual() ya fue verificado por el llamador.
    Token reconocerNumero();

    // Obtiene el siguiente token del flujo (usado internamente por analizar()).
    Token siguienteToken();
};

#endif // LEXER_H
