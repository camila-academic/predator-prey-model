@echo off
gcc -Wall -Wextra -pedantic -std=c11 -Iinclude src\main.c src\utils.c src\entities.c src\board.c src\simulation.c -o ecosystem.exe
if errorlevel 1 (
  echo Erro ao compilar.
  exit /b 1
)
echo Compilado com sucesso: ecosystem.exe
