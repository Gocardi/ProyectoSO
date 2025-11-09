# 🚀 GUÍA DE COMPILACIÓN Y EJECUCIÓN

Este documento proporciona instrucciones detalladas para compilar y ejecutar el proyecto en diferentes sistemas operativos.

---

## 📋 Tabla de Contenidos
1. [Linux (Arch, Ubuntu, Debian)](#linux)
2. [Windows (MinGW/MSYS2)](#windows)
3. [Solución de Problemas](#solución-de-problemas)

---

## 🐧 LINUX (Arch, Ubuntu, Debian)

### Arch Linux

#### 1. Instalar dependencias
```bash
sudo pacman -S base-devel gcc
```

#### 2. Compilar el proyecto
```bash
cd /home/gocardi/ProyectoSO
make clean
make
```

#### 3. Ejecutar
```bash
./simulador
```

#### 4. Compilación manual (alternativa)
```bash
g++ -std=c++17 -pthread -Wall -Wextra -O2 -I./include \
    src/main.cpp \
    src/productor_consumidor.cpp \
    src/lectores_escritores.cpp \
    src/monitor.cpp \
    src/deadlock.cpp \
    -o simulador
```

---

### Ubuntu/Debian

#### 1. Instalar dependencias
```bash
sudo apt update
sudo apt install build-essential g++ make
```

#### 2. Compilar el proyecto
```bash
cd ~/ProyectoSO
make clean
make
```

#### 3. Ejecutar
```bash
./simulador
```

---

## 🪟 WINDOWS

### Método 1: MSYS2 (Recomendado)

#### 1. Instalar MSYS2
- Descargar desde: https://www.msys2.org/
- Instalar en `C:\msys64`

#### 2. Abrir MSYS2 MinGW 64-bit

#### 3. Instalar herramientas de compilación
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
```

#### 4. Navegar al proyecto
```bash
cd /c/Users/TU_USUARIO/ProyectoSO
```

#### 5. Compilar
```bash
mingw32-make clean
mingw32-make
```

#### 6. Ejecutar
```bash
./simulador.exe
```

---

### Método 2: MinGW (Sin MSYS2)

#### 1. Instalar MinGW
- Descargar desde: https://sourceforge.net/projects/mingw/
- Agregar `C:\MinGW\bin` al PATH

#### 2. Abrir CMD o PowerShell

#### 3. Compilar manualmente
```cmd
cd C:\Users\TU_USUARIO\ProyectoSO

g++ -std=c++17 -pthread -Wall -Wextra -O2 -I./include ^
    src/main.cpp ^
    src/productor_consumidor.cpp ^
    src/lectores_escritores.cpp ^
    src/monitor.cpp ^
    src/deadlock.cpp ^
    -o simulador.exe
```

#### 4. Ejecutar
```cmd
simulador.exe
```

---

### Método 3: Visual Studio (Windows nativo)

#### 1. Abrir Visual Studio Developer Command Prompt

#### 2. Compilar
```cmd
cl /EHsc /std:c++17 /I.\include ^
   src\*.cpp ^
   /Fe:simulador.exe
```

#### 3. Ejecutar
```cmd
simulador.exe
```

---

## 📝 Makefile para Windows

Si estás en Windows con MSYS2, el Makefile existente debería funcionar. Si no, crea un archivo `Makefile.win`:

```makefile
# Makefile para Windows
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -O2
INCLUDES = -I./include

SRCDIR = src
OBJDIR = obj
TARGET = simulador.exe

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	if not exist $(OBJDIR) mkdir $(OBJDIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)
	@echo Compilacion exitosa: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	if exist $(OBJDIR) rmdir /s /q $(OBJDIR)
	if exist $(TARGET) del $(TARGET)

run: $(TARGET)
	$(TARGET)

.PHONY: all clean run
```

Luego compila con:
```cmd
mingw32-make -f Makefile.win
```

---

## ⚙️ Configuración del Proyecto

El archivo `config.json` permite personalizar la simulación:

```json
{
  "capacidad_cola": 10,
  "num_clientes": 2,
  "num_motores": 3,
  "num_analistas": 3,
  "num_administradores": 1,
  "duracion_segundos": 30,
  "demo_monitor": true,
  "demo_deadlock": true
}
```

### Parámetros:
- **capacidad_cola**: Tamaño máximo del buffer de transacciones
- **num_clientes**: Cantidad de productores (clientes que generan transacciones)
- **num_motores**: Cantidad de consumidores (motores antifraude)
- **num_analistas**: Cantidad de lectores (analistas financieros)
- **num_administradores**: Cantidad de escritores (administradores)
- **duracion_segundos**: Duración de la simulación principal
- **demo_monitor**: Ejecutar demostración del monitor de cuentas
- **demo_deadlock**: Ejecutar demostración de deadlock

---

## 🔧 Solución de Problemas

### Error: "pthread: No such file or directory"
**Solución**: Asegúrate de usar el flag `-pthread` al compilar.

### Error: "std::shared_mutex: No such file or directory"
**Solución**: Tu compilador no soporta C++17. Actualiza GCC a versión 7.0 o superior:
```bash
# Arch
sudo pacman -S gcc

# Ubuntu
sudo apt install g++-9
export CXX=g++-9
```

### Error: Variables no utilizadas en deadlock.cpp
**Solución**: Son solo warnings, no errores. Puedes ignorarlos o compilar sin `-Wextra`.

### Windows: "g++ no reconocido"
**Solución**: 
1. Verifica que MinGW/MSYS2 esté instalado
2. Agrega al PATH:
   - MinGW: `C:\MinGW\bin`
   - MSYS2: `C:\msys64\mingw64\bin`

### El programa se congela
**Solución**: Si ejecutaste la demo de deadlock con `provocar_deadlock()`, es normal. Usa `Ctrl+C` para terminar. La demo `resolver_deadlock()` funciona correctamente.

---

## 📊 Verificación de la Compilación

Después de compilar exitosamente, deberías ver:

### Linux:
```
$ ls -lh simulador
-rwxr-xr-x 1 user user 250K nov 9 12:00 simulador
```

### Windows:
```
> dir simulador.exe
09/11/2025  12:00           256,000 simulador.exe
```

### Ejecución de prueba:
```
./simulador    # Linux
simulador.exe  # Windows
```

Deberías ver la salida:
```
================================================
  SIMULADOR DE SISTEMA BANCARIO CONCURRENTE
  Gestión de Transacciones con Concurrencia
================================================

========== CONFIGURACIÓN ==========
Capacidad de cola: 10
...
```

---

## 🎯 Comandos Rápidos por Sistema

### Arch Linux
```bash
cd ~/ProyectoSO
make clean && make && ./simulador
```

### Ubuntu/Debian
```bash
cd ~/ProyectoSO
make clean && make && ./simulador
```

### Windows (MSYS2)
```bash
cd /c/Users/TU_USUARIO/ProyectoSO
mingw32-make clean && mingw32-make && ./simulador.exe
```

### Windows (CMD)
```cmd
cd C:\Users\TU_USUARIO\ProyectoSO
make clean & make & simulador.exe
```

---

## 📚 Estructura de Archivos

```
ProyectoSO/
├── include/              # Archivos de cabecera (.hpp)
│   ├── modelos.hpp
│   ├── productor_consumidor.hpp
│   ├── lectores_escritores.hpp
│   ├── monitor.hpp
│   ├── deadlock.hpp
│   └── semaforo.hpp
├── src/                  # Código fuente (.cpp)
│   ├── main.cpp
│   ├── productor_consumidor.cpp
│   ├── lectores_escritores.cpp
│   ├── monitor.cpp
│   └── deadlock.cpp
├── config.json           # Configuración
├── Makefile              # Compilación Linux
├── README.md             # Documentación general
└── COMPILACION.md        # Este archivo
```

---

## 🆘 Soporte Adicional

Si encuentras problemas no listados aquí:

1. **Verifica la versión del compilador**:
   ```bash
   g++ --version  # Debe ser 7.0 o superior
   ```

2. **Verifica soporte de C++17**:
   ```bash
   echo "#include <shared_mutex>" | g++ -std=c++17 -x c++ - -c -o /dev/null
   ```

3. **Compila en modo debug para más información**:
   ```bash
   make clean
   make CXXFLAGS="-std=c++17 -Wall -Wextra -pthread -g"
   ```

---

**Última actualización**: 9 de noviembre de 2025
