# LexLP — Analizador Léxico para el lenguaje LP

Proyecto del curso de **Compiladores** (Prof. Nury Y. Arosquipa Yanque). Analizador léxico en C++
para el lenguaje simplificado **LP**, desarrollado en 3 fases semanales.

## Estado: Fase 3 (final) — tokens completos + integración + pruebas

| Fase | Fecha | Contenido | Estado |
|------|-------|-----------|--------|
| 1 | 15/09 | Lectura + `NUM_INT` + `NUM_DEC` | ✅ |
| 2 | 22/09 | `ID` + `TEXTO` + palabras reservadas + tabla de símbolos | ✅ |
| 3 | 29/09 | operadores + símbolos especiales + comentarios + integración + lista de tokens + errores + pruebas | ✅ **esta entrega** |

## Integrantes

| Nombre | Aporte |
|--------|--------|
| _(completar)_ | _(p. ej. Fase 1: lectura y números)_ |
| _(completar)_ | _(p. ej. Fase 2: ID, TEXTO y tabla de símbolos)_ |
| _(completar)_ | _(p. ej. Fase 3: operadores, pruebas y documentación)_ |

## Tokens reconocidos (lista completa del curso)

| Expresión regular | Token (nomenclatura de la 2.ª columna) | Categoría |
|---|---|---|
| `D+` | `NUM_INT` | Número entero |
| `D+\.D+` | `NUM_DEC` | Número decimal |
| `L(L\|D)*` | `ID` → `<ID, n>` | Identificador |
| `".*"` | `TEXTO` | Constante de texto |
| `int float char boolean void if else for while scanf println main return` | `INT FLOAT CHAR BOOLEAN VOID IF ELSE FOR WHILE SCANF PRINTLN MAIN RETURN` | Palabra reservada |
| `//.*\n` | `COMENT` | Comentario |
| `=` | `=` | Operador de asignación |
| `+ - * / %` | `+` `-` `*` `/` `%` | Operador aritmético |
| `&& \|\| !` | `&&` `\|\|` `!` | Operador lógico |
| `> >= < <= != ==` | `COMP` → `<COMP, op>` | Operador de comparación / relacional |
| `( ) [ ] { } , ;` | `(` `)` `[` `]` `{` `}` `,` `;` | Símbolo especial |

Donde `D = [0-9]` y `L = [a-zA-Z_]`.

### Formato de la lista de tokens

```
<NUM_INT>   <NUM_DEC>   <ID, 0>   <TEXTO>   <INT>   <+>   <&&>   <COMP, >=>   <(>   <;>   <COMENT>
```

* **Atributo de `ID`**: posición del identificador en la tabla de símbolos.
* **Atributo de `COMP`**: el operador concreto (`>`, `>=`, `<`, `<=`, `!=`, `==`).
* Los demás tokens no llevan atributo.

## Estructura del repositorio

```
LexLP/
├── src/
│   ├── main.cpp            # argumentos, lectura del archivo, reportes y archivos de salida
│   ├── Lexer.h/.cpp        # analizador léxico (todos los tokens)
│   ├── Token.h/.cpp        # TokenType, Token, nombre y categoría de cada token
│   └── SymbolTable.h/.cpp  # tabla de símbolos (identificadores únicos)
├── tests/
│   ├── validas/            # entradas correctas
│   ├── invalidas/          # entradas con errores léxicos
│   ├── limite/             # casos límite
│   └── esperado/           # resultado esperado de cada prueba (una por archivo .lp)
├── output/                 # ejemplo de salida generada (tokens, tabla, errores)
├── docs/                   # guía de sustentación y checklist del repositorio
├── Makefile · CMakeLists.txt
├── correr_pruebas.sh / correr_pruebas.bat
└── README.md
```

## Compilación

Requiere un compilador con soporte C++17.

```bash
make                                   # Linux / macOS / MinGW
# o directamente:
g++ -std=c++17 -Wall -Wextra -O2 -o lexlp src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp
```

Con **CMake / Visual Studio**: abrir la carpeta `LexLP` (usa `CMakeLists.txt`).

## Ejecución

Ejecutar **desde la carpeta `LexLP`**:

```bash
./lexlp tests/validas/prueba_programa_completo.lp        # Linux/macOS
lexlp.exe tests\validas\prueba_programa_completo.lp      # Windows
```

Opciones:

| Opción | Efecto |
|--------|--------|
| `--salida <carpeta>` | carpeta donde se escriben los resultados (por defecto `output/`) |
| `--sin-comentarios` | reconoce los comentarios pero no los incluye en la lista de tokens |
| `--silencioso` | no imprime por consola, solo genera los archivos |

