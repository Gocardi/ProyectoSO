#!/bin/bash

# Script de compilación universal para Linux
# Autor: Equipo ProyectoSO
# Fecha: Noviembre 2025

echo "================================================"
echo "  Compilador Automático - Simulador Bancario"
echo "================================================"
echo ""

# Detectar el compilador
if command -v g++ &> /dev/null; then
    COMPILADOR="g++"
    echo "✓ Compilador encontrado: $(g++ --version | head -n1)"
elif command -v clang++ &> /dev/null; then
    COMPILADOR="clang++"
    echo "✓ Compilador encontrado: $(clang++ --version | head -n1)"
else
    echo "✗ Error: No se encontró g++ ni clang++"
    echo "  Instala con:"
    echo "    Arch:   sudo pacman -S gcc"
    echo "    Ubuntu: sudo apt install build-essential"
    exit 1
fi

# Verificar C++17
echo "✓ Verificando soporte de C++17..."
cat > /tmp/test_cpp17.cpp << 'EOF'
#include <shared_mutex>
#include <thread>
int main() { return 0; }
EOF

if $COMPILADOR -std=c++17 /tmp/test_cpp17.cpp -o /tmp/test_cpp17 2>/dev/null; then
    echo "✓ C++17 soportado"
    rm -f /tmp/test_cpp17 /tmp/test_cpp17.cpp
else
    echo "✗ Error: C++17 no soportado"
    echo "  Actualiza tu compilador a GCC 7+ o Clang 5+"
    exit 1
fi

# Limpiar compilación anterior
echo ""
echo "Limpiando archivos anteriores..."
rm -rf obj simulador

# Crear directorio de objetos
mkdir -p obj

# Compilar
echo ""
echo "Compilando el proyecto..."
echo ""

$COMPILADOR -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/main.cpp -o obj/main.o
if [ $? -ne 0 ]; then echo "Error compilando main.cpp"; exit 1; fi

$COMPILADOR -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/productor_consumidor.cpp -o obj/productor_consumidor.o
if [ $? -ne 0 ]; then echo "Error compilando productor_consumidor.cpp"; exit 1; fi

$COMPILADOR -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/lectores_escritores.cpp -o obj/lectores_escritores.o
if [ $? -ne 0 ]; then echo "Error compilando lectores_escritores.cpp"; exit 1; fi

$COMPILADOR -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/monitor.cpp -o obj/monitor.o
if [ $? -ne 0 ]; then echo "Error compilando monitor.cpp"; exit 1; fi

$COMPILADOR -std=c++17 -Wall -Wextra -pthread -O2 -I./include -c src/deadlock.cpp -o obj/deadlock.o
if [ $? -ne 0 ]; then echo "Error compilando deadlock.cpp"; exit 1; fi

# Enlazar
echo ""
echo "Enlazando..."
$COMPILADOR -std=c++17 -pthread obj/*.o -o simulador
if [ $? -ne 0 ]; then echo "Error en el enlazado"; exit 1; fi

echo ""
echo "================================================"
echo "  ✓ COMPILACIÓN EXITOSA"
echo "================================================"
echo ""
echo "Ejecutable creado: ./simulador"
echo ""
echo "Para ejecutar:"
echo "  ./simulador"
echo ""
echo "Para ejecutar con configuración personalizada:"
echo "  ./simulador mi_config.json"
echo ""
