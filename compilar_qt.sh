#!/bin/bash

echo "🔨 Compilando Sistema Bancario con Qt + JSON..."
echo ""

# Verificar que qmake esté instalado
if ! command -v qmake &> /dev/null; then
    echo "❌ Error: qmake no está instalado"
    echo "En Arch Linux, instala con: sudo pacman -S qt6-base"
    exit 1
fi

# Limpiar compilación anterior
echo "🧹 Limpiando archivos anteriores..."
make clean 2>/dev/null
rm -rf obj moc ui Makefile
rm -f simulador_qt

# Generar Makefile con qmake
echo "📝 Generando Makefile..."
qmake simulador_bancario.pro

if [ $? -ne 0 ]; then
    echo "❌ Error al generar Makefile"
    exit 1
fi

# Compilar
echo "⚙️  Compilando..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ ¡Compilación exitosa!"
    echo ""
    echo "Para ejecutar:"
    echo "  ./simulador_qt"
    echo ""
else
    echo ""
    echo "❌ Error en la compilación"
    exit 1
fi
