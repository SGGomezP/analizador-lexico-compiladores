# LexLP — Analizador Léxico y Sintáctico para el lenguaje LP

Proyecto del curso de **Compiladores** (Prof. Nury Y. Arosquipa Yanque). Compilador en C++ para el lenguaje
simplificado **LP**: analizador léxico (Fases 1–3) + **analizador sintáctico integrado** (Entregables 1 y 2).

## Estado: Entregable 1 (13/11) — integración léxico + sintáctico, estructura, declaraciones y asignaciones

| Etapa | Fecha | Contenido | Estado |
|-------|-------|-----------|--------|
| Fase 1 | 15/09 | Lectura + `NUM_INT` + `NUM_DEC` | ✅ |
| Fase 2 | 22/09 | `ID` + `TEXTO` + palabras reservadas + tabla de símbolos | ✅ |
| Fase 3 | 29/09 | operadores + símbolos + comentarios + integración + errores + pruebas | ✅ |
| **Entregable 1** | **13/11** | **integración con el léxico + parser: estructura del programa, declaraciones, asignaciones (y expresiones)** | ✅ **esta entrega** |
| Entregable 2 | _(por definir)_ | `if / else / while / for / scanf / println` | ⏳ (hoy se reconocen y se omiten con un aviso) |

## Integrantes

| Nombre | Aporte |
|--------|--------|
| _(completar)_ | _(p. ej. Fase 1: lectura y números)_ |
| _(completar)_ | _(p. ej. Fase 2: ID, TEXTO y tabla de símbolos)_ |
| _(completar)_ | _(p. ej. Fase 3: operadores, pruebas y documentación)_ |

## Analizador sintáctico (Entregable 1)

**Técnica:** descenso recursivo predictivo (LL(1)); un método de `Parser` por cada no terminal de la gramática.
Trabaja directamente con la lista de tokens que entrega `Lexer::analizar()`.

**Qué reconoce hoy**

| Construcción | Ejemplo |
|---|---|
| Estructura del programa (funciones, `main`, parámetros, bloques) | `int suma(int a, int b) { ... }`  ·  `void main() { ... }` |
| Declaraciones (todos los tipos, varias por línea, arreglos, inicialización) | `int a, b = 2;`  ·  `float notas[5];` |
| Asignaciones (variable y elemento de arreglo) | `x = 5;`  ·  `v[i + 1] = x * 2;` |
| Expresiones con precedencia: `\|\|` `&&` `COMP` `+ -` `* / %` `! -`, paréntesis, índices y llamadas | `r = a > b && !(c == 0);` |
| `return` y llamadas como sentencia | `return a + b;`  ·  `saludar();` |
| `if / else / while / for / scanf / println` | **Entregable 2**: se reconocen, **se omiten** y se emite un *aviso* |

La gramática completa (con conjuntos PRIMERO y las decisiones a confirmar) está en
[`docs/GRAMATICA_LP.md`](docs/GRAMATICA_LP.md).

**Errores sintácticos:** se informan con línea, columna, qué se esperaba y qué se encontró, y el parser **se recupera**
(modo pánico) para reportar varios errores en una sola corrida:

```
N   Linea   Columna  Encontrado  Descripcion
-------------------------------------------------------------
1   2       14       float       Se esperaba ';' al final de la declaracion (se encontro la palabra reservada 'float')
2   3       11       =           Se esperaba un identificador en la declaracion, pero se encontro '='
```

**Árbol sintáctico** (`output/arbol_sintactico.txt`) del bloque `{ int x = 5; x = x + 1; }`:

