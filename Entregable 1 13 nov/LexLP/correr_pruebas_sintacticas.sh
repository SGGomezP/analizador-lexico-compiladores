#!/bin/sh
# Pruebas automaticas del ANALIZADOR SINTACTICO de LP (Entregable 1).
#
# Uso:
#   ./correr_pruebas_sintacticas.sh              compila (si hace falta) y compara cada prueba
#                                                contra su resultado esperado en tests/sintactico/esperado/
#   ./correr_pruebas_sintacticas.sh --actualizar regenera los resultados esperados
#                                                (REVISARLOS a mano antes de hacer commit)
#
# Cada tests/sintactico/<validas|invalidas>/<nombre>.lp se compara con
# tests/sintactico/esperado/<nombre>.txt (resultado_sintactico.txt) y ademas se verifica el codigo de salida:
#   validas   -> 0  (lexico y sintactico correctos)
#   invalidas -> 3  (errores sintacticos)

make -s || exit 1

ACTUALIZAR=0
[ "$1" = "--actualizar" ] && ACTUALIZAR=1

total=0
ok=0
fallidas=""

for categoria in validas invalidas; do
    [ "$categoria" = "validas" ] && rc_esperado=0 || rc_esperado=3
    for f in tests/sintactico/$categoria/*.lp; do
        nombre=$(basename "$f" .lp)
        salida="output/pruebas_sintacticas/$nombre"
        esperado="tests/sintactico/esperado/$nombre.txt"
        total=$((total + 1))

        ./lexlp "$f" --silencioso --salida "$salida"
        rc=$?

        if [ $rc -ne $rc_esperado ]; then
            fallidas="$fallidas $nombre"
            echo "[FALLO]  $f (codigo de salida $rc, se esperaba $rc_esperado)"
            continue
        fi

        if [ $ACTUALIZAR -eq 1 ]; then
            cp "$salida/resultado_sintactico.txt" "$esperado"
            ok=$((ok + 1))
            echo "[ACTUALIZADO] $f"
        elif [ -f "$esperado" ] && diff -q "$salida/resultado_sintactico.txt" "$esperado" >/dev/null; then
            ok=$((ok + 1))
            echo "[OK]     $f"
        else
            fallidas="$fallidas $nombre"
            echo "[FALLO]  $f"
            [ -f "$esperado" ] && diff "$esperado" "$salida/resultado_sintactico.txt" | head -10
        fi
    done
done

echo "--------------------------------------------------"
echo "Pruebas sintacticas: $ok / $total correctas"
if [ -n "$fallidas" ]; then
    echo "Fallidas:$fallidas"
    exit 1
fi
