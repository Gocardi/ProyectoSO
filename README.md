# 🏦 Simulador de Sistema Bancario Concurrente

Implementación completa en C++ de un sistema bancario que demuestra los principales conceptos de programación concurrente y sincronización de hilos.

## 📋 Conceptos Implementados

- ✅ **Productor-Consumidor**: Clientes generan transacciones, motores antifraude las procesan
- ✅ **Lectores-Escritores**: Múltiples analistas leen, un administrador escribe
- ✅ **Monitor**: Gestión segura de cuentas bancarias con sincronización
- ✅ **Semáforos**: Control de acceso concurrente limitado (máximo 3 motores)
- ✅ **Deadlock**: Demostración de provocación y resolución con `std::scoped_lock`

---

## 📂 Estructura del Proyecto

```
ProyectoSO/
├── include/                         # Archivos de cabecera
│   ├── modelos.hpp                  # struct Transaccion
│   ├── productor_consumidor.hpp     # Cola, Cliente, MotorAntifraude
│   ├── lectores_escritores.hpp      # ConfiguracionSistema, Analistas, Admins
│   ├── monitor.hpp                  # MonitorCuentas
│   ├── deadlock.hpp                 # Demos de deadlock
│   └── semaforo.hpp                 # Implementación de semáforo (C++17)
├── src/                             # Código fuente
│   ├── main.cpp                     # Orquestador principal
│   ├── productor_consumidor.cpp     # Implementación Productor-Consumidor
│   ├── lectores_escritores.cpp      # Implementación Lectores-Escritores
│   ├── monitor.cpp                  # Implementación Monitor
│   └── deadlock.cpp                 # Implementación demos deadlock
├── config.json                      # Configuración de la simulación
├── Makefile                         # Compilación para Linux
├── Makefile.win                     # Compilación para Windows
├── compilar.sh                      # Script automático (Linux/Mac)
├── compilar.bat                     # Script automático (Windows)
└── README.md                        # Este archivo
```

---

## 🔧 Requisitos del Sistema

### Requisitos Mínimos
- **C++17** o superior
- **g++ 7.0+** o **clang++ 5.0+**
- **pthread** (incluido en Linux/Mac, MinGW en Windows)
- **make** (opcional, para usar Makefile)

### Sin Dependencias Externas
✅ No requiere CMake  
✅ No requiere librerías externas  
✅ Solo estándar de C++17

---

## 🚀 Compilación y Ejecución

### 🐧 Linux (Arch, Ubuntu, Debian)

#### Método 1: Con Makefile (Recomendado)
```bash
cd ProyectoSO
make clean
make
./simulador
```

#### Método 2: Script Automático
```bash
chmod +x compilar.sh
./compilar.sh
./simulador
```

#### Método 3: Compilación Manual
```bash
g++ -std=c++17 -pthread -Wall -Wextra -O2 -I./include \
    src/main.cpp \
    src/productor_consumidor.cpp \
    src/lectores_escritores.cpp \
    src/monitor.cpp \
    src/deadlock.cpp \
    -o simulador
./simulador
```

### Instalar dependencias en Linux

**Arch Linux:**
```bash
sudo pacman -S base-devel gcc
```

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential g++ make
```

---

### 🪟 Windows

#### Método 1: MSYS2 (Recomendado)

1. **Instalar MSYS2** desde https://www.msys2.org/

2. **Abrir MSYS2 MinGW 64-bit** y ejecutar:
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make
```

3. **Navegar al proyecto y compilar:**
```bash
cd /c/Users/TU_USUARIO/ProyectoSO
mingw32-make clean
mingw32-make
./simulador.exe
```

#### Método 2: Script Automático (Windows)
```cmd
compilar.bat
simulador.exe
```

#### Método 3: MinGW Manual
```cmd
g++ -std=c++17 -pthread -Wall -Wextra -O2 -I./include ^
    src/main.cpp ^
    src/productor_consumidor.cpp ^
    src/lectores_escritores.cpp ^
    src/monitor.cpp ^
    src/deadlock.cpp ^
    -o simulador.exe

simulador.exe
```

---

## ⚙️ Configuración

Edita `config.json` para personalizar la simulación:

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

### Parámetros

