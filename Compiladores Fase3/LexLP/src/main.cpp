#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
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

// Crea la carpeta de salida si no existe (para no perder las salidas).
static void asegurarCarpeta(const std::string& carpeta) {
#ifdef LEXLP_TIENE_FILESYSTEM
    std::error_code ec;
    std::filesystem::create_directories(carpeta, ec);
#else
    (void)carpeta;
#endif
}

static std::string textoError(const ErrorLexico& e) {
    return "Linea " + std::to_string(e.linea) + ", Columna " + std::to_string(e.columna) +
           ": " + e.motivo + " '" + e.lexema + "'";
}

// Secuencia compacta de tokens, en el formato del documento del proyecto:
// una línea del código fuente por línea de salida.
static std::string secuenciaTokens(const std::vector<Token>& tokens) {
    std::ostringstream out;
    int lineaActual = -1;
    for (const Token& t : tokens) {
        if (t.linea != lineaActual) {
            if (lineaActual != -1) out << "\n";
            lineaActual = t.linea;
        } else {
            out << " ";
        }
        out << t.toString();
    }
    if (lineaActual != -1) out << "\n";
    return out.str();
}

static std::string tablaTexto(const SymbolTable& tabla) {
    std::ostringstream out;
    out << "Posicion\tIdentificador\n";
    for (size_t i = 0; i < tabla.tamano(); ++i) {
        out << i << "\t\t" << tabla.getIdentificadores()[i] << "\n";
    }
    return out.str();
}

// Reporte compacto y DETERMINISTA (no incluye rutas ni fechas): es lo que se
// compara contra tests/esperado/ en las pruebas automáticas.
static std::string resultadoCompacto(const std::vector<Token>& tokens,
                                     const SymbolTable& tabla,
                                     const std::vector<ErrorLexico>& errores) {
    std::ostringstream out;
    out << "--- Tokens (" << tokens.size() << ") ---\n";
    out << secuenciaTokens(tokens);
    out << "--- Tabla de simbolos (" << tabla.tamano() << ") ---\n";
    for (size_t i = 0; i < tabla.tamano(); ++i) {
        out << i << " " << tabla.getIdentificadores()[i] << "\n";
    }
    out << "--- Errores lexicos (" << errores.size() << ") ---\n";
    for (const ErrorLexico& e : errores) {
        out << textoError(e) << "\n";
    }
    return out.str();
}

static void escribirArchivo(const std::string& ruta, const std::string& contenido) {
    std::ofstream f(ruta, std::ios::binary);
    if (f.is_open()) f << contenido;
}

static void mostrarUso(const char* exe) {
    std::cerr << "Uso: " << exe << " <archivo.lp> [--salida <carpeta>] [--sin-comentarios] [--silencioso]\n"
              << "  --salida <carpeta>   carpeta donde se escriben los resultados (por defecto: output)\n"
              << "  --sin-comentarios    reconoce los comentarios pero no los incluye en la lista de tokens\n"
              << "  --silencioso         no imprime el detalle por consola (solo genera los archivos)\n";
}

int main(int argc, char* argv[]) {
    std::string rutaEntrada = "tests/validas/prueba_programa.lp";
    std::string carpetaSalida = "output";
    bool incluirComentarios = true;
    bool silencioso = false;
    bool entradaDada = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--sin-comentarios") {
            incluirComentarios = false;
        } else if (a == "--silencioso") {
            silencioso = true;
        } else if (a == "--salida" && i + 1 < argc) {
            carpetaSalida = argv[++i];
        } else if (a == "-h" || a == "--help") {
            mostrarUso(argv[0]);
            return 0;
        } else if (!a.empty() && a[0] == '-' && a.size() > 1) {
            std::cerr << "Opcion desconocida: " << a << "\n";
            mostrarUso(argv[0]);
            return 1;
        } else {
            rutaEntrada = a;
            entradaDada = true;
        }
    }
    (void)entradaDada;

    std::string codigoFuente;
    if (!leerArchivo(rutaEntrada, codigoFuente)) {
        std::cerr << "Error: no se pudo abrir el archivo '" << rutaEntrada << "'\n";
        mostrarUso(argv[0]);
        return 1;
    }

    Lexer lexer(codigoFuente);
    lexer.setIncluirComentarios(incluirComentarios);
    std::vector<Token> tokens = lexer.analizar();
    const std::vector<ErrorLexico>& errores = lexer.getErrores();
    const SymbolTable& tabla = lexer.getTablaSimbolos();

    // ---- Salida por consola ----
    if (!silencioso) {
        std::cout << "===== Analizador Lexico LP - Fase 3 (tokens completos + integracion) =====\n";
        std::cout << "Archivo analizado: " << rutaEntrada << "\n\n";

        std::cout << "--- Lista de tokens (" << tokens.size() << ") ---\n";
        for (const Token& t : tokens) {
            std::cout << t.toString()
                      << "  [" << categoriaToken(t.tipo) << "]"
                      << "  lexema=\"" << t.lexema << "\""
                      << "  linea=" << t.linea
                      << "  columna=" << t.columna << "\n";
        }

        std::cout << "\n--- Secuencia de tokens (formato del documento) ---\n";
        std::cout << secuenciaTokens(tokens);

        std::cout << "\n--- Tabla de simbolos (" << tabla.tamano() << ") ---\n";
        if (tabla.tamano() == 0) {
            std::cout << "(vacia)\n";
        } else {
            std::cout << tablaTexto(tabla);
        }

        std::cout << "\n--- Errores lexicos (" << errores.size() << ") ---\n";
        if (errores.empty()) {
            std::cout << "(sin errores)\n";
        } else {
            for (const ErrorLexico& e : errores) {
                std::cout << textoError(e) << "\n";
            }
        }

        // Resumen: cuántos tokens hay de cada categoría.
        std::map<std::string, int> porCategoria;
        for (const Token& t : tokens) porCategoria[categoriaToken(t.tipo)]++;
        std::cout << "\n--- Resumen por categoria ---\n";
        for (const auto& par : porCategoria) {
            std::cout << par.first << ": " << par.second << "\n";
        }
    }

    // ---- Salida a archivos ----
    asegurarCarpeta(carpetaSalida);
    const std::string base = carpetaSalida + "/";

    {
        std::ostringstream out;
        for (const Token& t : tokens) {
            out << t.toString()
                << "\t" << categoriaToken(t.tipo)
                << "\tlexema=" << t.lexema
                << "\tlinea=" << t.linea
                << "\tcolumna=" << t.columna << "\n";
        }
        escribirArchivo(base + "tokens.txt", out.str());
    }
    escribirArchivo(base + "secuencia_tokens.txt", secuenciaTokens(tokens));
    escribirArchivo(base + "tabla_simbolos.txt", tablaTexto(tabla));
    {
        std::ostringstream out;
        for (const ErrorLexico& e : errores) out << textoError(e) << "\n";
        escribirArchivo(base + "errores.txt", out.str());
    }
    escribirArchivo(base + "resultado.txt", resultadoCompacto(tokens, tabla, errores));

    if (!silencioso) {
        std::cout << "\nSalidas generadas en " << base
                  << ": tokens.txt, secuencia_tokens.txt, tabla_simbolos.txt, errores.txt, resultado.txt\n";
    }

    return 0;
}
