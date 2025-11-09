# Sistema Bancario Concurrente

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Qt](https://img.shields.io/badge/Qt-5%2F6-green.svg)](https://www.qt.io/)
[![License](https://img.shields.io/badge/License-Academic-yellow.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-lightgrey.svg)](README.md)

**Simulador de conceptos de Sistemas Operativos con interfaz gráfica Qt y persistencia JSON**

Proyecto académico que implementa patrones de concurrencia fundamentales (Productor-Consumidor, Lectores-Escritores, Semáforos, Monitores, Deadlocks) en un sistema bancario simulado.

---

## Tabla de Contenidos

- [Características](#características)
- [Conceptos de SO Implementados](#conceptos-de-so-implementados)
- [Instalación](#instalación)
  - [Arch Linux](#arch-linux)
  - [Ubuntu/Debian](#ubuntudebian)
  - [Windows 10/11](#windows-1011)
- [Compilación](#compilación)
- [Modos de Ejecución](#modos-de-ejecución)
- [Interfaz Gráfica Qt](#interfaz-gráfica-qt)
- [Persistencia de Datos](#persistencia-de-datos)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Limpieza de Archivos](#limpieza-de-archivos)
- [Documentación LaTeX](#documentación-latex)

---

## Características

### 4 Modos de Ejecución

1. **GUI Qt** - Interfaz gráfica completa con widgets nativos (RECOMENDADO)
2. **CLI Interactiva** - Control manual mediante menú de texto
3. **GUI ASCII** - Interfaz visual en terminal con colores ANSI
4. **CLI Automática** - Simulación automática de 30 segundos

### Características Técnicas

- Concurrencia real con `std::thread` de C++17
- Sincronización thread-safe usando mutex, condition_variable, shared_mutex
- Persistencia local con archivos JSON (sin dependencias externas)
- Detección de fraude en transacciones sospechosas
- Demostraciones interactivas de conceptos de SO
- Multiplataforma (Linux y Windows)

---

## Conceptos de SO Implementados

### 1. Patrón Productor-Consumidor

**Clase**: `ColaTransacciones`

- Buffer limitado de 10 elementos (std::queue)
- Bloqueo de productores cuando la cola está llena
- Bloqueo de consumidores cuando la cola está vacía
- Cierre ordenado con desbloqueo de hilos

**Primitivas**: `std::mutex`, `std::condition_variable`, `std::atomic<bool>`

```cpp
void producir(Transaccion t) {
    unique_lock<mutex> lock(mtx);
    cv_productor.wait(lock, [this] { 
        return cola.size() < capacidad_maxima; 
    });
    cola.push(t);
    cv_consumidor.notify_one();
}
```

### 2. Patrón Lectores-Escritores

**Clase**: `ConfiguracionSistema`

- Lecturas concurrentes - Múltiples hilos leen simultáneamente
- Escritura exclusiva - Solo un escritor, sin lectores
- Prioridad de escritores para evitar inanición

**Primitivas**: `std::shared_mutex` (C++17)

```cpp
// Lectura compartida
int leer(const string& clave) const {
    shared_lock<shared_mutex> lock(mtx);
    return configuraciones.at(clave);
}

// Escritura exclusiva
void escribir(const string& clave, int valor) {
    unique_lock<shared_mutex> lock(mtx);
    configuraciones[clave] = valor;
}
```

### 3. Semáforo Contador

**Clase**: `Semaforo` (implementación C++17 compatible)

- Limita recursos concurrentes (máximo 3 motores antifraude)
- `acquire()` - Decrementa contador, bloquea si es 0
- `release()` - Incrementa contador, despierta hilos
- `try_acquire()` - Intento no bloqueante

### 4. Monitor (Patrón Monitor)

**Clase**: `MonitorCuentas`

- Exclusión mutua automática en todos los métodos
- Condiciones de espera para fondos suficientes
- Transferencias atómicas entre cuentas
- Thread-safe sin race conditions

```cpp
bool transferir(string origen, string destino, double monto) {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [&] { return saldos[origen] >= monto; });
    saldos[origen] -= monto;
    saldos[destino] += monto;
    cv.notify_all();
    return true;
}
```

### 5. Deadlock (Interbloqueo)

- **Provocar deadlock** - Ciclo de espera entre 2 hilos
- **Resolver deadlock** - Uso de `std::scoped_lock`
- **Condiciones de Coffman** demostradas visualmente
- **Explicación paso a paso** en log con colores

---

## Instalación

### Arch Linux

#### Paso 1: Instalar Dependencias

```bash
# Actualizar sistema
sudo pacman -Syu

# Instalar compilador C++, Qt6 y herramientas de compilación
sudo pacman -S base-devel gcc qt6-base qt6-tools make
```

#### Paso 2: Verificar Instalación

```bash
# Verificar versión de g++
g++ --version
# Debe mostrar: g++ (GCC) 13.x.x o superior

# Verificar Qt
qmake --version
# Debe mostrar: QMake version 3.1, Using Qt version 6.x.x

# Verificar make
make --version
# Debe mostrar: GNU Make 4.x
```

#### Paso 3: Clonar Proyecto

```bash
# Clonar repositorio
git clone https://github.com/Gocardi/ProyectoSO.git
cd ProyectoSO

# Verificar estructura
ls -la
# Debe mostrar: include/ src/ config.json README.md
```

---

### Ubuntu/Debian

#### Paso 1: Instalar Dependencias

```bash
# Actualizar repositorios
sudo apt update

# Instalar herramientas de compilación
sudo apt install build-essential

# Instalar Qt6 (Ubuntu 22.04+)
sudo apt install qt6-base-dev qt6-tools-dev qt6-base-dev-tools

# Si Qt6 no está disponible, usar Qt5
sudo apt install qtbase5-dev qttools5-dev qttools5-dev-tools
```

#### Paso 2: Verificar Instalación

```bash
# Verificar g++
g++ --version

# Verificar qmake
qmake --version

# Si usas Qt5, puede ser:
qmake-qt5 --version
```

#### Paso 3: Clonar y Configurar

```bash
git clone https://github.com/Gocardi/ProyectoSO.git
cd ProyectoSO

# Si usas Qt5, editar simulador_bancario.pro si es necesario
```

---

### Windows 10/11

#### Paso 1: Instalar MinGW-w64 con MSYS2

1. Descargar **MSYS2** desde: https://www.msys2.org/
2. Ejecutar instalador: `msys2-x86_64-xxxxxxxx.exe`
3. Instalar en: `C:\msys64` (ruta recomendada)
4. Abrir terminal **MSYS2 MINGW64** (icono azul)
5. Ejecutar los siguientes comandos:

```bash
# Actualizar paquetes base
pacman -Syu
# Cerrar terminal cuando lo pida

# Volver a abrir MSYS2 MINGW64 y ejecutar:
pacman -Su

# Instalar toolchain completo
pacman -S mingw-w64-x86_64-toolchain

# Instalar Qt6
pacman -S mingw-w64-x86_64-qt6

# Instalar make
pacman -S make
```

#### Paso 2: Configurar Variables de Entorno

1. Presionar `Win + R`, escribir `sysdm.cpl`, presionar Enter
2. Ir a pestaña **"Opciones avanzadas"**
3. Hacer clic en **"Variables de entorno"**
4. En **"Variables del sistema"**, buscar la variable `Path`
5. Seleccionar `Path` y hacer clic en **"Editar"**
6. Hacer clic en **"Nuevo"** y agregar las siguientes rutas:
   - `C:\msys64\mingw64\bin`
   - `C:\msys64\usr\bin`
7. Hacer clic en **"Aceptar"** en todas las ventanas
8. **Reiniciar** el terminal o sistema

#### Paso 3: Verificar Instalación

Abrir **PowerShell** o **cmd**:

```cmd
:: Verificar g++
g++ --version
:: Debe mostrar: g++ (Rev...) 13.x.x

:: Verificar qmake
qmake --version
:: Debe mostrar: QMake version 3.1, Using Qt version 6.x.x

:: Verificar make
mingw32-make --version
:: Debe mostrar: GNU Make 4.x
```

#### Paso 4: Clonar Proyecto

```cmd
:: Clonar repositorio
git clone https://github.com/Gocardi/ProyectoSO.git
cd ProyectoSO

:: Listar archivos
dir
```

#### Alternativa: Qt Installer (Método Gráfico)

Si prefieres un instalador gráfico con IDE incluido:

1. Descargar **Qt Online Installer**: https://www.qt.io/download-qt-installer
2. Ejecutar instalador e iniciar sesión (crear cuenta gratuita)
3. En la selección de componentes, elegir:
   - **Qt 6.x.x** para Windows
   - **MinGW 11.2.0 64-bit** (compilador)
   - **Qt Creator** (IDE opcional pero recomendado)
   - **Qt 5 Compatibility Module** (opcional)
4. Instalar en: `C:\Qt` (ruta por defecto)
5. Agregar al PATH las rutas:
   - `C:\Qt\6.x.x\mingw_64\bin`
   - `C:\Qt\Tools\mingw1120_64\bin`
6. Reiniciar terminal

---

## Compilación

### Opción 1: GUI Qt (RECOMENDADO)

#### Linux (Arch/Ubuntu)

```bash
cd ProyectoSO

# Usar script de compilación
chmod +x compilar_qt.sh
./compilar_qt.sh

# O manualmente:
qmake simulador_bancario.pro
make -j$(nproc)

# Ejecutar
./simulador_qt
```

#### Windows

```cmd
cd ProyectoSO

:: Generar Makefile
qmake simulador_bancario.pro

:: Compilar (usar número de cores disponibles)
mingw32-make -j4

:: Ejecutar
simulador_qt.exe
```

### Opción 2: Versiones CLI

#### Linux

```bash
# Compilar todas las versiones
make

# O individualmente:
make simulador                # CLI automática (30s)
make simulador_interactivo    # CLI con menú
make simulador_gui            # GUI ASCII

# Ejecutar
./simulador
./simulador_interactivo
./simulador_gui
```

#### Windows

```cmd
:: Usar Makefile de Windows
mingw32-make -f Makefile.win

:: O compilar manualmente cada versión:
g++ -std=c++17 -Iinclude src/main.cpp src/productor_consumidor.cpp src/lectores_escritores.cpp src/monitor.cpp src/deadlock.cpp -o simulador.exe -pthread

g++ -std=c++17 -Iinclude src/main_interactivo.cpp src/productor_consumidor.cpp src/lectores_escritores.cpp src/monitor.cpp src/deadlock.cpp src/simulador_interactivo.cpp -o simulador_interactivo.exe -pthread

g++ -std=c++17 -Iinclude src/main_gui.cpp src/productor_consumidor.cpp src/lectores_escritores.cpp src/monitor.cpp src/deadlock.cpp src/gui_basica.cpp -o simulador_gui.exe -pthread

:: Ejecutar
simulador.exe
simulador_interactivo.exe
simulador_gui.exe
```

---

## Modos de Ejecución

### 1. GUI Qt (RECOMENDADO)

```bash
# Linux
./simulador_qt

# Windows
simulador_qt.exe
```

**4 Tabs disponibles**:

#### Tab 1: Gestión de Usuarios
- Crear nuevos usuarios con saldo inicial
- Ver lista completa con saldos actuales
- Actualizar tabla en tiempo real
- Seleccionar usuario para transacciones

**Columnas**: Nombre | Cuenta ID | Saldo | Fecha Creación

#### Tab 2: Transacciones
- Enviar transferencias entre usuarios
- Ver historial completo de transacciones
- Visualizar transacciones sospechosas (fondo amarillo)
- Validación automática de fondos

**Columnas**: ID | Origen | Destino | Monto | Tipo | Sospechosa | Fecha

**Reglas de detección de fraude**:
- Monto > $8,000 = Sospechosa
- Retiro > $5,000 = Sospechosa

#### Tab 3: Estadísticas y Monitoreo
**Métricas en tiempo real**:
- Total de transacciones procesadas
- Transacciones aprobadas (verde)
- Transacciones sospechosas (naranja)
- Monto total procesado
- Tamaño actual de la cola (X/10)

**Control**: Iniciar/Detener procesamiento automático

**Log de actividad**: Con timestamps y colores (éxito/error/warning)

#### Tab 4: Demostraciones

**Demostraciones interactivas con explicación paso a paso**:

##### 1. Provocar Deadlock
- Simula interbloqueo entre 2 hilos
- Muestra ciclo de espera paso a paso
- Explica condiciones de Coffman
- Log detallado: Thread 1 espera Thread 2, Thread 2 espera Thread 1

##### 2. Resolver Deadlock
- Usa `std::scoped_lock` para prevención
- Adquisición atómica de múltiples mutex
- Compara con versión con deadlock
- Explica algoritmo de ordenamiento interno

##### 3. Demostrar Semáforo
- Limita 3 motores antifraude simultáneos
- Simula 5 transacciones compitiendo por recursos
- Muestra bloqueo cuando no hay recursos disponibles
- Muestra liberación y reasignación de recursos

##### 4. Demostrar Lectores-Escritores
- Múltiples lectores acceden simultáneamente
- Escritor requiere acceso exclusivo
- Lectores esperan cuando hay escritor activo
- Explica `shared_lock` vs `unique_lock`

**Características del log de demostraciones**:
- Timestamps en cada línea
- Verde (éxito), Rojo (error), Naranja (warning), Azul (info)
- Explicaciones pedagógicas detalladas
- Ejecución en hilos separados (no bloquea UI)

---

### 2. CLI Interactiva

```bash
# Linux
./simulador_interactivo

# Windows
simulador_interactivo.exe
```

**Menú principal**:
```
===========================================
    SIMULADOR BANCARIO INTERACTIVO
===========================================

[1] Crear usuario
[2] Listar usuarios
[3] Enviar transacción
[4] Consultar saldo
[5] Ver estadísticas
[0] Salir
```

**Funcionalidades**:
- Crear usuarios con nombre y saldo inicial
- Listar todos los usuarios con sus saldos
- Enviar transferencias manuales entre usuarios
- Consultar saldo de un usuario específico
- Ver estadísticas de transacciones procesadas

**Usuarios preconfigurados**:
- Juan - CTA-Juan - $10,000
- Maria - CTA-Maria - $15,000
- Pedro - CTA-Pedro - $8,000
- Ana - CTA-Ana - $12,000
- Luis - CTA-Luis - $20,000

---

### 3. GUI ASCII

```bash
# Linux
./simulador_gui

# Windows
simulador_gui.exe
```

Interfaz visual en terminal usando códigos ANSI con:
- Dashboard con estadísticas
- Panel de usuarios con colores
- Panel de transacciones
- Formularios interactivos

---

### 4. CLI Automática

```bash
# Linux
./simulador

# Windows
simulador.exe
```

Simula 30 segundos de operación automática con:
- 5 clientes generando transacciones
- 3 motores antifraude procesando
- Lectores/escritores de configuración
- Analista financiero consultando
- Administrador modificando configuración

---

## Persistencia de Datos

### Archivos JSON

El sistema usa 2 archivos JSON:

#### `usuarios.json`
```json
[
  {
    "nombre": "Juan",
    "cuenta_id": "CTA-Juan",
    "saldo": 10000.00,
    "fecha_creacion": "2025-11-09 12:00:00"
  }
]
```

#### `transacciones.json`
```json
[
  {
    "id": 1,
    "usuario_origen": "Juan",
    "usuario_destino": "Maria",
    "monto": 500.00,
    "tipo": "TRANSFERENCIA",
    "es_sospechosa": false,
    "fecha": "2025-11-09 12:05:30"
  }
]
```

### Clase DatabaseJSON

**Operaciones disponibles**:
- `guardar_usuario()` - Crea nuevo usuario
- `actualizar_saldo()` - Modifica saldo
- `cargar_usuarios()` - Lee todos los usuarios
- `guardar_transaccion()` - Registra transacción
- `cargar_transacciones(limite)` - Lee historial
- `cargar_transacciones_usuario()` - Filtra por usuario
- `exportar_backup(dir)` - Crea backup con timestamp

**Thread-Safety**: Todas las operaciones usan `std::lock_guard<std::mutex>`

---

## Estructura del Proyecto

```
ProyectoSO/
│
├── include/                       # Headers (10 archivos)
│   ├── modelos.hpp                # Estructura Transaccion
│   ├── productor_consumidor.hpp   # Cola + Cliente + Motor
│   ├── lectores_escritores.hpp    # ConfiguracionSistema
│   ├── monitor.hpp                # MonitorCuentas
│   ├── deadlock.hpp               # Demostraciones deadlock
│   ├── semaforo.hpp               # Semáforo C++17
│   ├── database_json.hpp          # Persistencia JSON
│   ├── simulador_interactivo.hpp  # Lógica CLI interactiva
│   ├── gui_basica.hpp             # GUI ASCII
│   └── mainwindow.hpp             # Ventana principal Qt
│
├── src/                           # Implementaciones (12 archivos)
│   ├── main.cpp                   # CLI automática
│   ├── main_interactivo.cpp       # CLI interactiva
│   ├── main_gui.cpp               # GUI ASCII
│   ├── main_qt.cpp                # GUI Qt
│   ├── mainwindow.cpp             # Ventana Qt (700+ líneas)
│   ├── database_json.cpp          # Persistencia (300+ líneas)
│   ├── gui_basica.cpp             # GUI ASCII
│   ├── simulador_interactivo.cpp  # Lógica interactiva
│   ├── productor_consumidor.cpp   # Prod-Cons (200+ líneas)
│   ├── lectores_escritores.cpp    # Lect-Escrit
│   ├── monitor.cpp                # Monitor (150+ líneas)
│   └── deadlock.cpp               # Demos deadlock
│
├── Configuración
│   ├── config.json                # Configuración del sistema
│   ├── simulador_bancario.pro     # Proyecto Qt (qmake)
│   ├── Makefile                   # Build CLI (Linux)
│   ├── Makefile.win               # Build CLI (Windows)
│   ├── compilar.sh                # Script Linux
│   ├── compilar.bat               # Script Windows
│   └── compilar_qt.sh             # Script Qt
│
├── Documentación
│   ├── README.md                  # Este archivo
│   └── documentacion.tex          # LaTeX (70+ páginas)
│
└── Datos (generados en runtime)
    ├── usuarios.json              # BD de usuarios
    └── transacciones.json         # Historial
```

---

## Limpieza de Archivos

### Archivos Generados (Pueden Eliminarse)

```bash
# Directorios de compilación
rm -rf obj/              # Archivos objeto (.o)
rm -rf moc/              # Meta-Object Compiler (Qt)
rm -rf ui/               # UI generados (Qt Designer)

# Ejecutables
rm -f simulador                  # CLI automática
rm -f simulador_interactivo     # CLI interactiva
rm -f simulador_gui             # GUI ASCII
rm -f simulador_qt              # GUI Qt

# Archivos temporales
rm -f .qmake.stash
rm -f Makefile.Debug
rm -f Makefile.Release
rm -f *~ *.swp .DS_Store
```

### Datos de Ejemplo (Opcional)

```bash
# ADVERTENCIA: Eliminar solo si quieres empezar desde cero
rm -f usuarios.json           # Perderás usuarios creados
rm -f transacciones.json      # Perderás historial
```

### Comando de Limpieza Completa

```bash
# Limpieza segura (solo archivos compilados)
make clean
rm -rf moc obj ui .qmake.stash
rm -f simulador simulador_interactivo simulador_gui simulador_qt

# Limpieza total (incluye datos) - ADVERTENCIA
make clean
rm -rf moc obj ui .qmake.stash
rm -f simulador* *.json
```

### Archivos que SI Debes Mantener

**NO ELIMINES**:
- `include/*.hpp` - Headers del código fuente
- `src/*.cpp` - Implementaciones
- `config.json` - Configuración del sistema
- `*.pro` - Proyecto Qt
- `Makefile*` - Build systems
- `README.md` - Documentación
- `documentacion.tex` - LaTeX

---

## Documentación LaTeX

El proyecto incluye documentación profesional de **70+ páginas** en LaTeX.

### Compilar PDF

```bash
cd ProyectoSO

# Primera compilación
pdflatex documentacion.tex

# Segunda compilación (para referencias cruzadas)
pdflatex documentacion.tex

# Resultado: documentacion.pdf
```

### Contenido del Documento

1. **Introducción** - Objetivos y tecnologías
2. **Arquitectura** - Diagramas de componentes y flujo
3. **Conceptos de SO** - Explicación detallada con código
4. **Implementación** - Pseudocódigo de cada patrón
5. **Interfaz Qt** - Screenshots y guía de uso
6. **Persistencia JSON** - Estructura y operaciones
7. **Casos de Uso** - Ejemplos paso a paso
8. **Pruebas** - Validación de concurrencia
9. **Limitaciones** - Mejoras futuras
10. **Código Fuente** - Funciones clave comentadas
11. **Referencias** - Bibliografía académica

---

## Ejemplos de Uso

### Ejemplo 1: Crear Usuario y Transferir (GUI Qt)

```bash
./simulador_qt

# Tab "Usuarios"
-> Nombre: "Pedro"
-> Saldo inicial: $20,000
-> Click "Crear Usuario"

# Tab "Transacciones"
-> Origen: "Juan"
-> Destino: "Pedro"
-> Monto: $1,500
-> Click "Enviar Transacción"

# Resultado: Transferencia exitosa, JSON actualizado
```

### Ejemplo 2: Demostrar Deadlock (GUI Qt)

```bash
# Tab "Demostraciones"

# Provocar Deadlock
-> Click "Provocar Deadlock"
-> Log muestra: Thread 1 espera Thread 2
-> Log muestra: Thread 2 espera Thread 1
-> Deadlock detectado!

# Resolver Deadlock
-> Click "Resolver Deadlock"
-> Log muestra: std::scoped_lock adquiere ambos mutex
-> Sin ciclo de espera, transacciones completan
```

### Ejemplo 3: Transacción Sospechosa

```bash
# GUI Qt - Tab "Transacciones"
-> Origen: "Maria"
-> Destino: "Ana"
-> Monto: $10,000  # > $8,000

# Resultado:
-> Marcada como sospechosa
-> Fondo amarillo en tabla
-> Contador de sospechosas incrementa
-> Log: "Transacción sospechosa detectada"
```

### Ejemplo 4: CLI Interactiva

```bash
./simulador_interactivo

# Opción [1] - Crear usuario
-> Nombre: Carlos
-> Saldo: 5000
-> Usuario creado: CTA-Carlos

# Opción [3] - Enviar transacción
-> Usuario origen: Juan
-> Usuario destino: Carlos
-> Monto: 500
-> Transferencia exitosa

# Opción [5] - Ver estadísticas
-> Transacciones procesadas: 1
-> Aprobadas: 1
-> Sospechosas: 0
```

---

## Tecnologías

| Tecnología | Versión | Uso |
|------------|---------|-----|
| **C++** | 17 | Lenguaje base |
| **Qt** | 5/6 | Framework GUI |
| **std::thread** | C++11 | Hilos nativos |
| **std::mutex** | C++11 | Exclusión mutua |
| **std::shared_mutex** | C++17 | Lectores-Escritores |
| **std::condition_variable** | C++11 | Sincronización |
| **std::atomic** | C++11 | Variables atómicas |
| **std::scoped_lock** | C++17 | Prevención deadlock |
| **qmake** | - | Build Qt |
| **Make** | - | Build CLI |

---

## Configuración (config.json)

```json
{
  "capacidad_cola": 10,              // Tamaño buffer Prod-Cons
  "num_clientes": 5,                 // Productores
  "num_motores": 3,                  // Consumidores + límite semáforo
  "delay_cliente_ms": 1000,          // Intervalo producción
  "delay_motor_ms": 1500,            // Tiempo procesamiento
  "duracion_simulacion_segundos": 30,// Tiempo CLI auto
  "max_transacciones_simultaneas": 100,
  "timeout_transaccion_segundos": 30,
  "modo_antifraude": true,           // Detección activa
  "demo_deadlock": false             // Activar demo deadlock
}
```

---

## Referencias

1. Tanenbaum, A. S. (2014). *Modern Operating Systems* (4th ed.)
2. Silberschatz, A. (2018). *Operating System Concepts* (10th ed.)
3. Williams, A. (2019). *C++ Concurrency in Action* (2nd ed.)
4. ISO/IEC 14882:2017 - C++17 Standard
5. Qt Documentation - https://doc.qt.io/
6. Dijkstra, E. W. (1965). *Concurrent Programming*
7. Hoare, C. A. R. (1974). *Monitors*
8. Coffman, E. G. (1971). *System Deadlocks*

---

## Aprendizajes Clave

### Sincronización
- Uso correcto de `std::condition_variable` con predicados  
- Prevención de deadlocks con `std::scoped_lock`  
- Lectores-Escritores con `std::shared_mutex`  
- Semáforos personalizados en C++17  

### Buenas Prácticas
- RAII para gestión automática de recursos  
- Thread-safety en todas las operaciones críticas  
- Separación de capas (presentación/lógica/datos)  
- Código documentado con comentarios  
- Multiplataforma (Linux/Windows)  

---

## Mejoras Futuras

### Corto Plazo
- [ ] Migrar a SQLite para mejor I/O
- [ ] Autenticación con hash bcrypt
- [ ] Logs rotativos
- [ ] Gráficos Qt Charts

### Mediano Plazo
- [ ] Arquitectura cliente-servidor (TCP)
- [ ] Protocolo JSON-RPC
- [ ] Balanceo de carga
- [ ] Replicación maestro-esclavo

### Largo Plazo
- [ ] Microservicios
- [ ] RabbitMQ/Kafka
- [ ] CQRS pattern
- [ ] Kubernetes deployment

---

## Autor

**Proyecto Académico de Sistemas Operativos**  
Implementación de conceptos de concurrencia y sincronización

---

## Licencia

Uso académico y educativo

---

## Agradecimientos

- Profesores de Sistemas Operativos
- Comunidad de C++ y Qt
- Autores de las referencias bibliográficas

---

*Última actualización: Noviembre 2025*