| Parámetro | Descripción | Valor por defecto |
|-----------|-------------|-------------------|
| `capacidad_cola` | Tamaño máximo del buffer de transacciones | 10 |
| `num_clientes` | Cantidad de productores (clientes) | 2 |
| `num_motores` | Cantidad de consumidores (motores antifraude) | 3 |
| `num_analistas` | Cantidad de lectores (analistas financieros) | 3 |
| `num_administradores` | Cantidad de escritores (administradores) | 1 |
| `duracion_segundos` | Duración de la simulación principal | 30 |
| `demo_monitor` | Ejecutar demostración del monitor | true |
| `demo_deadlock` | Ejecutar demostración de deadlock | true |

---

## 🎯 Componentes Implementados

### 1️⃣ Productor-Consumidor (Persona 2)

**Archivos:** `productor_consumidor.hpp/cpp`, `modelos.hpp`

- **ColaTransacciones**: Buffer limitado thread-safe con `std::mutex` y `std::condition_variable`
- **Cliente (Productor)**: Genera transacciones bancarias aleatorias
- **MotorAntifraude (Consumidor)**: Procesa y analiza transacciones

**Conceptos:**
- Buffer limitado
- Bloqueo cuando está llena (productor) o vacía (consumidor)
- Notificaciones con `condition_variable`

```cpp
// Ejemplo de uso
ColaTransacciones cola(10);
cola.producir(transaccion);  // Bloquea si está llena
Transaccion t = cola.consumir();  // Bloquea si está vacía
```

---

### 2️⃣ Lectores-Escritores (Persona 3)

**Archivos:** `lectores_escritores.hpp/cpp`

- **ConfiguracionSistema**: Usa `std::shared_mutex` para acceso concurrente
- **AnalistaFinanciero (Lector)**: Lee configuración sin bloqueo mutuo
- **AdministradorSistema (Escritor)**: Modifica configuración exclusivamente

**Conceptos:**
- Múltiples lectores simultáneos con `std::shared_lock`
- Un solo escritor con `std::unique_lock`
- Bloqueo de escritores cuando hay lectores activos

```cpp
// Lectura (múltiples lectores)
std::shared_lock<std::shared_mutex> lock(mtx);
// ... leer datos ...

// Escritura (exclusiva)
std::unique_lock<std::shared_mutex> lock(mtx);
// ... modificar datos ...
```

---

### 3️⃣ Semáforo (Persona 3)

**Archivo:** `semaforo.hpp`

- Implementación de semáforo contador para C++17
- Limita acceso concurrente a recursos (máximo 3 motores antifraude)
- Métodos `acquire()` y `release()`

**Conceptos:**
- Control de acceso limitado
- Espera bloqueante cuando no hay permisos
- Notificación al liberar permisos

```cpp
Semaforo sem(3);  // Máximo 3 hilos concurrentes
sem.acquire();    // Obtener permiso
// ... usar recurso ...
sem.release();    // Liberar permiso
```

---

### 4️⃣ Monitor (Persona 4)

**Archivo:** `monitor.hpp/cpp`

- **MonitorCuentas**: Encapsula saldos bancarios con sincronización interna
- Métodos: `transferir()`, `depositar()`, `retirar()`, `consultar_saldo()`
- Usa `std::mutex` y `std::condition_variable`

**Conceptos:**
- Encapsulación de datos compartidos
- Sincronización interna automática
- Espera bloqueante si no hay fondos suficientes

```cpp
MonitorCuentas monitor;
monitor.transferir("CTA-001", "CTA-002", 500.0);
double saldo = monitor.consultar_saldo("CTA-001");
```

---

### 5️⃣ Deadlock (Persona 4)

**Archivo:** `deadlock.hpp/cpp`

- **provocar_deadlock()**: Demuestra deadlock con locks en orden inverso
- **resolver_deadlock()**: Previene deadlock con `std::scoped_lock`

**Conceptos:**
- Condiciones para deadlock (ciclo de espera)
- Prevención con ordenamiento de locks
- Adquisición atómica con `std::scoped_lock`

```cpp
// ❌ Deadlock
std::lock_guard<std::mutex> lock1(mutex_A);
std::lock_guard<std::mutex> lock2(mutex_B);

// ✅ Sin deadlock
std::scoped_lock lock(mutex_A, mutex_B);  // Atómico
```

---

## 📊 Salida Esperada

Al ejecutar el programa:

