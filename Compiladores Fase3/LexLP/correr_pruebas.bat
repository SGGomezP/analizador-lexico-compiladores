@echo off
rem Pruebas automaticas del analizador lexico LP (Fase 3) - Windows
rem Compilar antes (ver README):  g++ -std=c++17 -O2 -o lexlp.exe src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp
rem Uso: correr_pruebas.bat            compara contra tests\esperado\
rem      correr_pruebas.bat actualizar regenera los resultados esperados
setlocal enabledelayedexpansion
set TOTAL=0
set OK=0
set FALLAS=0
for %%D in (validas invalidas limite) do (
  for %%F in (tests\%%D\*.lp) do (
    set /a TOTAL+=1
    lexlp.exe "%%F" --silencioso --salida "output\pruebas\%%~nF" >nul
    if /i "%~1"=="actualizar" (
      copy /y "output\pruebas\%%~nF\resultado.txt" "tests\esperado\%%~nF.txt" >nul
      set /a OK+=1
      echo [ACTUALIZADO] %%F
    ) else (
      powershell -NoProfile -Command "if (Compare-Object (Get-Content \"output\pruebas\%%~nF\resultado.txt\") (Get-Content \"tests\esperado\%%~nF.txt\")) { exit 1 } else { exit 0 }"
      if !errorlevel! equ 0 (
        set /a OK+=1
        echo [OK]     %%F
      ) else (
        set /a FALLAS+=1
        echo [FALLO]  %%F
      )
    )
  )
)
echo --------------------------------------------------
echo Pruebas correctas: !OK! de !TOTAL!  (fallidas: !FALLAS!)
pause
