#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iomanip>
#include "Lexer.h"
#include "Token.h"
#include "SymbolTable.h"
#include "Parser.h"
#include "AST.h"

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

// Tabla de errores léxicos (columnas alineadas): N°, línea, columna, lexema, descripción.
static std::string tablaErrores(const std::vector<ErrorLexico>& errores) {
    std::ostringstream out;
    size_t anchoLexema = std::string("Lexema").size();
    for (const ErrorLexico& e : errores) {
        anchoLexema = std::max(anchoLexema, e.lexema.size());
    }
    // Se usa "'" ... "'" solo para lexemas; el ancho es aproximado (bytes).
    out << std::left
        << std::setw(4)  << "N"
        << std::setw(8)  << "Linea"
        << std::setw(9)  << "Columna"
        << std::setw(static_cast<int>(anchoLexema) + 2) << "Lexema"
        << "Descripcion\n";
    out << std::string(4 + 8 + 9 + anchoLexema + 2 + 40, '-') << "\n";
    int n = 1;
    for (const ErrorLexico& e : errores) {
        out << std::left
            << std::setw(4)  << n++
            << std::setw(8)  << e.linea
            << std::setw(9)  << e.columna
            << std::setw(static_cast<int>(anchoLexema) + 2) << e.lexema
            << e.motivo << "\n";
    }
    return out.str();
}

// Mensaje final: indica si el análisis léxico fue correcto o no.
static std::string veredicto(const std::vector<ErrorLexico>& errores) {
    if (errores.empty()) {
        return "ANALISIS LEXICO CORRECTO: no se encontraron errores lexicos.";
    }
    return "ANALISIS LEXICO INCORRECTO: se encontraron " + std::to_string(errores.size()) +
           (errores.size() == 1 ? " error lexico." : " errores lexicos.");
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
    out << veredicto(errores) << "\n";
    return out.str();
}

static void escribirArchivo(const std::string& ruta, const std::string& contenido) {
    std::ofstream f(ruta, std::ios::binary);
    if (f.is_open()) f << contenido;
}

// ---------------------------------------------------------------------------
// Reportes del analizador sintáctico (Entregable 1)
// ---------------------------------------------------------------------------

// Tabla de errores sintácticos: N°, línea, columna, token encontrado, descripción.
static std::string tablaErroresSintacticos(const std::vector<ErrorSintactico>& errores) {
    std::ostringstream out;
    size_t anchoEnc = std::string("Encontrado").size();
    for (const ErrorSintactico& e : errores) {
        anchoEnc = std::max(anchoEnc, e.encontrado.size());
    }
    out << std::left
        << std::setw(4)  << "N"
        << std::setw(8)  << "Linea"
        << std::setw(9)  << "Columna"
        << std::setw(static_cast<int>(anchoEnc) + 2) << "Encontrado"
        << "Descripcion\n";
    out << std::string(4 + 8 + 9 + anchoEnc + 2 + 40, '-') << "\n";
    int n = 1;
    for (const ErrorSintactico& e : errores) {
        out << std::left
            << std::setw(4)  << n++
            << std::setw(8)  << e.linea
            << std::setw(9)  << e.columna
            << std::setw(static_cast<int>(anchoEnc) + 2) << e.encontrado
            << e.mensaje << "\n";
    }
    return out.str();
}

static std::string textoAviso(const AvisoSintactico& a) {
    return "Linea " + std::to_string(a.linea) + ", Columna " + std::to_string(a.columna) + ": " + a.mensaje;
}

static std::string veredictoSintactico(const std::vector<ErrorSintactico>& errores) {
    if (errores.empty()) {
        return "ANALISIS SINTACTICO CORRECTO: el programa cumple la gramatica de LP.";
    }
    return "ANALISIS SINTACTICO INCORRECTO: se encontraron " + std::to_string(errores.size()) +
           (errores.size() == 1 ? " error sintactico." : " errores sintacticos.");
}

// Reporte compacto y DETERMINISTA del análisis sintáctico (se compara contra tests/sintactico/esperado/).
static std::string resultadoSintactico(const NodoAST& arbol,
                                       const std::vector<ErrorSintactico>& errores,
                                       const std::vector<AvisoSintactico>& avisos,
                                       size_t erroresLexicos) {
    std::ostringstream out;
    out << "--- Errores lexicos previos: " << erroresLexicos << " ---\n";
    out << "--- Arbol sintactico (" << contarNodos(arbol) << " nodos) ---\n";
    out << arbolATexto(arbol);
    out << "--- Errores sintacticos (" << errores.size() << ") ---\n";
    for (const ErrorSintactico& e : errores) {
        out << "Linea " << e.linea << ", Columna " << e.columna << ": " << e.mensaje << "\n";
    }
    out << "--- Avisos (" << avisos.size() << ") ---\n";
    for (const AvisoSintactico& a : avisos) {
        out << textoAviso(a) << "\n";
    }
    out << veredictoSintactico(errores) << "\n";
    return out.str();
}

static void mostrarUso(const char* exe) {
    std::cerr << "Uso: " << exe << " <archivo.lp> [--salida <carpeta>] [--sin-comentarios] [--silencioso] [--solo-lexico]\n"
              << "  --salida <carpeta>   carpeta donde se escriben los resultados (por defecto: output)\n"
              << "  --sin-comentarios    reconoce los comentarios pero no los incluye en la lista de tokens\n"
              << "  --silencioso         no imprime el detalle por consola (solo genera los archivos)\n"
              << "  --solo-lexico        ejecuta unicamente el analizador lexico (comportamiento de la Fase 3)\n"
              << "Codigos de salida: 0 = correcto, 1 = no se pudo abrir el archivo,\n"
              << "                   2 = solo errores lexicos, 3 = errores sintacticos\n";
}

