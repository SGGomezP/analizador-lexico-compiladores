@echo off
rem Pruebas automaticas del analizador SINTACTICO de LP (Entregable 1) - Windows
rem Compilar antes (ver README):  g++ -std=c++17 -O2 -o lexlp.exe src/main.cpp src/Lexer.cpp src/Token.cpp src/SymbolTable.cpp src/Parser.cpp src/AST.cpp
rem Uso: correr_pruebas_sintacticas.bat            compara contra tests\sintactico\esperado\
rem      correr_pruebas_sintacticas.bat actualizar regenera los resultados esperados
setlocal enabledelayedexpansion
set TOTAL=0
set OK=0
set FALLAS=0
for %%D in (validas invalidas) do (
  if "%%D"=="validas" (set RCESP=0) else (set RCESP=3)
  for %%F in (tests\sintactico\%%D\*.lp) do (
    set /a TOTAL+=1
    lexlp.exe "%%F" --silencioso --salida "output\pruebas_sintacticas\%%~nF" >nul
    set RC=!errorlevel!
    if /i "%~1"=="actualizar" (
      copy /y "output\pruebas_sintacticas\%%~nF\resultado_sintactico.txt" "tests\sintactico\esperado\%%~nF.txt" >nul
      set /a OK+=1
      echo [ACTUALIZADO] %%F
    ) else if not "!RC!"=="!RCESP!" (
      set /a FALLAS+=1
      echo [FALLO]  %%F ^(codigo de salida !RC!, se esperaba !RCESP!^)
    ) else (
      powershell -NoProfile -Command "if (Compare-Object (Get-Content \"output\pruebas_sintacticas\%%~nF\resultado_sintactico.txt\") (Get-Content \"tests\sintactico\esperado\%%~nF.txt\")) { exit 1 } else { exit 0 }"
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
echo Pruebas sintacticas correctas: !OK! de !TOTAL!  ^(fallidas: !FALLAS!^)
pause
