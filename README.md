# Simulador de Sistema Bancario Concurrente

Implementación en C++ de un sistema bancario que demuestra conceptos de programación concurrente:
- **Productor-Consumidor**: Clientes generan transacciones, motores antifraude las procesan
- **Lectores-Escritores**: Analistas leen configuración, administradores la modifican
- **Monitor**: Gestión segura de cuentas bancarias
- **Semáforos**: Control de acceso concurrente limitado
- **Deadlock**: Demostración de provocación y resolución

## Estructura del Proyecto

```
ProyectoSO/
├── include/
│   ├── modelos.hpp                  # Definición de struct Transaccion
│   ├── productor_consumidor.hpp     # Cola, Cliente, MotorAntifraude
│   ├── lectores_escritores.hpp      # ConfiguracionSistema, Analistas, Admins
│   ├── monitor.hpp                  # MonitorCuentas
│   └── deadlock.hpp                 # Demos de deadlock
├── src/
│   ├── main.cpp                     # Orquestador principal
│   ├── productor_consumidor.cpp     # Implementación Productor-Consumidor
│   ├── lectores_escritores.cpp      # Implementación Lectores-Escritores
│   ├── monitor.cpp                  # Implementación Monitor
│   └── deadlock.cpp                 # Implementación demos deadlock
├── config.json                      # Configuración de la simulación
├── CMakeLists.txt                   # Archivo de construcción CMake
└── README.md                        # Este archivo
```

## Requisitos

- **C++17** o superior (para `std::shared_mutex`, `std::scoped_lock`, `std::counting_semaphore`)
- **CMake** 3.14 o superior
- **g++** o **clang++** con soporte de C++17
- **pthread** (generalmente incluido en sistemas Linux)

### Instalación de dependencias (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake
```

## Compilación

### Opción 1: Usando CMake (Recomendado)

```bash
# Crear directorio de build
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
