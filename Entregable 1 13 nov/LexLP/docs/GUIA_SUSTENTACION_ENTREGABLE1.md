# Guía de sustentación — Entregable 1 (Analizador sintáctico)

Todos los integrantes deben poder explicar **todo** el flujo y modificar/probar código en vivo.
No basta con que funcione: hay que **justificar las decisiones**.

## 1. Arquitectura en una imagen

```
 archivo.lp ──▶ Lexer ──▶ vector<Token> ──▶ Parser ──▶ árbol sintáctico (AST)
   (texto)     (Fase 3)   (sin COMENT,       (NUEVO)    + errores sintácticos
                           sin errores)                 + avisos
                  │                                │
                  ├─ tabla de símbolos             └─ main.cpp imprime y escribe los reportes
                  └─ errores léxicos
```

| Módulo | Responsabilidad |
|---|---|
| `Lexer`, `Token`, `SymbolTable` | **Sin cambios** respecto a la Fase 3: producen los tokens. |
| `Parser.h/.cpp` | **Nuevo.** Descenso recursivo: un método por no terminal de la gramática. |
| `AST.h/.cpp` | **Nuevo.** Nodo del árbol sintáctico y su dibujo en texto. |
| `main.cpp` | Orquesta: léxico → sintáctico → reportes. Opción `--solo-lexico` conserva el comportamiento anterior. |

## 2. Flujo del parser (explicable en 1 minuto)

1. `main` llama a `lexer.analizar()` y le pasa **la lista de tokens** a `Parser`.
2. El `Parser` descarta los `COMENT` (no forman parte de la gramática) y mantiene un índice `pos`.
3. `programa()` llama a `funcion()` mientras haya tokens. `funcion()` lee tipo, nombre, `(`, parámetros, `)` y llama a `bloque()`.
4. `bloque()` llama a `sentencia()` hasta ver `}`. `sentencia()` **mira el token actual** y decide:
   tipo → `declaracion()` · `ID` → `asignacionOLlamada()` · `return` → `retorno()` · `{` → `bloque()`.
5. Las expresiones se parsean por **niveles de precedencia**: `exprOr → exprAnd → exprComp → exprSuma → exprMult → exprUnaria → primario`.
6. Cada método devuelve un nodo del árbol; al terminar se imprime el árbol, la tabla de errores y el veredicto.

## 3. Preguntas probables (y respuesta corta)

* **¿Qué técnica usaron y por qué?** Descenso recursivo predictivo (LL(1)): un método por no terminal, se decide con un solo
  token de lookahead. Es fácil de escribir a mano, de depurar y de dar mensajes de error claros.
* **¿Por qué no hay retroceso (backtracking)?** Porque la gramática está factorizada: cada alternativa empieza con tokens distintos
  (tabla PRIMERO en `GRAMATICA_LP.md`). Solo `ID '('` (llamada) vs `ID` (variable) necesita mirar 2 tokens.
* **¿Cómo manejan la precedencia?** Un método por nivel; el que liga más fuerte está más "abajo" en la cadena de llamadas.
  `a + b * 3` → `exprSuma` pide `exprMult` para cada lado, así `b * 3` se agrupa primero.
* **¿Y la asociatividad?** Cada nivel usa un `while`, no recursión izquierda → `a - b - c` = `(a - b) - c`. La recursión por la izquierda
  haría que el parser se llamara a sí mismo infinitamente.
* **¿Por qué `COMP` es un solo nivel?** El léxico entrega `> >= < <= != ==` con el mismo token `COMP`; la gramática no los distingue.
* **¿Cómo se integra con el léxico?** El parser consume exactamente la `vector<Token>` que devuelve `Lexer::analizar()`. Los errores
  léxicos no llegan al parser (el léxico los registra aparte); si hay, el reporte avisa que pueden provocar errores sintácticos derivados.
* **¿Qué pasa con los comentarios?** El léxico los emite (`COMENT`); el parser los filtra en su constructor.
* **¿Cómo se recuperan de un error?** Modo pánico: tras un error salta hasta `;`, `}`, inicio de sentencia o inicio de función y sigue,
  para reportar **varios errores** en una corrida. El `;` faltante se reporta *después del último token válido* y se continúa como si existiera.
* **¿Cómo evitan errores en cascada / ciclos infinitos?** No se reportan 2 errores en el mismo token, y los bucles fuerzan avanzar ≥1 token.
  Se probó con 1500 programas aleatorios + AddressSanitizer/UBSan sin fallos.
* **¿Por qué `y = x = 3;` da error?** En la gramática implementada la asignación es una *sentencia*, no una expresión. (Ver decisión #5 de `GRAMATICA_LP.md`.)
* **¿Por qué `if/while/for/scanf/println` solo dan un aviso?** Son del Entregable 2. Se reconocen y se **omiten** (saltando paréntesis/llaves
  balanceadas) para poder analizar programas completos sin falsos errores; el aviso deja claro que no se verificó su sintaxis.
* **¿Qué devuelve el programa?** `0` correcto · `1` no abre el archivo · `2` solo errores léxicos · `3` errores sintácticos.
* **¿Qué es el árbol que imprime?** Un árbol sintáctico simplificado (los paréntesis y `;` no se guardan porque no aportan significado).
  Es la base para el análisis semántico y la generación de código de las próximas etapas.

## 4. Cambios en vivo que suelen pedir (practicar)

1. **Aceptar `y = x = 3;`** (asignación encadenada): en `asignacionOLlamada()`, después de `'='` llamar recursivamente a la misma lógica
   (o hacer que `expresion()` acepte `Variable '=' Expresion`).
2. **Agregar un operador** (p. ej. `**` o `++`): primero el léxico (`Token.h/.cpp`, `reconocerOperador()`), luego un nivel o caso en el parser.
3. **Permitir declaraciones globales**: en `programa()`, si `esInicioFuncion()` es falso y `esTipoDato(actual().tipo)` → `declaracion()`.
4. **Permitir `int a[] = ...` / tamaño con expresión**: `declaracionVariable()`.
5. **Aceptar `boolean` literales `true` / `false`**: agregarlos como palabras reservadas en el léxico y como `Primario`.
6. **Nuevo caso de prueba**: crear `tests/sintactico/<validas|invalidas>/x.lp`, ejecutar
   `./correr_pruebas_sintacticas.sh --actualizar`, **revisar** `tests/sintactico/esperado/x.txt` y hacer commit.
7. **Ver el árbol de un archivo**: `./lexlp archivo.lp --silencioso` y abrir `output/arbol_sintactico.txt`.

## 5. Demo sugerida (3 min)

1. `./lexlp tests/sintactico/validas/expresiones.lp` → mostrar tokens, árbol y la precedencia en el árbol.
2. `./lexlp tests/sintactico/invalidas/errores_multiples.lp` → varios errores con línea/columna y recuperación.
3. `./lexlp tests/sintactico/invalidas/errores_lexicos_y_sintacticos.lp` → léxico + sintáctico juntos.
4. `./correr_pruebas_sintacticas.sh` → 31/31 correctas.
