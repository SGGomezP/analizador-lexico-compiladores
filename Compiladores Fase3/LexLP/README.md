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
| Rafael | **Fase 1**: lectura del archivo carácter por carácter (línea/columna), reconocimiento de `NUM_INT` y `NUM_DEC`, y estructura base de `Token` y `TokenType`. |
| Sebastián | **Fase 2**: reconocimiento de `ID` y `TEXTO`, las 13 palabras reservadas y la tabla de símbolos (`SymbolTable`) sin identificadores duplicados. |
| Albana | **Fase 3**: operadores aritméticos, lógicos, de asignación y de comparación (`COMP` con atributo), símbolos especiales y comentarios (`COMENT`); regla de coincidencia más larga. |
| Daniel | **Fase 3**: integración en `main.cpp`, tabla de errores y veredicto final, validación de números mal formados y comillas sin cerrar, casos de prueba (`tests/`), script de pruebas automáticas y documentación. |

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
| `errores.txt` | **tabla de errores** léxicos (N.º, línea, columna, lexema, descripción) y el veredicto final |
| `resultado.txt` | resumen determinista (tokens + tabla + errores), usado por las pruebas |

## Pruebas automáticas

```bash
./correr_pruebas.sh                 # compara cada prueba con tests/esperado/   (Linux/macOS/MinGW)
./correr_pruebas.sh --actualizar    # regenera los esperados (revisarlos a mano antes de commitear)
correr_pruebas.bat                  # Windows (usa PowerShell para comparar)
```

Resultado actual: **31 / 31 pruebas correctas**.

| Carpeta | Qué cubre |
|---------|-----------|
| `tests/validas/` (13) | programa de la sección 12, programa completo con todas las categorías, cada grupo de operadores, comparaciones, símbolos, comentarios, ID/TEXTO/reservadas/tabla de símbolos |
| `tests/invalidas/` (7) | caracteres inválidos (`@ # $ ? ~ ^ \ ' : .`), `&` y `\|` solitarios, textos con comillas sin cerrar, **números mal formados** (`12.3.4`, `1.2.3.4`, `5..3`, `10.`, `.5`), errores mezclados con código válido |
| `tests/limite/` (11) | operadores pegados (`>= == != <=> === !== &&& a+++b`), `=>`, `! =`, comentarios (`//`, `///`, `a//b`, `/ /`, sin salto de línea final, CRLF), archivo vacío, solo espacios, `10.`, `1.2.3` (números mal formados) |

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
* **Números mal formados** (`12.3.4`, `1.2.3.4`, `5..3`, `10.`, `.5`, `3.14.`): NO se acepta un prefijo válido
  (`12.3`) dejando el resto suelto. Se consume todo el bloque de dígitos y puntos y se reporta **un solo error**
  (`numero decimal invalido`) con su motivo: más de un punto, falta la parte decimal o falta la parte entera.
  No genera ningún token (`NUM_DEC` exige `D+\.D+` exacto).
* **Comillas**: `TEXTO` termina en la primera `"` y no cruza líneas; si falta la comilla de cierre se reporta
  **un solo error** (`texto sin cerrar (falta la comilla de cierre)`) desde la comilla hasta el fin de la línea.

**Reporte de errores y veredicto**
* Los errores se muestran en una **tabla** (N.º, línea, columna, lexema, descripción), por consola y en `errores.txt`.
* Al final se imprime el veredicto: `ANALISIS LEXICO CORRECTO` o
  `ANALISIS LEXICO INCORRECTO: se encontraron N errores lexicos.`
* Código de salida del programa: `0` = sin errores léxicos, `2` = con errores léxicos, `1` = no se pudo abrir el archivo.

## Cómo funciona (resumen)

1. `main.cpp` lee el `.lp` completo en un `std::string` y crea el `Lexer`.
2. `Lexer::analizar()` llama repetidamente a `siguienteToken()`, que salta espacios y mira el primer carácter:
   dígito → `reconocerNumero()` · letra o `_` → `reconocerIdentificador()` (reservada o `ID` + tabla de símbolos) ·
   `"` → `reconocerTexto()` · `//` → `reconocerComentario()` · cualquier otro → `reconocerOperador()`
   (operadores, símbolos o error).
3. Tokens, tabla de símbolos y errores se acumulan por separado; `main.cpp` los imprime y los guarda en `output/`.