# Makefile para Simulador Bancario Concurrente
# Alternativa a CMake para compilación directa

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -O2
INCLUDES = -I./include

# Archivos fuente y objeto
SRCDIR = src
INCDIR = include
OBJDIR = obj
TARGET = simulador

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Regla principal
all: $(TARGET)

# Crear directorio de objetos si no existe
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Enlazar objetos para crear el ejecutable
$(TARGET): $(OBJDIR) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)
	@echo "Compilación exitosa: $(TARGET)"
	@echo "Ejecuta con: ./$(TARGET)"

# Compilar archivos fuente a objetos
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -rf $(OBJDIR) $(TARGET)
	@echo "Archivos limpiados"

# Ejecutar el programa
run: $(TARGET)
	./$(TARGET)

# Ayuda
help:
	@echo "Makefile para Simulador Bancario Concurrente"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  make         - Compila el proyecto"
	@echo "  make run     - Compila y ejecuta"
	@echo "  make clean   - Limpia archivos compilados"
	@echo "  make help    - Muestra esta ayuda"

.PHONY: all clean run help
