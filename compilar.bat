@echo off
REM Script de compilacion para Windows
REM Autor: Equipo ProyectoSO
REM Fecha: Noviembre 2025

echo ================================================
echo   Compilador Automatico - Simulador Bancario
echo ================================================
echo.

REM Verificar si g++ esta disponible
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: g++ no encontrado
    echo.
    echo Instala MinGW o MSYS2:
    echo   - MinGW: https://sourceforge.net/projects/mingw/
    echo   - MSYS2: https://www.msys2.org/
    echo.
    pause
    exit /b 1
)

echo Compilador encontrado: g++
g++ --version | findstr /C:"g++"

REM Limpiar compilacion anterior
echo.
echo Limpiando archivos anteriores...
if exist obj rmdir /s /q obj
if exist simulador.exe del simulador.exe

REM Crear directorio de objetos
mkdir obj

REM Compilar
echo.
echo Compilando el proyecto...
echo.

g++ -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/main.cpp -o obj/main.o
if %errorlevel% neq 0 goto error

g++ -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/productor_consumidor.cpp -o obj/productor_consumidor.o
if %errorlevel% neq 0 goto error

g++ -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/lectores_escritores.cpp -o obj/lectores_escritores.o
if %errorlevel% neq 0 goto error

g++ -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/monitor.cpp -o obj/monitor.o
if %errorlevel% neq 0 goto error

g++ -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/deadlock.cpp -o obj/deadlock.o
if %errorlevel% neq 0 goto error

REM Enlazar
echo.
echo Enlazando...
g++ -std=c++17 -pthread obj/*.o -o simulador.exe
if %errorlevel% neq 0 goto error

echo.
echo ================================================
echo   COMPILACION EXITOSA
echo ================================================
echo.
echo Ejecutable creado: simulador.exe
echo.
echo Para ejecutar:
echo   simulador.exe
echo.
echo Para ejecutar con configuracion personalizada:
echo   simulador.exe mi_config.json
echo.
pause
exit /b 0

:error
echo.
echo ================================================
echo   ERROR EN LA COMPILACION
echo ================================================
echo.
pause
exit /b 1
