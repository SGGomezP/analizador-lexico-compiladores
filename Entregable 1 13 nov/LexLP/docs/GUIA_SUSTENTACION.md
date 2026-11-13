# Guía de sustentación y presentación — Fase 3

Todos los integrantes deben poder explicar **todo** el flujo y modificar/probar código en vivo (4 pts).

## 1. Esquema de la presentación (≈ 8–10 min)

| # | Diapositiva | Qué decir |
|---|-------------|-----------|
| 1 | Portada | Proyecto, curso, integrantes |
| 2 | Objetivo | Analizador léxico de LP en C++: tokens + tabla de símbolos + errores |
| 3 | Fases 1 → 2 → 3 | Qué se agregó en cada una (tabla del README) |
| 4 | Tokens de LP | La tabla de expresiones regulares (D, L, NUM_INT … COMP, símbolos) |
| 5 | Arquitectura | `main → Lexer → Token / SymbolTable` (un diagrama de 4 cajas) |
| 6 | Flujo del analizador | `analizar()` → `siguienteToken()` → según el 1.er carácter: número / ID / texto / comentario / operador |
| 7 | Decisiones de la Fase 3 | Coincidencia más larga, `&` y `\|` solos = error, `COMENT`, `COMP` con atributo |
| 8 | Demo en vivo | Correr `prueba_programa_completo.lp` y `prueba_errores_mezclados.lp` |
| 9 | Pruebas | 31 pruebas: 13 válidas, 7 inválidas, 11 límite; correr `correr_pruebas` |
| 10 | Conclusiones | Qué aprendimos / qué sigue (analizador sintáctico) |

## 2. Flujo del analizador (para explicar en 1 minuto)

1. `main.cpp` lee el archivo `.lp` completo en un `std::string` y crea el `Lexer`.
2. `Lexer::analizar()` reinicia el estado y llama a `siguienteToken()` hasta llegar al fin del archivo.
3. `siguienteToken()` salta espacios y mira **solo el primer carácter** para decidir:
   * dígito → `reconocerNumero()` → `NUM_INT` o `NUM_DEC` (necesita `.` + dígito para ser decimal);
   * letra o `_` → `reconocerIdentificador()` → lee `L(L|D)*` completo; si está en la tabla de palabras
     reservadas devuelve ese token, si no lo inserta en la `SymbolTable` y devuelve `<ID, pos>`;
   * `"` → `reconocerTexto()`;
   * `//` → `reconocerComentario()` (se prueba **antes** que `/`);
   * cualquier otro → `reconocerOperador()`: `switch` con un carácter de *lookahead* para `== != <= >= && ||`;
     si no coincide nada → error léxico.
4. Los errores se guardan aparte (línea, columna, motivo) y **no** cortan el análisis.
5. `main.cpp` imprime y escribe `tokens.txt`, `secuencia_tokens.txt`, `tabla_simbolos.txt`, `errores.txt`.

## 3. Preguntas probables (y respuesta corta)

* **¿Por qué `>=` es un token y no `>` + `=`?** Regla de la coincidencia más larga: se mira el siguiente carácter antes de decidir.
* **¿Por qué `a & b` da error?** LP solo define `&&` y `||`; un `&` o `|` solo no pertenece a ningún token.
* **¿Las palabras reservadas van en la tabla de símbolos?** No, solo los `ID`. Se distinguen después de leer el lexema completo.
* **¿Qué pasa con `ifelse` o `int2`?** Son `ID` (el lexema completo no es una palabra reservada).
* **¿`Int` es `INT`?** No, el lenguaje es sensible a mayúsculas: `Int` es `ID`.
* **¿Cómo evitan duplicados en la tabla?** `SymbolTable::insertar` busca primero en un `unordered_map`; si existe devuelve su posición.
* **¿Cómo se recupera de un error?** Consume un carácter, lo registra y sigue. Un texto sin cerrar se reporta una vez hasta fin de línea.
* **¿`//` dentro de un texto?** Es parte del `TEXTO`: el lexer entra por `"` y no vuelve a mirar `//` hasta cerrarlo.
* **¿Qué atributo llevan `ID` y `COMP`?** `ID`: posición en la tabla. `COMP`: el operador concreto (`<COMP, >=>`).
* **¿Y los números negativos?** `-5` es `-` + `NUM_INT`; el signo lo resolvería el analizador sintáctico.
* **¿Qué pasa con `12.3.4`?** Es **un solo error** (número decimal inválido: más de un punto). No se acepta `12.3` y se deja `.4` suelto: se consume todo el bloque de dígitos y puntos y se reporta completo.
* **¿Y `10.` o `.5`?** También un solo error: `NUM_DEC = D+\.D+` exige dígitos a ambos lados del punto.
* **¿Qué pasa con las comillas sin cerrar?** Error `texto sin cerrar (falta la comilla de cierre)`, una sola vez, hasta el fin de la línea; el análisis continúa.
* **¿Cómo informan los errores?** Tabla (N.º, línea, columna, lexema, descripción) y un veredicto final: `ANALISIS LEXICO INCORRECTO: se encontraron N errores lexicos.`

## 4. Cambios en vivo que suelen pedir (practicar)

1. **Agregar un operador nuevo** (p. ej. `++`): `Token.h` (enum) → `Token.cpp` (nombre y categoría) → `reconocerOperador()` (caso `'+'` con lookahead).
2. **Agregar una palabra reservada** (p. ej. `do`): `Token.h` + `Token.cpp` + mapa en `palabrasReservadas()`.
3. **Ocultar comentarios**: ejecutar con `--sin-comentarios` o cambiar `incluirComentarios` a `false`.
4. **Nuevo caso de prueba**: crear `tests/<categoria>/x.lp`, ejecutar `./correr_pruebas.sh --actualizar`, revisar `tests/esperado/x.txt`.
