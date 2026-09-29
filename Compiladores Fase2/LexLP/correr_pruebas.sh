#!/bin/sh
# Ejecuta el analizador sobre todos los archivos de tests/ (compila si hace falta).
make -s || exit 1
for f in tests/*.lp; do
    echo "=================================================="
    echo ">>> $f"
    ./lexlp "$f"
done
