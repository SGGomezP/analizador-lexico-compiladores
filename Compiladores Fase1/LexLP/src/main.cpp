#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Lexer.h"
#include "Token.h"

// Lee el contenido completo de un archivo de texto en un std::string.
// Devuelve false si el archivo no se pudo abrir.
static bool leerArchivo(const std::string& ruta, std::string& contenido) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        return false;
    }
    std::ostringstream buffer;
    buffer << archivo.rdbuf();
    contenido = buffer.str();
    return true;
}

int main(int argc, char* argv[]) {
    // Archivo de entrada: por parámetro, o tests/prueba_basica.lp por defecto.
    std::string rutaEntrada = (argc > 1) ? argv[1] : "tests/prueba_basica.lp";

    std::string codigoFuente;
    if (!leerArchivo(rutaEntrada, codigoFuente)) {
        std::cerr << "Error: no se pudo abrir el archivo '" << rutaEntrada << "'\n";
        std::cerr << "Uso: " << argv[0] << " <archivo.lp>\n";
        return 1;
    }

    Lexer lexer(codigoFuente);
    std::vector<Token> tokens = lexer.analizar();
    const std::vector<ErrorLexico>& errores = lexer.getErrores();

    // ---- Salida por consola ----
    std::cout << "===== Analizador Lexico LP - Fase 1 (Lectura + NUM_INT + NUM_DEC) =====\n";
    std::cout << "Archivo analizado: " << rutaEntrada << "\n\n";

    std::cout << "--- Lista de tokens (" << tokens.size() << ") ---\n";
    for (const Token& t : tokens) {
        std::cout << t.toString()
                   << "  lexema=\"" << t.lexema << "\""
                   << "  linea=" << t.linea
                   << "  columna=" << t.columna << "\n";
    }

    std::cout << "\n--- Errores lexicos (" << errores.size() << ") ---\n";
    if (errores.empty()) {
        std::cout << "(sin errores)\n";
    } else {
        for (const ErrorLexico& e : errores) {
            std::cout << "Linea " << e.linea << ", Columna " << e.columna
                       << ": caracter no reconocido '" << e.lexema << "'\n";
        }
    }

    // ---- Salida a archivos (output/) ----
    std::ofstream outTokens("output/tokens.txt");
    if (outTokens.is_open()) {
        for (const Token& t : tokens) {
            outTokens << t.toString()
                       << "\tlexema=" << t.lexema
                       << "\tlinea=" << t.linea
                       << "\tcolumna=" << t.columna << "\n";
        }
    }

    std::ofstream outErrores("output/errores.txt");
    if (outErrores.is_open()) {
        for (const ErrorLexico& e : errores) {
            outErrores << "Linea " << e.linea << ", Columna " << e.columna
                        << ": caracter no reconocido '" << e.lexema << "'\n";
        }
    }

    // La tabla de simbolos se incorpora en la Fase 2 (junto con ID).
    // Se deja el archivo generado (vacio) para respetar la estructura de
    // salidas obligatoria definida en el documento del proyecto.
    std::ofstream outTabla("output/tabla_simbolos.txt");
    if (outTabla.is_open()) {
        outTabla << "# La tabla de simbolos se implementa en la Fase 2 (identificadores).\n";
    }

    std::cout << "\nSalidas generadas en: output/tokens.txt, output/errores.txt, output/tabla_simbolos.txt\n";

    return 0;
}
