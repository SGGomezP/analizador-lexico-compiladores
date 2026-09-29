@echo off
rem Ejecuta el analizador sobre todos los archivos de tests\ (compilar antes: ver README)
for %%f in (tests\*.lp) do (
    echo ==================================================
    echo ^>^>^> %%f
    lexlp.exe %%f
)
pause
