#!/bin/sh
# Pruebas automaticas del analizador lexico LP (Fase 3).
#
# Uso:
#   ./correr_pruebas.sh              compila (si hace falta) y compara cada prueba
#                                    contra su resultado esperado en tests/esperado/
#   ./correr_pruebas.sh --actualizar regenera los resultados esperados
#                                    (REVISARLOS a mano antes de hacer commit)
#
# Cada tests/<categoria>/<nombre>.lp  se compara con  tests/esperado/<nombre>.txt

make -s || exit 1

ACTUALIZAR=0
[ "$1" = "--actualizar" ] && ACTUALIZAR=1

total=0
ok=0
fallidas=""

for f in tests/validas/*.lp tests/invalidas/*.lp tests/limite/*.lp; do
    nombre=$(basename "$f" .lp)
    salida="output/pruebas/$nombre"
    esperado="tests/esperado/$nombre.txt"
    total=$((total + 1))

    ./lexlp "$f" --silencioso --salida "$salida" || { fallidas="$fallidas $nombre"; continue; }

    if [ $ACTUALIZAR -eq 1 ]; then
        cp "$salida/resultado.txt" "$esperado"
        ok=$((ok + 1))
        echo "[ACTUALIZADO] $f"
    elif [ -f "$esperado" ] && diff -q "$salida/resultado.txt" "$esperado" >/dev/null; then
        ok=$((ok + 1))
        echo "[OK]     $f"
    else
        fallidas="$fallidas $nombre"
        echo "[FALLO]  $f"
        [ -f "$esperado" ] && diff "$esperado" "$salida/resultado.txt" | head -10
    fi
done

echo "--------------------------------------------------"
echo "Pruebas: $ok / $total correctas"
if [ -n "$fallidas" ]; then
    echo "Fallidas:$fallidas"
    exit 1
fi
