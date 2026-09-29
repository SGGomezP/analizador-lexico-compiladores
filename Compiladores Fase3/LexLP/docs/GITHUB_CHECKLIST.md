# Checklist del repositorio GitHub (3 pts + entrega)

**Entrega:** todos los integrantes suben el link del repositorio a la tarea de Classroom **hasta el lunes 28/09**.

## Estructura recomendada del repositorio

```
Compiladores/                      <- raíz del repo
├── README.md                      <- (opcional) resumen general + links a cada fase
├── Compiladores Fase1/LexLP/      <- intacto
├── Compiladores Fase2/LexLP/      <- intacto
└── Compiladores Fase3/LexLP/      <- versión final (esta carpeta)
    ├── src/  tests/  output/  docs/
    ├── Makefile  CMakeLists.txt  correr_pruebas.sh  correr_pruebas.bat
    ├── .gitignore
    └── README.md
```

## Qué debe verse en GitHub

| Criterio | Qué debe haber |
|----------|----------------|
| **Proyecto ejecutable** | `src/` completo, `Makefile` y `CMakeLists.txt`; compila con `make` sin errores ni warnings |
| **README con instrucciones** | integrantes, tokens, compilación, ejecución, pruebas, decisiones de diseño (ya está en `README.md`; **completar la tabla de integrantes**) |
| **Casos de prueba** | `tests/validas`, `tests/invalidas`, `tests/limite` + `tests/esperado` |
| **Lista de tokens / tabla de símbolos / errores** | ejemplos en `output/` (`tokens.txt`, `secuencia_tokens.txt`, `tabla_simbolos.txt`, `errores.txt`) |
| **Commits progresivos y de todo el grupo** | historial con varios commits, de **distintos integrantes** (ver plan abajo) |

## Antes de subir

* [ ] Completar la tabla **Integrantes** del README.
* [ ] `make clean && make && ./correr_pruebas.sh` → 31/31 correctas.
* [ ] No subir el binario (`lexlp`, `lexlp.exe`): ya está en `.gitignore`.
* [ ] Verificar que el repositorio sea **público** o que la profesora tenga acceso.
* [ ] Cada integrante ha subido el link a Classroom.

## Plan de commits sugerido para la Fase 3 (repartir entre integrantes)

Cada persona hace `git pull`, sus cambios/commits propios y `git push`:

```bash
git commit -m "Fase 3: agrega COMENT y categorias de token en Token.h/.cpp"
git commit -m "Fase 3: reconoce operadores aritmeticos, logicos y de asignacion"
git commit -m "Fase 3: agrega COMP (> >= < <= != ==) con atributo"
git commit -m "Fase 3: simbolos especiales y comentarios //"
git commit -m "Fase 3: error de '&' y '|' solitarios"
git commit -m "Fase 3: main con opciones --salida, --sin-comentarios y reportes"
git commit -m "Tests: casos validos, invalidos y limite"
git commit -m "Tests: script correr_pruebas y resultados esperados"
git commit -m "Docs: README y guia de sustentacion"
```

> Importante: los commits deben reflejar trabajo **real** de cada integrante. Repártanse estas tareas y hagan
> cada commit desde su propia cuenta; no fabriquen historial.
