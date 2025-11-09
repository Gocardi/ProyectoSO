cmake_minimum_required(VERSION 3.14)
project(SimuladorBancarioConcurrente)

# Configurar C++17 (requerido para std::shared_mutex, std::scoped_lock)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Flags de compilación
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")

# Incluir el directorio de headers
include_directories(include)

# Buscar o descargar nlohmann/json
find_package(nlohmann_json 3.2.0 QUIET)

if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json no encontrado, descargando...")
    include(FetchContent)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(json)
endif()

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/productor_consumidor.cpp
    src/lectores_escritores.cpp
    src/monitor.cpp
    src/deadlock.cpp
)

# Crear el ejecutable
add_executable(simulador ${SOURCES})

# Enlazar con nlohmann_json y threads
target_link_libraries(simulador PRIVATE nlohmann_json::nlohmann_json pthread)

# Copiar config.json al directorio de build
configure_file(config.json config.json COPYONLY)

message(STATUS "Configuración completada")
message(STATUS "Para compilar: mkdir build && cd build && cmake .. && make")
message(STATUS "Para ejecutar: ./simulador")
