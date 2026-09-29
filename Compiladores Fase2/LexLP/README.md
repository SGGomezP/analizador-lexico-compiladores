# LexLP — Analizador Léxico para el lenguaje LP

Proyecto del curso de Compiladores (Prof. Nury Y. Arosquipa Yanque). Analizador
léxico en C++ para el lenguaje simplificado **LP**, desarrollado en 3 fases
semanales.

## Estado actual: Fase 2 — ID + TEXTO + palabras reservadas + tabla de símbolos

| Fase | Fecha | Contenido | Estado |
|------|-------|-----------|--------|
| 1 | 15/09 | Lectura + `NUM_INT` + `NUM_DEC` | ✅ hecha |
| 2 | 22/09 | `ID` + `TEXTO` + palabras reservadas + tabla de símbolos | ✅ **esta entrega** |
| 3 | 29/09 | operadores, símbolos especiales, comentarios, integración, pruebas | ⏳ pendiente |

### Qué reconoce esta fase

| Token | Expresión regular | Ejemplo |
|-------|-------------------|---------|
| `NUM_INT` | `D+` | `20` |
| `NUM_DEC` | `D+\.D+` | `15.5` |
| `ID` | `L(L\|D)*` con `L=[a-zA-Z_]`, `D=[0-9]` | `edad`, `_x`, `nota1` |
| `TEXTO` | `".*"` | `"Hola mundo"` |
| Palabras reservadas | `int float char boolean void if else for while scanf println main return` | `INT`, `IF`, `MAIN`… |

Además:

- **Tabla de símbolos**: cada identificador se guarda una sola vez; el token
  lleva su posición: `<ID, 0>`. Las palabras reservadas **no** entran en la tabla.
- **Errores léxicos** con línea y columna (incluye textos sin cerrar).
- Todo lo que todavía no es de esta fase (operadores `= + - * / % && || ! < > <= >= == !=`,
  símbolos `( ) [ ] { } , ;` y comentarios `//`) se reporta como error léxico
  `caracter no reconocido`. **Es lo esperado hasta la Fase 3.**

## Estructura del proyecto

```
LexLP/
├── src/
│   ├── main.cpp           # lee el archivo, ejecuta el lexer, imprime y guarda resultados
│   ├── Lexer.h/.cpp       # analizador léxico (números, ID, palabras reservadas, TEXTO)
│   ├── Token.h/.cpp       # TokenType, Token y su formato <TIPO> / <ID, n>
│   └── SymbolTable.h/.cpp # tabla de símbolos (NUEVO en Fase 2)
├── tests/                 # programas .lp de prueba
├── output/                # tokens.txt, tabla_simbolos.txt, errores.txt (se generan)
├── Makefile
├── CMakeLists.txt
├── correr_pruebas.sh / correr_pruebas.bat
└── README.md
```

## Cómo funciona (resumen)

1. `main.cpp` lee el archivo `.lp` completo en un `std::string` (ignora el BOM UTF-8 si existe).
2. `Lexer::analizar()` llama repetidamente a `siguienteToken()`, que:
   - salta espacios, tabuladores y saltos de línea;
   - mira el primer carácter para decidir qué reconocer:
     - dígito → `reconocerNumero()` (`NUM_INT` o `NUM_DEC`);
     - letra o `_` → `reconocerIdentificador()`: lee `L(L|D)*` completo (coincidencia más larga),
       busca el lexema en la tabla de palabras reservadas; si está, devuelve ese token,
       si no, lo inserta en la `SymbolTable` y devuelve `<ID, posición>`;
     - `"` → `reconocerTexto()`: lee hasta la siguiente `"` en la misma línea;
     - cualquier otro carácter → error léxico (se consume y se sigue analizando).
3. Los errores no van a la lista de tokens: se acumulan aparte con línea/columna.
4. `main.cpp` imprime todo por consola y lo guarda en `output/`.

## Decisiones de diseño (casos límite documentados)

- **Coincidencia más larga**: `promedio2` es un solo `ID`; `notaFinal` también.
- **Palabras reservadas sensibles a mayúsculas**: `int` es `INT`, pero `Int`, `INT` e `integer` son `ID`.
  Un ID que *empieza* con una palabra reservada (`ifelse`, `int2`, `_int`) sigue siendo `ID`.