```
================================================
  SIMULADOR DE SISTEMA BANCARIO CONCURRENTE
  Gestión de Transacciones con Concurrencia
================================================

========== CONFIGURACIÓN ==========
Capacidad de cola: 10
Número de clientes: 2
Número de motores: 3
Número de analistas: 3
Número de administradores: 1
Duración: 30 segundos
===================================

[MAIN] Iniciando hilos...

[PRODUCTOR] Cliente CLI-1 añadió transacción #1 | Cola: 1/10
[CONSUMIDOR] Motor #1 procesando transacción #1 de cliente CLI-1
[LECTOR] Analista #1 leyó limite_transaccion = 10000
[OK] Motor #1 aprobó transacción #1

[ALERTA] Motor #2 detectó transacción sospechosa #5 | Monto: $8500.00

[ESCRITOR] Administrador #1 va a modificar limite_transaccion
[CONFIG] Actualizando limite_transaccion: 10000 -> 12500

...

[MAIN] Iniciando apagado ordenado...
[CLIENTE] Cliente CLI-1 finalizó.
[MOTOR] Motor Antifraude #1 finalizó.
...

============================================
DEMO: MONITOR DE CUENTAS
============================================

[MONITOR] Transferencia: CTA-001 -> CTA-002 | Monto: $500.00

========== ESTADO DE CUENTAS ==========
CTA-001: $3500.00
CTA-002: $3500.00
========================================

============================================
DEMO 2: RESOLVIENDO DEADLOCK
============================================

[HILO 1] MUTEX A y B bloqueados ✓
[HILO 2] MUTEX B y A bloqueados ✓

✅ Ambos hilos completaron exitosamente.
No hubo deadlock gracias a std::scoped_lock.
```

---

## 🔧 Solución de Problemas

### ❌ Error: "pthread: No such file or directory"
**Solución:** Usa el flag `-pthread` al compilar.

### ❌ Error: "std::shared_mutex: No such file or directory"
**Solución:** Actualiza tu compilador a GCC 7+ o Clang 5+
```bash
# Verificar versión
g++ --version

# Arch
sudo pacman -S gcc

# Ubuntu
sudo apt install g++-9
export CXX=g++-9
```

### ❌ Warnings sobre variables no utilizadas
**Solución:** Son solo warnings en `deadlock.cpp`, no errores. Puedes ignorarlos.

### ❌ Windows: "g++ no reconocido"
**Solución:** Agrega MinGW/MSYS2 al PATH:
- MinGW: `C:\MinGW\bin`
- MSYS2: `C:\msys64\mingw64\bin`

### ⚠️ El programa se congela
**Solución:** Si ejecutaste `provocar_deadlock()`, es intencional. Usa `Ctrl+C` para terminar.

---

## 👥 Asignación de Tareas por Persona

### Persona 1: Orquestador
**Archivos:** `main.cpp`
- ✅ Lectura de `config.json` (parser simple sin librerías)
- ✅ Gestión de hilos con `std::thread`
- ✅ Apagado ordenado con `std::atomic<bool>`
- ✅ Coordinación de todos los componentes

### Persona 2: Productor-Consumidor
**Archivos:** `modelos.hpp`, `productor_consumidor.hpp/cpp`
- ✅ `struct Transaccion`
- ✅ `ColaTransacciones` (buffer limitado thread-safe)
- ✅ `Cliente` (Productor)
- ✅ `MotorAntifraude` (Consumidor con semáforo)

### Persona 3: Lectores-Escritores y Semáforos
**Archivos:** `lectores_escritores.hpp/cpp`, `semaforo.hpp`
- ✅ `ConfiguracionSistema` con `std::shared_mutex`
- ✅ `AnalistaFinanciero` (Lector)
- ✅ `AdministradorSistema` (Escritor)
- ✅ `Semaforo` (implementación para C++17)

### Persona 4: Monitor y Deadlock
**Archivos:** `monitor.hpp/cpp`, `deadlock.hpp/cpp`
- ✅ `MonitorCuentas` con `std::mutex` y `condition_variable`
- ✅ `provocar_deadlock()`: Demo de deadlock
- ✅ `resolver_deadlock()`: Solución con `std::scoped_lock`

---

## 📚 Detalles Técnicos

### Mecanismos de Sincronización