int main(int argc, char* argv[]) {
    std::string rutaEntrada = "tests/sintactico/validas/programa_minimo.lp";
    std::string carpetaSalida = "output";
    bool incluirComentarios = true;
    bool silencioso = false;
    bool soloLexico = false;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--sin-comentarios") {
            incluirComentarios = false;
        } else if (a == "--silencioso") {
            silencioso = true;
        } else if (a == "--solo-lexico") {
            soloLexico = true;
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
        }
    }

    std::string codigoFuente;
    if (!leerArchivo(rutaEntrada, codigoFuente)) {
        std::cerr << "Error: no se pudo abrir el archivo '" << rutaEntrada << "'\n";
        mostrarUso(argv[0]);
        return 1;
    }

    // =====================================================================
    // ETAPA 1: ANALISIS LEXICO (reutilizado sin cambios de la Fase 3)
    // =====================================================================
    Lexer lexer(codigoFuente);
    lexer.setIncluirComentarios(incluirComentarios);
    std::vector<Token> tokens = lexer.analizar();
    const std::vector<ErrorLexico>& errores = lexer.getErrores();
    const SymbolTable& tabla = lexer.getTablaSimbolos();

    // ---- Salida por consola ----
    if (!silencioso) {
        std::cout << (soloLexico ? "===== Analizador Lexico LP =====\n"
                                 : "===== Analizador LP: Lexico + Sintactico (Entregable 1) =====\n");
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

        std::cout << "\n--- Tabla de errores lexicos (" << errores.size() << ") ---\n";
        if (errores.empty()) {
            std::cout << "(sin errores)\n";
        } else {
            std::cout << tablaErrores(errores);
        }

        // Resumen: cuántos tokens hay de cada categoría.
        std::map<std::string, int> porCategoria;
        for (const Token& t : tokens) porCategoria[categoriaToken(t.tipo)]++;
        std::cout << "\n--- Resumen por categoria ---\n";
        for (const auto& par : porCategoria) {
            std::cout << par.first << ": " << par.second << "\n";
        }
        std::cout << "\n" << veredicto(errores) << "\n";
    }

    // ---- Salida a archivos (léxico) ----
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
        std::string contenido = errores.empty() ? std::string("(sin errores)\n") : tablaErrores(errores);
        escribirArchivo(base + "errores.txt", contenido + "\n" + veredicto(errores) + "\n");
    }
    escribirArchivo(base + "resultado.txt", resultadoCompacto(tokens, tabla, errores));

    if (soloLexico) {
        if (!silencioso) {
            std::cout << "\nSalidas generadas en " << base
                      << ": tokens.txt, secuencia_tokens.txt, tabla_simbolos.txt, errores.txt, resultado.txt\n";
        }
        // 0 = análisis correcto, 2 = hubo errores léxicos.
        return errores.empty() ? 0 : 2;
    }

    // =====================================================================
    // ETAPA 2: ANALISIS SINTACTICO (trabaja con los tokens de la etapa 1)
    // =====================================================================
    Parser parser(tokens);
    NodoPtr arbol = parser.analizar();
    const std::vector<ErrorSintactico>& erroresSint = parser.getErrores();
    const std::vector<AvisoSintactico>& avisos = parser.getAvisos();

    if (!silencioso) {
        std::cout << "\n===== Analisis sintactico =====\n";
        if (!errores.empty()) {
            std::cout << "(Nota: hubo " << errores.size()
                      << " error(es) lexico(s); esos caracteres no llegan al parser y pueden provocar errores sintacticos derivados.)\n";
        }
        std::cout << "\n--- Arbol sintactico (" << contarNodos(*arbol) << " nodos) ---\n";
        std::cout << arbolATexto(*arbol);

        std::cout << "\n--- Tabla de errores sintacticos (" << erroresSint.size() << ") ---\n";
        if (erroresSint.empty()) {
            std::cout << "(sin errores)\n";
        } else {
            std::cout << tablaErroresSintacticos(erroresSint);
        }

        if (!avisos.empty()) {
            std::cout << "\n--- Avisos (" << avisos.size() << ") ---\n";
            for (const AvisoSintactico& a : avisos) std::cout << textoAviso(a) << "\n";
        }
        std::cout << "\n" << veredictoSintactico(erroresSint) << "\n";
    }

    // ---- Salida a archivos (sintáctico) ----
    escribirArchivo(base + "arbol_sintactico.txt", arbolATexto(*arbol));
    {
        std::string contenido = erroresSint.empty() ? std::string("(sin errores)\n") : tablaErroresSintacticos(erroresSint);
        escribirArchivo(base + "errores_sintacticos.txt", contenido + "\n" + veredictoSintactico(erroresSint) + "\n");
    }
    escribirArchivo(base + "resultado_sintactico.txt",
                    resultadoSintactico(*arbol, erroresSint, avisos, errores.size()));

    if (!silencioso) {
        std::cout << "\nSalidas generadas en " << base
                  << ": tokens.txt, secuencia_tokens.txt, tabla_simbolos.txt, errores.txt, resultado.txt,\n"
                  << "                           arbol_sintactico.txt, errores_sintacticos.txt, resultado_sintactico.txt\n";
    }

    // Código de salida: 0 = todo correcto, 2 = solo errores léxicos, 3 = errores sintácticos
    // (1 = no se pudo abrir el archivo / opción inválida).
    if (!erroresSint.empty()) return 3;
    return errores.empty() ? 0 : 2;
}