- **`L` no incluye letras acentuadas ni `ñ`**: `niño` se lee como `ni` (ID) + `ñ` (error) + `o` (ID).
- **Números pegados a letras**: `12abc` → `NUM_INT(12)` + `ID(abc)` (dos tokens, sin error).
- **TEXTO** termina en la **primera** `"` de cierre. No puede ocupar varias líneas (el `.` de `".*"`
  no incluye `\n`). No se inventaron secuencias de escape (`\"` no está en la especificación).
- **Texto sin cerrar** (llega fin de línea o de archivo sin `"`): se reporta **un solo** error
  desde la comilla de apertura hasta el fin de la línea, y el análisis continúa en la línea siguiente.
- **Caracteres UTF-8 multibyte** (`ñ`, `á`…) fuera de un texto se reportan como un solo error y
  la columna cuenta caracteres, no bytes. Dentro de un `TEXTO` se aceptan sin problema.
- **Errores en cascada**: `$variable` genera error en `$` y luego el `ID` `variable` (recuperación por carácter).

## Compilación

Requiere un compilador con soporte C++17 (g++ o clang++, o Visual Studio).

**Con make (Linux / macOS / MinGW):**
```bash
make
```

**Con g++ directamente (Windows con MinGW, o cualquier sistema):**
```bash
g++ -std=c++17 -Wall -Wextra -O2 -o lexlp src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp
```

**Con CMake / Visual Studio:** abrir la carpeta `LexLP` (usa `CMakeLists.txt`).

## Ejecución

```bash
./lexlp tests/prueba_programa.lp        # Linux/macOS
lexlp.exe tests\prueba_programa.lp      # Windows
```

Sin argumentos usa `tests/prueba_basica.lp`. Ejecutar siempre **desde la carpeta `LexLP`**
(las salidas se escriben en `output/`, que se crea si no existe).

Para correr todas las pruebas de una vez: `./correr_pruebas.sh` (Linux/macOS) o `correr_pruebas.bat` (Windows).

## Pruebas y resultados esperados

| Archivo | Qué comprueba | Resultado esperado |
|---------|---------------|--------------------|
| `prueba_basica.lp` | Fase 1: enteros y decimales | 5 tokens `NUM_INT`/`NUM_DEC`, tabla vacía, 0 errores |
| `prueba_completa.lp` | Fase 1: casos límite `10.` y `12.3.4` | 2 errores (`.` en L4 C3 y L6 C5) |
| `prueba_errores.lp` | Fase 1: `@`, `#`, `&` | 3 errores |
| `prueba_ids.lp` | IDs válidos y repetidos | tabla de 12 IDs: `edad, promedio, nota1, notaFinal, _promedio, _x, x_1, A1b2, x1y2z3, __, _, a`; `edad` repetido → `<ID, 0>` |
| `prueba_reservadas.lp` | las 13 palabras reservadas | 13 tokens (`INT FLOAT … RETURN`), tabla **vacía** |
| `prueba_reservadas_limite.lp` | `Int INT Float RETURN integer mainx printlnn ifelse int2 _int` | los 10 son `ID`; al final `main int main` → `MAIN INT MAIN` |
| `prueba_texto.lp` | textos normales, vacío, con símbolos y con `//` | 8 tokens `TEXTO`, 1 solo ID (`edad`), 0 errores |
| `prueba_tabla_simbolos.lp` | ejemplo 5.3 del enunciado | tabla `0 edad`, `1 promedio`; `<ID,0>` se repite en las 3 líneas |
| `prueba_programa.lp` | programa completo de la sección 12 | tabla `0 edad`, `1 promedio` (igual al enunciado). Los `= ; ( ) { } >=` salen como error hasta la Fase 3 |
| `prueba_errores_fase2.lp` | `@`, `$`, `#`, `ñ`, textos sin cerrar | 7 errores (ver archivo de errores) |
| `prueba_mezcla.lp` | `x = 12abc + 3.5y 7.` | `<ID,0> <NUM_INT> <ID,1> <NUM_DEC> <ID,2> <NUM_INT>` y errores en `=`, `+`, `.` |
