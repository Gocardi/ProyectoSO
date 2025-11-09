#!/bin/bash

# Script de instalación y compilación para el Simulador Bancario

echo "================================================"
echo "  Instalación del Simulador Bancario"
echo "================================================"
echo ""

# Verificar si g++ está instalado
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ no está instalado."
    echo "Instala con: sudo apt install build-essential"
    exit 1
fi

# Verificar versión de g++
GCC_VERSION=$(g++ --version | head -n1)
echo "✓ Compilador encontrado: $GCC_VERSION"

# Verificar soporte de C++17
echo "✓ Verificando soporte de C++17..."
cat > /tmp/test_cpp17.cpp << 'EOF'
#include <shared_mutex>
int main() {
    std::shared_mutex mtx;
    return 0;
}
EOF

if g++ -std=c++17 /tmp/test_cpp17.cpp -o /tmp/test_cpp17 2>/dev/null; then
    echo "✓ C++17 soportado"
    rm -f /tmp/test_cpp17 /tmp/test_cpp17.cpp
else
    echo "✗ Error: C++17 no soportado"
    exit 1
fi

# Descargar nlohmann/json si no existe
NLOHMANN_JSON="include/nlohmann/json.hpp"
if [ ! -f "$NLOHMANN_JSON" ]; then
    echo ""
    echo "Descargando nlohmann/json..."
    mkdir -p include/nlohmann
    curl -s -L https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp \
         -o "$NLOHMANN_JSON"
    
    if [ -f "$NLOHMANN_JSON" ]; then
        echo "✓ nlohmann/json descargado"
    else
        echo "✗ Error al descargar nlohmann/json"
        echo "Por favor, descárgalo manualmente desde:"
        echo "https://github.com/nlohmann/json/releases"
        exit 1
    fi
else
    echo "✓ nlohmann/json ya existe"
fi

echo ""
echo "================================================"
echo "  Compilando el proyecto..."
echo "================================================"
echo ""

# Opción de compilación
echo "Elige el método de compilación:"
echo "1) Makefile (rápido, simple)"
echo "2) CMake (recomendado)"
read -p "Opción [1]: " OPCION
OPCION=${OPCION:-1}

if [ "$OPCION" == "2" ]; then
    # Verificar si CMake está instalado
    if ! command -v cmake &> /dev/null; then
        echo "Error: CMake no está instalado."
        echo "Instala con: sudo apt install cmake"
        exit 1
    fi
    
    echo "Compilando con CMake..."
    mkdir -p build
    cd build
    cmake ..
    make
    cd ..
    
    echo ""
    echo "✓ Compilación exitosa"
    echo "Ejecuta con: ./build/simulador"
    
else
    # Compilación con Makefile
    echo "Compilando con Makefile..."
    make clean
    make
    
    echo ""
    echo "✓ Compilación exitosa"
    echo "Ejecuta con: ./simulador"
fi

echo ""
echo "================================================"
echo "  Instalación completada"
echo "================================================"
echo ""
echo "Configuración:"
echo "  Edita config.json para ajustar parámetros"
echo ""
echo "Documentación:"
echo "  Ver README_PROYECTO.md para más información"
echo ""
