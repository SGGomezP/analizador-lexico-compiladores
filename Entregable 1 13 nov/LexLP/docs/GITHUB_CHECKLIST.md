# Checklist del repositorio GitHub — Entregable 1 (Analizador sintáctico)

**Entrega:** viernes **13 de noviembre**. Todos los integrantes deben haber participado (historial de commits de cada uno).

## Estructura recomendada del repositorio

```
Compiladores/                      <- raíz del repo
├── README.md                      <- (opcional) resumen general + links a cada etapa
├── Compiladores Fase1/LexLP/      <- intacto
├── Compiladores Fase2/LexLP/      <- intacto
├── Compiladores Fase3/LexLP/      <- intacto (léxico final)
└── Entregable 1 13 nov/LexLP/     <- léxico integrado + analizador sintáctico (esta carpeta)
    ├── src/  tests/  output/  docs/
    ├── Makefile  CMakeLists.txt
    ├── correr_pruebas.sh/.bat  correr_pruebas_sintacticas.sh/.bat
    ├── .gitignore
    └── README.md
```

## Qué debe verse en GitHub

| Criterio | Qué debe haber |
|----------|----------------|
| **Proyecto ejecutable** | `src/` completo (incluye `Parser.*` y `AST.*`), `Makefile` y `CMakeLists.txt`; compila con `make` sin errores ni warnings |
| **Integración con el léxico** | `main.cpp` llama a `Lexer` y pasa sus tokens a `Parser` |
| **Código modular** | léxico, tokens, tabla de símbolos, parser y árbol en archivos separados |
| **README con instrucciones** | integrantes, alcance, compilación, ejecución, pruebas, decisiones de diseño (**completar la tabla de integrantes**) |
| **Gramática documentada** | `docs/GRAMATICA_LP.md` **verificada contra el anexo oficial** |
| **Casos de prueba** | `tests/sintactico/validas`, `tests/sintactico/invalidas` + `tests/sintactico/esperado` |
| **Salidas de ejemplo** | `output/` (`arbol_sintactico.txt`, `errores_sintacticos.txt`, tokens, tabla de símbolos…) |
| **Commits progresivos y de todo el grupo** | historial con varios commits, de **distintos integrantes** |

## Antes de subir

* [ ] Completar la tabla **Integrantes** del README.
* [ ] **Comparar `docs/GRAMATICA_LP.md` con el anexo oficial** y ajustar el parser donde difiera (tabla "Decisiones a confirmar").
* [ ] `make clean && make && make test` → léxico 31/31 y sintáctico 31/31.
* [ ] No subir el binario (`lexlp`, `lexlp.exe`): ya está en `.gitignore`.
* [ ] Verificar que el repositorio sea **público** o que la profesora tenga acceso.
* [ ] Cada integrante puede explicar **todo** el código (ver `docs/GUIA_SUSTENTACION_ENTREGABLE1.md`).

## Plan de commits sugerido (repartir entre integrantes)

Cada persona hace `git pull`, sus cambios/commits propios y `git push`:

```bash
git commit -m "Entregable 1: agrega AST (nodo del arbol sintactico y dibujo en texto)"
git commit -m "Entregable 1: Parser - estructura del programa (funciones, parametros, bloques)"
git commit -m "Entregable 1: Parser - declaraciones (tipos, arreglos, inicializacion)"
git commit -m "Entregable 1: Parser - asignaciones y return"
git commit -m "Entregable 1: Parser - expresiones con precedencia y llamadas"
git commit -m "Entregable 1: Parser - recuperacion de errores (modo panico) y mensajes"
git commit -m "Entregable 1: integra Lexer + Parser en main (reportes, --solo-lexico, codigos de salida)"
git commit -m "Tests: casos sintacticos validos e invalidos + script correr_pruebas_sintacticas"
git commit -m "Docs: gramatica, README y guia de sustentacion del Entregable 1"
```

> Importante: los commits deben reflejar trabajo **real** de cada integrante. Repártanse estas tareas y hagan
> cada commit desde su propia cuenta; no fabriquen historial.