| Componente | Mecanismo | Descripción |
|-----------|-----------|-------------|
| ColaTransacciones | `std::mutex` + `condition_variable` | Buffer limitado bloqueante |
| ConfiguracionSistema | `std::shared_mutex` | Múltiples lectores, escritor exclusivo |
| MotorAntifraude | `Semaforo` (custom) | Limita a 3 motores concurrentes |
| MonitorCuentas | `std::mutex` + `condition_variable` | Monitor con espera condicional |
| Deadlock | `std::scoped_lock` | Prevención de deadlock |

### Características de C++17 Utilizadas

- ✅ `std::shared_mutex` - Para lectores-escritores
- ✅ `std::scoped_lock` - Para prevenir deadlock
- ✅ `std::atomic<bool>` - Para señalización entre hilos
- ✅ `std::thread` - Para crear hilos
- ✅ `std::condition_variable` - Para esperas bloqueantes

---

## 🎓 Conceptos de Sistemas Operativos Demostrados

1. **Sincronización de Hilos**: Mutex, locks, semáforos
2. **Problemas Clásicos**: Productor-Consumidor, Lectores-Escritores
3. **Deadlock**: Detección y prevención
4. **Monitores**: Encapsulación de sincronización
5. **Condiciones de Carrera**: Prevención con locks
6. **Exclusión Mutua**: Con diferentes mecanismos
7. **Comunicación entre Procesos**: A través de memoria compartida

---

## 📖 Referencias