```
Bloque
|-- Declaracion
|   |-- Tipo: int
|   `-- Variable
|       |-- ID: x  <ID, 0>
|       `-- Inicializacion
|           `-- NUM_INT: 5
`-- Asignacion
    |-- ID: x  <ID, 0>
    `-- Op: +
        |-- ID: x  <ID, 0>
        `-- NUM_INT: 1
```

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
│   ├── main.cpp            # argumentos, lectura, orquesta léxico -> sintáctico, reportes y archivos de salida
│   ├── Lexer.h/.cpp        # analizador léxico (sin cambios respecto a la Fase 3)
│   ├── Token.h/.cpp        # TokenType, Token, nombre y categoría de cada token
│   ├── SymbolTable.h/.cpp  # tabla de símbolos (identificadores únicos)
│   ├── Parser.h/.cpp       # NUEVO: analizador sintáctico (descenso recursivo + recuperación de errores)
│   └── AST.h/.cpp          # NUEVO: nodo del árbol sintáctico y su dibujo en texto
├── tests/
│   ├── validas/ invalidas/ limite/ esperado/   # pruebas del LÉXICO (31)
│   └── sintactico/
│       ├── validas/        # programas correctos (11)
│       ├── invalidas/      # programas con errores sintácticos (20)
│       └── esperado/       # resultado esperado de cada prueba
├── output/                 # ejemplo de salida generada (tokens, tabla, errores, árbol)
├── docs/
│   ├── GRAMATICA_LP.md                  # gramática implementada, PRIMERO, decisiones a confirmar
│   ├── GUIA_SUSTENTACION_ENTREGABLE1.md # flujo, preguntas probables, cambios en vivo
│   ├── GUIA_SUSTENTACION.md             # (Fase 3, léxico)
│   └── GITHUB_CHECKLIST.md
├── Makefile · CMakeLists.txt
├── correr_pruebas.sh/.bat                   # pruebas del léxico
├── correr_pruebas_sintacticas.sh/.bat       # pruebas del sintáctico
└── README.md
```

## Compilación

Requiere un compilador con soporte C++17.

```bash
make                                   # Linux / macOS / MinGW
# o directamente:
g++ -std=c++17 -Wall -Wextra -O2 -o lexlp src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp src/Parser.cpp src/AST.cpp
```

Con **CMake / Visual Studio**: abrir la carpeta `LexLP` (usa `CMakeLists.txt`).

## Ejecución

Ejecutar **desde la carpeta `LexLP`**:

```bash
./lexlp tests/sintactico/validas/funciones.lp          # Linux/macOS: léxico + sintáctico
lexlp.exe tests\sintactico\validas\funciones.lp        # Windows
./lexlp programa.lp --solo-lexico                      # solo el analizador léxico (como en la Fase 3)
```

| Opción | Efecto |
|--------|--------|
| `--salida <carpeta>` | carpeta donde se escriben los resultados (por defecto `output/`) |
| `--sin-comentarios` | reconoce los comentarios pero no los incluye en la lista de tokens |
| `--silencioso` | no imprime por consola, solo genera los archivos |
| `--solo-lexico` | ejecuta únicamente el analizador léxico (no se llama al parser) |

**Código de salida:** `0` correcto · `1` no se pudo abrir el archivo · `2` solo errores léxicos · `3` errores sintácticos.

### Archivos que genera (`output/`)

| Archivo | Contenido |
|---------|-----------|
| `tokens.txt` | un token por línea: token, categoría, lexema, línea y columna |
| `secuencia_tokens.txt` | lista de tokens en formato compacto, una línea del fuente por línea |
| `tabla_simbolos.txt` | posición e identificador (cada ID aparece una sola vez) |
| `errores.txt` | tabla de errores **léxicos** y veredicto |
| `resultado.txt` | resumen léxico determinista (usado por las pruebas del léxico) |
| `arbol_sintactico.txt` | **árbol sintáctico** del programa (parcial si hubo errores) |
| `errores_sintacticos.txt` | tabla de errores **sintácticos** y veredicto |
| `resultado_sintactico.txt` | resumen sintáctico determinista (usado por las pruebas del parser) |

## Pruebas automáticas

```bash
./correr_pruebas.sh                            # léxico:     31 / 31 correctas
./correr_pruebas_sintacticas.sh                # sintáctico: 31 / 31 correctas
./correr_pruebas_sintacticas.sh --actualizar   # regenera los esperados (revisarlos a mano antes de commitear)
make test                                      # ambas
```

El script sintáctico además verifica el **código de salida** (`validas` → 0, `invalidas` → 3).

| Carpeta | Qué cubre |
|---|---|
| `tests/sintactico/validas/` (11) | programa mínimo, declaraciones, asignaciones, expresiones y precedencia, funciones y llamadas, comentarios, bloques anidados, varias funciones, programas completos con construcciones del Entregable 2 |
| `tests/sintactico/invalidas/` (20) | `;` faltante, `}` faltante/sobrante, paréntesis, declaración sin identificador, asignación sin expresión o mal formada, sin `main`, archivo vacío, declaración global, arreglos mal formados, encabezados de función mal formados, `else` sin `if`, función anidada, errores múltiples, errores léxicos + sintácticos |

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

**Entregable 1 (sintáctico)**
* **Descenso recursivo** (no tabla LL(1) ni LR): cada no terminal es un método; se decide con 1 token de lookahead (2 para `ID '('`).
* **Precedencia por niveles + asociatividad izquierda con `while`** (sin recursión por la izquierda). Los paréntesis solo agrupan: no generan nodo.
* **El léxico no cambia**: el parser recibe `vector<Token>` y descarta `COMENT`. Los errores léxicos no llegan al parser; si existen, el reporte
  lo indica porque pueden causar errores sintácticos derivados.
* **Recuperación de errores**: `;` faltante → se reporta tras el último token válido y se continúa; otros errores → salto hasta `;` / `}` /
  inicio de sentencia / inicio de función. No se reportan dos errores en el mismo token y el parser siempre avanza (sin ciclos infinitos).
* **`main` faltante** se informa solo si no hay otros errores (evita un error derivado).
* **Entregable 2**: `if / else / while / for / scanf / println` se reconocen y se **omiten** (salto de paréntesis/llaves balanceadas) con un
  *aviso* — así los programas completos no producen falsos errores, y queda explícito que su sintaxis aún no se verificó.
* **Gramática reconstruida**: varias decisiones (sin declaraciones globales, asignación como sentencia, etc.) deben **confirmarse con el anexo
  oficial**; ver la tabla en `docs/GRAMATICA_LP.md`.

## Cómo funciona (resumen)

1. `main.cpp` lee el `.lp` completo en un `std::string`.
2. **Etapa léxica:** `Lexer::analizar()` produce la lista de tokens, la tabla de símbolos y los errores léxicos (ver `docs/GUIA_SUSTENTACION.md`).
3. **Etapa sintáctica:** `Parser(tokens).analizar()` recorre los tokens con descenso recursivo:
   `programa → funcion → bloque → sentencia → (declaracion | asignacionOLlamada | retorno | bloque)`, y las expresiones por niveles de
   precedencia `exprOr → … → primario`. Devuelve el árbol y acumula errores y avisos.
4. `main.cpp` imprime y guarda tokens, tabla de símbolos, errores léxicos, **árbol sintáctico** y **errores sintácticos**, y decide el código de salida.
