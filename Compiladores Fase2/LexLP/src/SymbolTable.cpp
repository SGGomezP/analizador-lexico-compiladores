#include "SymbolTable.h"

int SymbolTable::insertar(const std::string& identificador) {
    auto it = indice.find(identificador);
    if (it != indice.end()) {
        return it->second; // ya existe: se recupera la posición, no se duplica
    }
    int posicion = static_cast<int>(identificadores.size());
    identificadores.push_back(identificador);
    indice.emplace(identificador, posicion);
    return posicion;
}

int SymbolTable::buscar(const std::string& identificador) const {
    auto it = indice.find(identificador);
    return (it != indice.end()) ? it->second : -1;
}

const std::string& SymbolTable::obtener(int posicion) const {
    return identificadores.at(static_cast<size_t>(posicion));
}

void SymbolTable::limpiar() {
    identificadores.clear();
    indice.clear();
}