- [C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition)
- [C++17 std::shared_mutex](https://en.cppreference.com/w/cpp/thread/shared_mutex)
- [C++17 std::scoped_lock](https://en.cppreference.com/w/cpp/thread/scoped_lock)
- [Productor-Consumidor](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
- [Lectores-Escritores](https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem)

---

## 📝 Licencia

Este proyecto es una implementación educativa para el curso de Sistemas Operativos.

**Autor**: Equipo ProyectoSO  
**Fecha**: Noviembre 2025  
**Versión**: 1.0

---

## 🆘 Ayuda Adicional

Para más ayuda:

1. **Verifica soporte de C++17:**
   ```bash
   echo "#include <shared_mutex>" | g++ -std=c++17 -x c++ - -c -o /dev/null
   ```

2. **Compila en modo debug:**
   ```bash
   make clean
   make CXXFLAGS="-std=c++17 -Wall -Wextra -pthread -g"
   ```

3. **Ejecuta con valgrind (detectar memory leaks):**
   ```bash
   valgrind --leak-check=full ./simulador
   ```

---

**¡Disfruta explorando la programación concurrente! 🚀**
mkdir build
cd build

# Configurar con CMake
cmake ..

# Compilar
make

# Ejecutar
./simulador
```

### Opción 2: Compilación manual

```bash
g++ -std=c++17 -pthread \
    -I./include \
    src/main.cpp \
    src/productor_consumidor.cpp \
    src/lectores_escritores.cpp \
    src/monitor.cpp \
    src/deadlock.cpp \
    -o simulador

./simulador
```

**Nota**: La compilación manual requiere tener `nlohmann/json` instalado o incluir el header directamente.

## Configuración

Edita `config.json` para ajustar los parámetros de la simulación:

```json
{
  "capacidad_cola": 10,           // Tamaño máximo del buffer de transacciones
  "num_clientes": 2,              // Número de productores (clientes)
  "num_motores": 3,               // Número de consumidores (motores antifraude)
  "num_analistas": 3,             // Número de lectores
  "num_administradores": 1,       // Número de escritores
  "duracion_segundos": 30,        // Duración de la simulación
  "demo_monitor": true,           // Ejecutar demo del monitor
  "demo_deadlock": true           // Ejecutar demo de deadlock
}
```

## Uso

### Ejecución básica

```bash
./simulador
```

### Ejecución con archivo de configuración personalizado

```bash
./simulador mi_config.json
```

## Componentes Implementados

### 1. Productor-Consumidor (ColaTransacciones)

**Persona 2**: Implementa un buffer limitado thread-safe.

- **ColaTransacciones**: Cola con `std::mutex` y `std::condition_variable`
- **Cliente** (Productor): Genera transacciones bancarias aleatorias
- **MotorAntifraude** (Consumidor): Procesa y analiza transacciones

**Conceptos**: 
- Buffer limitado
- Bloqueo cuando está llena (productor) o vacía (consumidor)
- Notificaciones con `condition_variable`

### 2. Lectores-Escritores (ConfiguracionSistema)

**Persona 3**: Gestiona configuración con acceso concurrente.

- **ConfiguracionSistema**: Usa `std::shared_mutex` (C++17)
- **AnalistaFinanciero** (Lector): Lee configuración sin bloqueo mutuo
- **AdministradorSistema** (Escritor): Modifica configuración exclusivamente

**Conceptos**:
- Múltiples lectores simultáneos
- Un solo escritor (bloquea lectores y otros escritores)
- `std::shared_lock` vs `std::unique_lock`

### 3. Semáforo

**Persona 3**: Limita acceso concurrente a recursos.

- `std::counting_semaphore<3>`: Máximo 3 motores procesando simultáneamente
- `acquire()` antes de procesar
- `release()` después de procesar

### 4. Monitor (MonitorCuentas)

**Persona 4**: Encapsula estado y sincronización.

- **MonitorCuentas**: Gestiona saldos con `std::mutex` y `std::condition_variable`
- Métodos sincronizados: `transferir()`, `depositar()`, `retirar()`
- Espera bloqueante si no hay fondos suficientes

**Conceptos**:
- Encapsulación de datos compartidos
- Sincronización interna
- Variables de condición para esperas

### 5. Deadlock

**Persona 4**: Demostración de deadlock y su resolución.

- **provocar_deadlock()**: Dos hilos bloquean mutex en orden inverso
- **resolver_deadlock()**: Usa `std::scoped_lock` (C++17) para prevenir deadlock

**Conceptos**:
- Condiciones para deadlock
- Prevención con ordenamiento de locks
- Adquisición atómica de múltiples locks

## Salida Esperada

El programa mostrará:

```
================================================
  SIMULADOR DE SISTEMA BANCARIO CONCURRENTE
  Gestión de Transacciones con Concurrencia
================================================

========== CONFIGURACIÓN ==========
Capacidad de cola: 10
Número de clientes: 2
Número de motores: 3
...

[PRODUCTOR] Cliente CLI-1 añadió transacción #1 | Cola: 1/10
[CONSUMIDOR] Motor #1 procesando transacción #1 de cliente CLI-1
[LECTOR] Analista #1 leyó limite_transaccion = 10000
[ESCRITOR] Administrador #1 va a modificar limite_transaccion
...

[MAIN] Iniciando apagado ordenado...
[MAIN] Todos los hilos finalizados.

============================================
DEMO: MONITOR DE CUENTAS
============================================
...
```

## Detalles Técnicos

### Sincronización Implementada

| Componente | Mecanismo | Descripción |
|-----------|-----------|-------------|
| ColaTransacciones | `std::mutex` + `condition_variable` | Buffer limitado bloqueante |
| ConfiguracionSistema | `std::shared_mutex` | Lectores múltiples, escritor exclusivo |
| MotorAntifraude | `std::counting_semaphore<3>` | Limita acceso concurrente |
| MonitorCuentas | `std::mutex` + `condition_variable` | Monitor con espera condicional |
| Deadlock | `std::scoped_lock` | Prevención de deadlock |

### Características C++17

- `std::shared_mutex`: Para lectores-escritores
- `std::scoped_lock`: Para prevenir deadlock
- `std::counting_semaphore`: Para limitar concurrencia (C++20, puede requerir ajuste)

**Nota**: Si tu compilador no soporta `std::counting_semaphore` (C++20), puedes implementarlo con `std::mutex` y `std::condition_variable`.

## Resolución de Problemas

### Error: `std::counting_semaphore` no encontrado

Si usas C++17 y no C++20, reemplaza el semáforo con una implementación manual:

```cpp
class Semaphore {
    std::mutex mtx;
    std::condition_variable cv;
    int count;
public:
    Semaphore(int count) : count(count) {}
    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        --count;
    }
    void release() {
        std::lock_guard<std::mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }
};
```

### Error: nlohmann/json no encontrado

CMake descargará automáticamente la librería. Si falla:

```bash
# Instalar manualmente
sudo apt install nlohmann-json3-dev
```

O descargar el header único desde: https://github.com/nlohmann/json/releases

## Licencia

Este proyecto es una implementación educativa para el curso de Sistemas Operativos.

## Autores

- Equipo ProyectoSO
- Fecha: Noviembre 2025
