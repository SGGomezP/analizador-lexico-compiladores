# Gramática de LP implementada en el analizador sintáctico (Entregable 1)

> ## ⚠️ VERIFICAR contra el anexo oficial antes de entregar
>
> El documento con las producciones de la gramática LP **no venía incluido** cuando se desarrolló esta entrega.
> Esta gramática se reconstruyó a partir de (1) los tokens del analizador léxico, (2) los programas de prueba
> del proyecto (`void main() {...}`, `int suma(int a, int b)`, `int notas[5]`, `y = x = 3`, …) y (3) el alcance
> del Entregable 1 (estructura, declaraciones y asignaciones).
>
> **Comparen cada producción con el anexo.** Al final hay una tabla *"Decisiones a confirmar"* que indica,
> para cada punto dudoso, **qué método de `src/Parser.cpp` cambiar**. Como cada no terminal es un método,
> ajustar una producción son pocas líneas.

Notación: `→` produce · `|` alternativa · `{ X }` cero o más veces · `[ X ]` opcional · `'x'` terminal literal ·
`MAYÚSCULAS` = token del analizador léxico (`ID`, `NUM_INT`, `NUM_DEC`, `TEXTO`, `COMP`, …).

## 1. Estructura del programa

```
Programa      → Funcion { Funcion }                      (debe existir la función main)
Funcion       → TipoRetorno NombreFuncion '(' [ ListaParam ] ')' Bloque
TipoRetorno   → 'int' | 'float' | 'char' | 'boolean' | 'void'
NombreFuncion → ID | 'main'
ListaParam    → Parametro { ',' Parametro }
Parametro     → Tipo ID [ '[' ']' ]
Tipo          → 'int' | 'float' | 'char' | 'boolean'
Bloque        → '{' { Sentencia } '}'
```

## 2. Sentencias

```
Sentencia     → Declaracion
              | Asignacion
              | LlamadaSent
              | Return
              | Bloque
              | If | While | For | Scanf | Println        ← ENTREGABLE 2 (hoy se omiten con un aviso)

Declaracion   → Tipo DeclVar { ',' DeclVar } ';'
DeclVar       → ID [ '[' NUM_INT ']' ] [ '=' Expresion ]    (un arreglo no se inicializa en la declaración)
Asignacion    → Variable '=' Expresion ';'
Variable      → ID | ID '[' Expresion ']'
LlamadaSent   → Llamada ';'
Return        → 'return' [ Expresion ] ';'
```

## 3. Expresiones (de menor a mayor precedencia)

```
Expresion → ExprOr
ExprOr    → ExprAnd  { '||' ExprAnd }
ExprAnd   → ExprComp { '&&' ExprComp }
ExprComp  → ExprSuma { COMP ExprSuma }              COMP = > >= < <= != ==
ExprSuma  → ExprMult { ( '+' | '-' ) ExprMult }
ExprMult  → ExprUna  { ( '*' | '/' | '%' ) ExprUna }
ExprUna   → ( '!' | '-' ) ExprUna | Primario
Primario  → NUM_INT | NUM_DEC | TEXTO | Llamada | Variable | '(' Expresion ')'
Llamada   → ID '(' [ Expresion { ',' Expresion } ] ')'
```

* Todos los binarios son **asociativos a la izquierda** (se implementan con un `while`, no con recursión izquierda).
* Cuanto más abajo está un nivel, **más fuerte liga**: `a + b * 3` = `a + (b * 3)`.
* El `-` unario se resuelve aquí (el léxico entrega `-5` como `-` + `NUM_INT`).

## 4. ¿Por qué es predictiva (sin retroceso)?

Cada alternativa se decide mirando **un solo token** (LL(1)), salvo dos casos que miran **dos**:

| Decisión | Tokens mirados | Dónde |
|---|---|---|
| `ID '('` → llamada · `ID '['`/`ID` → variable | `ID` + siguiente | `primario()`, `asignacionOLlamada()` |
| `Tipo ID '('` → inicio de función (para recuperarse de errores) | 3 tokens | `esInicioFuncion()` |

