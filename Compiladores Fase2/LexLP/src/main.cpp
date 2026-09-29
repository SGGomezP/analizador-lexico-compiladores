#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Lexer.h"
#include "Token.h"
#include "SymbolTable.h"

#if defined(__has_include)
#  if __has_include(<filesystem>)
#    include <filesystem>
#    define LEXLP_TIENE_FILESYSTEM 1
#  endif
#endif

// Lee el contenido completo de un archivo de texto en un std::string.
// Devuelve false si el archivo no se pudo abrir.
static bool leerArchivo(const std::string& ruta, std::string& contenido) {
    std::ifstream archivo(ruta, std::ios::binary);
    if (!archivo.is_open()) {
        return false;
    }
    std::ostringstream buffer;
    buffer << archivo.rdbuf();
    contenido = buffer.str();

    // Los editores de Windows a veces guardan UTF-8 con BOM (EF BB BF) al
    // inicio. Se descarta para que no aparezca como un error léxico falso.
    if (contenido.size() >= 3 &&
        static_cast<unsigned char>(contenido[0]) == 0xEF &&
        static_cast<unsigned char>(contenido[1]) == 0xBB &&
        static_cast<unsigned char>(contenido[2]) == 0xBF) {
        contenido.erase(0, 3);
    }
    return true;
}

// Crea la carpeta output/ si no existe (para no perder las salidas).
static void asegurarCarpetaSalida() {
#ifdef LEXLP_TIENE_FILESYSTEM
    std::error_code ec;
    std::filesystem::create_directories("output", ec);
#endif
}

static std::string textoError(const ErrorLexico& e) {
    return "Linea " + std::to_string(e.linea) + ", Columna " + std::to_string(e.columna) +
           ": " + e.motivo + " '" + e.lexema + "'";
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
    const SymbolTable& tabla = lexer.getTablaSimbolos();

    // ---- Salida por consola ----
    std::cout << "===== Analizador Lexico LP - Fase 2 (ID + TEXTO + palabras reservadas + tabla de simbolos) =====\n";
    std::cout << "Archivo analizado: " << rutaEntrada << "\n\n";

    std::cout << "--- Lista de tokens (" << tokens.size() << ") ---\n";
    for (const Token& t : tokens) {
        std::cout << t.toString()
                  << "  lexema=\"" << t.lexema << "\""
                  << "  linea=" << t.linea
                  << "  columna=" << t.columna << "\n";
    }

    // Secuencia compacta, en el formato del documento (secciones 5.3 y 12),
    // una línea del código fuente por línea de salida.
    std::cout << "\n--- Secuencia de tokens (formato del documento) ---\n";
    {
        int lineaActual = -1;
        for (const Token& t : tokens) {
            if (t.linea != lineaActual) {
                if (lineaActual != -1) std::cout << "\n";
                lineaActual = t.linea;
            } else {
                std::cout << " ";
            }
            std::cout << t.toString();
        }
        if (lineaActual != -1) std::cout << "\n";
    }

    std::cout << "\n--- Tabla de simbolos (" << tabla.tamano() << ") ---\n";
    if (tabla.tamano() == 0) {
        std::cout << "(vacia)\n";
    } else {
        std::cout << "Posicion\tIdentificador\n";
        for (size_t i = 0; i < tabla.tamano(); ++i) {
            std::cout << i << "\t\t" << tabla.getIdentificadores()[i] << "\n";
        }
    }

    std::cout << "\n--- Errores lexicos (" << errores.size() << ") ---\n";
    if (errores.empty()) {
        std::cout << "(sin errores)\n";
    } else {
        for (const ErrorLexico& e : errores) {
            std::cout << textoError(e) << "\n";
        }
    }

    // ---- Salida a archivos (output/) ----
    asegurarCarpetaSalida();

    std::ofstream outTokens("output/tokens.txt");
    if (outTokens.is_open()) {
        for (const Token& t : tokens) {
            outTokens << t.toString()
                      << "\tlexema=" << t.lexema
                      << "\tlinea=" << t.linea
                      << "\tcolumna=" << t.columna << "\n";
        }
    }

    std::ofstream outTabla("output/tabla_simbolos.txt");
    if (outTabla.is_open()) {
        outTabla << "Posicion\tIdentificador\n";
        for (size_t i = 0; i < tabla.tamano(); ++i) {
            outTabla << i << "\t" << tabla.getIdentificadores()[i] << "\n";
        }
    }

    std::ofstream outErrores("output/errores.txt");
    if (outErrores.is_open()) {
        for (const ErrorLexico& e : errores) {
            outErrores << textoError(e) << "\n";
        }
    }

    std::cout << "\nSalidas generadas en: output/tokens.txt, output/tabla_simbolos.txt, output/errores.txt\n";

    return 0;
}
