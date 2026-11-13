#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

// Nodo del árbol sintáctico (de derivación simplificado) que construye el Parser.
//
//   etiqueta : no terminal o tipo de nodo   ("Programa", "Funcion", "Declaracion", "Op", "ID", ...)
//   valor    : lexema asociado, si lo hay   (el operador "+", el identificador "edad", el número "20", ...)
//   linea/columna : posición del token que originó el nodo (útil para fases posteriores)
//
// Se usa std::unique_ptr: cada nodo es dueño de sus hijos, no hay que liberar memoria a mano.
struct NodoAST;
using NodoPtr = std::unique_ptr<NodoAST>;

struct NodoAST {
    std::string etiqueta;
    std::string valor;
    int linea;
    int columna;
    std::vector<NodoPtr> hijos;

    NodoAST(std::string etiqueta_, std::string valor_ = "", int linea_ = 0, int columna_ = 0)
        : etiqueta(std::move(etiqueta_)), valor(std::move(valor_)), linea(linea_), columna(columna_) {}

    // Agrega un hijo (ignora punteros nulos) y devuelve una referencia al nodo agregado.
    NodoAST& agregar(NodoPtr hijo);
};

// Crea un nodo ya envuelto en unique_ptr.
NodoPtr crearNodo(const std::string& etiqueta, const std::string& valor = "", int linea = 0, int columna = 0);

// Dibuja el árbol en texto (ASCII, indentado), por ejemplo:
//   Programa
//   `-- Funcion
//       |-- Tipo: void
//       `-- Bloque
std::string arbolATexto(const NodoAST& raiz);

// Cantidad total de nodos del árbol (para el resumen del reporte).
int contarNodos(const NodoAST& raiz);

#endif // AST_H
