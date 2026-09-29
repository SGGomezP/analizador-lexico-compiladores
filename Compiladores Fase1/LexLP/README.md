# LexLP — Analizador Léxico para el lenguaje LP

Proyecto del curso de Compiladores. Analizador léxico en C++ para el
lenguaje simplificado **LP**, desarrollado de forma incremental en 3 fases
semanales.

## Estado actual: Fase 1 — Lectura + NUM_INT + NUM_DEC

Implementado en esta entrega:

- Lectura de un archivo fuente `.lp` carácter por carácter.
- Seguimiento de **línea** y **columna** para cada lexema reconocido.
- Reconocimiento de las expresiones regulares:
  - `NUM_INT = D+`
  - `NUM_DEC = D+\.D+` (con `D = [0-9]`)
- Descarte de espacios en blanco, tabulaciones y saltos de línea.
- Reporte de **errores léxicos** (carácter por carácter) para cualquier
  símbolo que aún no esté implementado (letras, operadores, símbolos
  especiales, etc. — se incorporan en las fases 2 y 3).
- Generación de:
  - `output/tokens.txt` — lista secuencial de tokens reconocidos.
  - `output/errores.txt` — errores léxicos con línea y columna.
  - `output/tabla_simbolos.txt` — placeholder (la tabla real se
    implementa en la Fase 2, junto con `ID`).

Pendiente para las próximas fases (ver sección 10 del enunciado):

- **Fase 2 (22/09):** `ID`, `TEXTO`, palabras reservadas y tabla de símbolos.
- **Fase 3 (29/09):** operadores, símbolos especiales, comentarios,
  integración completa, lista de tokens final y batería de pruebas.

## Estructura del proyecto

```
LexLP/
├── src/
│   ├── main.cpp        # punto de entrada
│   ├── Lexer.h/.cpp     # analizador léxico
│   └── Token.h/.cpp     # definición de Token y TokenType
├── tests/
│   ├── prueba_basica.lp     # números simples (enteros y decimales)
│   ├── prueba_completa.lp   # casos límite (10., 12.3.4, etc.)
│   └── prueba_errores.lp    # caracteres no reconocidos
├── output/               # se genera al ejecutar el programa
├── Makefile
└── README.md
```

## Compilación

Requiere un compilador con soporte C++17 (g++ o clang++).

```bash
make
```

Esto genera el ejecutable `lexlp` en la raíz del proyecto.

## Ejecución

```bash
./lexlp tests/prueba_basica.lp
```

Si no se indica un archivo, el programa usa `tests/prueba_basica.lp` por
defecto:

```bash
./lexlp
```

También se puede usar el atajo:

```bash
make run
```

## Ejemplo de salida (prueba_completa.lp)

Entrada:

```
123
456.789
0.5
10.
5 10.25 3
12.3.4
```

Salida esperada (resumen):

```
<NUM_INT>   lexema=123
<NUM_DEC>   lexema=456.789
<NUM_DEC>   lexema=0.5
<NUM_INT>   lexema=10
<NUM_INT>   lexema=5
<NUM_DEC>   lexema=10.25
<NUM_INT>   lexema=3
<NUM_DEC>   lexema=12.3
<NUM_INT>   lexema=4

Errores lexicos (2):
Linea 4, Columna 3: caracter no reconocido '.'
Linea 6, Columna 5: caracter no reconocido '.'
```

Nota: `10.` no cumple `D+\.D+` (no hay dígitos después del punto), por lo
que se reconoce `10` como `NUM_INT` y el `.` queda como error léxico. De
igual forma, en `12.3.4` el lexer reconoce `12.3` como `NUM_DEC` (coincidencia
más larga posible) y dado que no existe ningún token que empiece con `.`
en la especificación de LP, el segundo `.` se reporta como error y `4`
se reconoce como `NUM_INT`.