### Archivos que genera (`output/`)

| Archivo | Contenido |
|---------|-----------|
| `tokens.txt` | un token por línea: token, categoría, lexema, línea y columna |
| `secuencia_tokens.txt` | lista de tokens en formato compacto, una línea del fuente por línea |
| `tabla_simbolos.txt` | posición e identificador (cada ID aparece una sola vez) |
| `errores.txt` | errores léxicos con línea, columna y motivo |
| `resultado.txt` | resumen determinista (tokens + tabla + errores), usado por las pruebas |

## Pruebas automáticas

```bash
./correr_pruebas.sh                 # compara cada prueba con tests/esperado/   (Linux/macOS/MinGW)
./correr_pruebas.sh --actualizar    # regenera los esperados (revisarlos a mano antes de commitear)
correr_pruebas.bat                  # Windows (usa PowerShell para comparar)
```

Resultado actual: **30 / 30 pruebas correctas**.

| Carpeta | Qué cubre |
|---------|-----------|
| `tests/validas/` (13) | programa de la sección 12, programa completo con todas las categorías, cada grupo de operadores, comparaciones, símbolos, comentarios, ID/TEXTO/reservadas/tabla de símbolos |
| `tests/invalidas/` (6) | caracteres inválidos (`@ # $ ? ~ ^ \ ' : .`), `&` y `\|` solitarios, textos sin cerrar, errores mezclados con código válido |
| `tests/limite/` (11) | operadores pegados (`>= == != <=> === !== &&& a+++b`), `=>`, `! =`, comentarios (`//`, `///`, `a//b`, `/ /`, sin salto de línea final, CRLF), archivo vacío, solo espacios, `10.`, `1.2.3` |

## Decisiones de diseño (casos límite documentados)

**Generales**
* **Coincidencia más larga** en todos los tokens: `>=` es un `COMP`, no `>` + `=`; `promedio2` es un solo `ID`.
* **Errores sin detener el análisis**: se consume un carácter, se registra línea/columna y se continúa.
  Los errores **no** entran en la lista de tokens.
* Las **palabras reservadas** no entran en la tabla de símbolos y son sensibles a mayúsculas
  (`Int`, `INT`, `integer` son `ID`; `int2`, `_int`, `ifelse` también).
* `L` no incluye letras acentuadas ni `ñ`: `niño` → `ni` (ID) + `ñ` (error) + `o` (ID).
* La columna cuenta caracteres, no bytes (UTF-8); se ignora el BOM inicial.

**Fase 3**
* **`&` y `|` solitarios** no existen en LP (solo `&&` y `||`): error `operador incompleto`.
* `==`, `!=`, `<=`, `>=` se resuelven con un carácter de *lookahead*. `a===b` → `==` `=`; `a!==b` → `!=` `=`;
  `! =` (con espacio) → `!` `=`.
* **`COMENT = //.*\n`**: se reconoce desde `//` hasta el fin de línea (el `\n` se consume pero no se guarda
  en el lexema; tampoco el `\r` de Windows). Un comentario en la **última línea sin `\n`** también es válido.
  Por defecto se emite `<COMENT>` en la lista (es un token de la lista del curso);
  con `--sin-comentarios` se omite.
* **`//` dentro de un `TEXTO`** no es comentario (`"a // b"` es un solo `TEXTO`); una `"` dentro de un comentario
  tampoco abre un texto. `/ /` son dos `DIV`; `a/b` es `ID DIV ID`.
* **Números negativos**: `-5` son dos tokens (`-` y `NUM_INT`); la especificación no define signo en el número.
* **`10.` y `.5`**: el `.` suelto es error léxico (`D+\.D+` exige dígitos a ambos lados).
* **TEXTO** termina en la primera `"` y no cruza líneas; un texto sin cerrar se reporta **una sola vez**
  hasta el fin de la línea.

## Cómo funciona (resumen)

1. `main.cpp` lee el `.lp` completo en un `std::string` y crea el `Lexer`.
2. `Lexer::analizar()` llama repetidamente a `siguienteToken()`, que salta espacios y mira el primer carácter:
   dígito → `reconocerNumero()` · letra o `_` → `reconocerIdentificador()` (reservada o `ID` + tabla de símbolos) ·
   `"` → `reconocerTexto()` · `//` → `reconocerComentario()` · cualquier otro → `reconocerOperador()`
   (operadores, símbolos o error).
3. Tokens, tabla de símbolos y errores se acumulan por separado; `main.cpp` los imprime y los guarda en `output/`.
