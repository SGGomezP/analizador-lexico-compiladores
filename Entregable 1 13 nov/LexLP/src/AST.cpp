#include "AST.h"

NodoAST& NodoAST::agregar(NodoPtr hijo) {
    // Un hijo nulo significa "esta parte no se pudo construir" (error ya reportado):
    // se ignora para no romper el dibujo del árbol.
    if (hijo) {
        hijos.push_back(std::move(hijo));
    }
    return *this;
}

NodoPtr crearNodo(const std::string& etiqueta, const std::string& valor, int linea, int columna) {
    return std::make_unique<NodoAST>(etiqueta, valor, linea, columna);
}

static void dibujar(const NodoAST& nodo, const std::string& prefijo, bool esUltimo,
                    bool esRaiz, std::string& salida) {
    if (esRaiz) {
        salida += nodo.etiqueta;
    } else {
        salida += prefijo + (esUltimo ? "`-- " : "|-- ") + nodo.etiqueta;
    }
    if (!nodo.valor.empty()) {
        salida += ": " + nodo.valor;
    }
    salida += "\n";

    // Prefijo de los hijos: si este nodo es el último de su nivel no se continúa la línea vertical.
    std::string prefijoHijos = esRaiz ? "" : prefijo + (esUltimo ? "    " : "|   ");
    for (size_t i = 0; i < nodo.hijos.size(); ++i) {
        dibujar(*nodo.hijos[i], prefijoHijos, i + 1 == nodo.hijos.size(), false, salida);
    }
}

std::string arbolATexto(const NodoAST& raiz) {
    std::string salida;
    dibujar(raiz, "", true, true, salida);
    return salida;
}

int contarNodos(const NodoAST& raiz) {
    int total = 1;
    for (const NodoPtr& h : raiz.hijos) {
        total += contarNodos(*h);
    }
    return total;
}
