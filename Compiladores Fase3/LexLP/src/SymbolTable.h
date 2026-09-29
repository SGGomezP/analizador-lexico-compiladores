#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <unordered_map>
#include <vector>

// Tabla de símbolos de LP (sección 5.2 del documento del proyecto).
//
// En esta etapa SOLO almacena identificadores (tokens ID). Cada identificador
// aparece una única vez: si vuelve a aparecer en el código fuente no se
// inserta de nuevo, se recupera la posición que ya tenía.
//
// Internamente usa dos estructuras:
//   - un vector<string>   -> conserva el orden de inserción (posición 0, 1, 2...)
//   - un unordered_map    -> permite buscar un identificador en O(1)
class SymbolTable {
public:
    // Inserta el identificador si no existe y devuelve su posición.
    // Si ya existía, NO lo duplica y devuelve la posición existente.
    int insertar(const std::string& identificador);

    // Devuelve la posición del identificador, o -1 si no está en la tabla.
    int buscar(const std::string& identificador) const;

    // Identificador almacenado en la posición 'posicion' (0 <= posicion < tamano()).
    const std::string& obtener(int posicion) const;

    // Cantidad de identificadores distintos almacenados.
    size_t tamano() const { return identificadores.size(); }

    // Todos los identificadores, en orden de posición.
    const std::vector<std::string>& getIdentificadores() const { return identificadores; }

    // Vacía la tabla.
    void limpiar();

private:
    std::vector<std::string> identificadores;
    std::unordered_map<std::string, int> indice;
};

#endif // SYMBOLTABLE_H