Conjuntos **PRIMERO** (FIRST) que usa el parser para decidir:

| No terminal | PRIMERO |
|---|---|
| `Programa`, `Funcion`, `TipoRetorno` | `int float char boolean void` |
| `Declaracion`, `Tipo` | `int float char boolean` |
| `Sentencia` | `int float char boolean` · `ID` · `return` · `{` · `if while for scanf println` |
| `Expresion` … `ExprUna` | `NUM_INT NUM_DEC TEXTO ID ( - !` |
| `Primario` | `NUM_INT NUM_DEC TEXTO ID (` |
| `DeclVar`, `Variable`, `Llamada` | `ID` |

Operadores `{ … }` de cada nivel: el parser continúa en el `while` mientras el token actual sea uno de **sus** operadores
(eso es el conjunto SIGUIENTE usado en la práctica).

## 5. Ejemplo de derivación

`x = a + 2 * b;` dentro de un bloque:

```
Sentencia → Asignacion → Variable '=' Expresion ';'
                         ID       =  ExprOr → … → ExprSuma
                                       ExprSuma → ExprMult '+' ExprMult
                                                  (a)          (2 '*' b)
```

Árbol que construye el parser (`output/arbol_sintactico.txt`):

```
Asignacion
|-- ID: x  <ID, 0>
`-- Op: +
    |-- ID: a  <ID, 1>
    `-- Op: *
        |-- NUM_INT: 2
        `-- ID: b  <ID, 2>
```

## 6. Decisiones a confirmar con el anexo oficial

| # | Decisión tomada | Si el anexo dice otra cosa, cambiar… |
|---|---|---|
| 1 | El programa es una **secuencia de funciones**; **no** hay declaraciones globales. | `programa()` y `funcion()` (agregar `Declaracion` global como alternativa en `programa()`, decidiendo con `esInicioFuncion()`). |
| 2 | `main` puede tener cualquier tipo de retorno y parámetros; se exige que exista. | `funcion()` (rama `MAIN`) y el chequeo al final de `programa()`. |
| 3 | Un parámetro puede ser arreglo (`int a[]`). | `parametro()`. |
| 4 | El tamaño de un arreglo es un **`NUM_INT` constante** (`int a[5]`). Sin inicialización de arreglos. | `declaracionVariable()`. |
| 5 | La asignación es **sentencia**, no expresión: `y = x = 3;` **no** se acepta. | `asignacionOLlamada()`; para encadenar, que `expresion()` acepte `Variable '=' Expresion`. |
| 6 | Se permite una **llamada como sentencia** (`saludar();`) y como factor (`suma(1,2)`). | `asignacionOLlamada()` y `primario()`. |
| 7 | `TEXTO` es un `Primario` válido en cualquier expresión. | `primario()`. |
| 8 | Todos los `COMP` están en **un solo nivel** (el léxico los entrega con el mismo token). | `exprComp()`. |
| 9 | Se permiten **bloques anidados** `{ ... }` como sentencia. | `sentencia()`. |
| 10 | `if / else / while / for / scanf / println` → **Entregable 2**: se reconocen y se omiten con aviso. | Ver `sentenciaPendiente()` (sección "Entregable 2" del parser). |

## 7. Recuperación de errores (modo pánico)

* **`;` faltante** → se informa **justo después del último token válido** (donde se olvidó) y, si lo que sigue es otra
  sentencia en otra línea (o un `}`), se continúa como si estuviera.
* **Otro error** → el parser lanza una excepción interna, salta tokens hasta un `;` (lo consume), un `}`, el inicio de una
  sentencia o el inicio de otra función, y sigue. Así se reportan **varios errores por corrida**.
* **Sin cascadas**: no se reportan dos errores en el mismo token, y el parser siempre avanza al menos un token (no hay ciclos infinitos).
* **`main` faltante** solo se informa si no hubo otros errores (si no, sería un error derivado).
