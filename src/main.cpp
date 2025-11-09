#include "productor_consumidor.hpp"
#include "lectores_escritores.hpp"
#include "monitor.hpp"
#include "deadlock.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <fstream>
#include <sstream>
#include <string>

// Estructura de configuración cargada desde JSON
struct ConfiguracionApp {
    int capacidad_cola;
    int num_clientes;
    int num_motores;
    int num_analistas;
    int num_administradores;
    int duracion_segundos;
    bool demo_monitor;
    bool demo_deadlock;
    
    ConfiguracionApp() 
        : capacidad_cola(10), num_clientes(2), num_motores(3),
          num_analistas(3), num_administradores(1), duracion_segundos(30),
          demo_monitor(true), demo_deadlock(false) {}
};

// Función para cargar configuración desde config.json
ConfiguracionApp cargar_configuracion(const std::string& archivo) {
    ConfiguracionApp config;
    
    try {
        std::ifstream file(archivo);
        if (!file.is_open()) {
            std::cout << "[CONFIG] No se encontró " << archivo 
                      << ", usando configuración por defecto." << std::endl;
            return config;
        }
        
        // Lectura simple sin JSON (formato: clave=valor)
        std::string linea;
        while (std::getline(file, linea)) {
            // Ignorar líneas vacías y comentarios
            if (linea.empty() || linea[0] == '#' || linea[0] == '{' || linea[0] == '}') 
                continue;
            
            // Buscar el formato "clave": valor
            size_t pos_colon = linea.find(':');
            if (pos_colon == std::string::npos) continue;
            
            // Extraer clave
            size_t start_quote = linea.find('"');
            size_t end_quote = linea.find('"', start_quote + 1);
            if (start_quote == std::string::npos || end_quote == std::string::npos) continue;
            
            std::string clave = linea.substr(start_quote + 1, end_quote - start_quote - 1);
            
            // Extraer valor (después de los dos puntos)
            std::string valor_str = linea.substr(pos_colon + 1);
            
            // Limpiar espacios y comas
            valor_str.erase(0, valor_str.find_first_not_of(" \t"));
            valor_str.erase(valor_str.find_last_not_of(" ,\t\n\r") + 1);
            
            // Asignar valores
            if (clave == "capacidad_cola") {
                config.capacidad_cola = std::stoi(valor_str);
            } else if (clave == "num_clientes") {
                config.num_clientes = std::stoi(valor_str);
            } else if (clave == "num_motores") {
                config.num_motores = std::stoi(valor_str);
            } else if (clave == "num_analistas") {
                config.num_analistas = std::stoi(valor_str);
            } else if (clave == "num_administradores") {
                config.num_administradores = std::stoi(valor_str);
            } else if (clave == "duracion_segundos") {
                config.duracion_segundos = std::stoi(valor_str);
            } else if (clave == "demo_monitor") {
                config.demo_monitor = (valor_str == "true");
            } else if (clave == "demo_deadlock") {
                config.demo_deadlock = (valor_str == "true");
            }
        }
        
        std::cout << "[CONFIG] Configuración cargada desde " << archivo << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[CONFIG] Error al cargar configuración: " << e.what() << std::endl;
        std::cout << "[CONFIG] Usando configuración por defecto." << std::endl;
    }
    
    return config;
}

// Función para mostrar la configuración
void mostrar_configuracion(const ConfiguracionApp& config) {
    std::cout << "\n========== CONFIGURACIÓN ==========" << std::endl;
    std::cout << "Capacidad de cola: " << config.capacidad_cola << std::endl;
    std::cout << "Número de clientes: " << config.num_clientes << std::endl;
    std::cout << "Número de motores: " << config.num_motores << std::endl;
    std::cout << "Número de analistas: " << config.num_analistas << std::endl;
    std::cout << "Número de administradores: " << config.num_administradores << std::endl;
    std::cout << "Duración: " << config.duracion_segundos << " segundos" << std::endl;
    std::cout << "Demo Monitor: " << (config.demo_monitor ? "Sí" : "No") << std::endl;
    std::cout << "Demo Deadlock: " << (config.demo_deadlock ? "Sí" : "No") << std::endl;
    std::cout << "===================================\n" << std::endl;
}

// Demo del Monitor de Cuentas
void demo_monitor() {
    std::cout << "\n============================================" << std::endl;
    std::cout << "DEMO: MONITOR DE CUENTAS" << std::endl;
    std::cout << "============================================\n" << std::endl;
    
    auto monitor = std::make_shared<MonitorCuentas>();
    
    monitor->mostrar_estado();
    
    // Crear hilos que realizan operaciones
    std::thread t1([monitor]() {
        for (int i = 0; i < 3; ++i) {
            monitor->transferir("CTA-001", "CTA-002", 500.0);
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
        }
    });
    
    std::thread t2([monitor]() {
        for (int i = 0; i < 3; ++i) {
            monitor->transferir("CTA-003", "CTA-004", 1000.0);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    
    std::thread t3([monitor]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        monitor->depositar("CTA-001", 2000.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        monitor->depositar("CTA-003", 1500.0);
    });
    
    t1.join();
    t2.join();
    t3.join();
    
    monitor->mostrar_estado();
    
    std::cout << "============================================\n" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "================================================" << std::endl;
    std::cout << "  SIMULADOR DE SISTEMA BANCARIO CONCURRENTE" << std::endl;
    std::cout << "  Gestión de Transacciones con Concurrencia" << std::endl;
    std::cout << "================================================\n" << std::endl;
    
    // 1. Cargar configuración
    std::string archivo_config = "config.json";
    if (argc > 1) {
        archivo_config = argv[1];
    }
    
    ConfiguracionApp config = cargar_configuracion(archivo_config);
    mostrar_configuracion(config);
    
    // 2. Bandera atómica para controlar el ciclo de vida de todos los hilos
    std::atomic<bool> activo(true);
    
    // 3. Crear objetos compartidos
    
    // Semáforo para limitar motores concurrentes (máximo 3)
    Semaforo semaforo(config.num_motores);  // Usar valor de configuración
    
    // Cola de transacciones (buffer limitado)
    auto cola = std::make_shared<ColaTransacciones>(config.capacidad_cola);
    
    // Sistema de configuración (lectores-escritores)
    auto sistema_config = std::make_shared<ConfiguracionSistema>();
    // Creamos una unica instancia de ContextoFraude compartida
    auto contexto_fraude_compartido = std::make_shared<ContextoFraude>();
    
    std::cout << "[MAIN] Iniciando hilos...\n" << std::endl;
    
    // 4. Crear y lanzar hilos
    
    std::vector<std::thread> hilos;
    
    // --- Productores: Clientes ---
    for (int i = 0; i < config.num_clientes; ++i) {
        hilos.emplace_back([i, cola, &activo]() {
            Cliente cliente("CLI-" + std::to_string(i + 1), cola, activo, 1000 + i * 200);
            cliente.ejecutar();
        });
    }
    
    // --- Consumidores: Motores Antifraude ---
    for (int i = 0; i < config.num_motores; ++i) {
        // Pasamos el puntero al contexto comrtido a cada motor
        hilos.emplace_back([i, cola, &activo, &semaforo, contexto_fraude_compartido]() {
            MotorAntifraude motor(i + 1, cola, activo, semaforo, contexto_fraude_compartido, 1500);
            motor.ejecutar();
        });
    }
    
    // --- Lectores: Analistas Financieros ---
    for (int i = 0; i < config.num_analistas; ++i) {
        hilos.emplace_back([i, sistema_config, &activo]() {
            AnalistaFinanciero analista(i + 1, sistema_config, activo, 2000 + i * 300);
            analista.ejecutar();
        });
    }
    
    // --- Escritores: Administradores ---
    for (int i = 0; i < config.num_administradores; ++i) {
        hilos.emplace_back([i, sistema_config, &activo]() {
            AdministradorSistema admin(i + 1, sistema_config, activo, 5000);
            admin.ejecutar();
        });
    }
    
    std::cout << "[MAIN] Todos los hilos lanzados (" << hilos.size() << " hilos)" << std::endl;
    std::cout << "[MAIN] Simulación en progreso...\n" << std::endl;
    
    // 5. Ejecutar la simulación durante el tiempo configurado
    std::this_thread::sleep_for(std::chrono::seconds(config.duracion_segundos));
    
    // 6. Apagado ordenado
    std::cout << "\n[MAIN] Iniciando apagado ordenado..." << std::endl;
    activo = false;  // Señalar a todos los hilos que deben terminar
    
    // Cerrar la cola para desbloquear a los consumidores que estén esperando
    cola->cerrar();
    
    std::cout << "[MAIN] Esperando a que todos los hilos finalicen..." << std::endl;
    
    // Esperar a que todos los hilos terminen
    for (auto& hilo : hilos) {
        if (hilo.joinable()) {
            hilo.join();
        }
    }
    
    std::cout << "\n[MAIN] Todos los hilos finalizados." << std::endl;
    
    // 7. Demos opcionales
    
    if (config.demo_monitor) {
        demo_monitor();
    }
    
    if (config.demo_deadlock) {
        std::cout << "\n[MAIN] ⚠️  ADVERTENCIA: La demo de deadlock puede congelar el programa." << std::endl;
        std::cout << "[MAIN] Se recomienda ejecutar solo la resolución de deadlock." << std::endl;
        std::cout << "\n¿Desea ejecutar la demo de PROVOCAR deadlock? (puede requerir Ctrl+C)" << std::endl;
        std::cout << "Descomente la línea correspondiente en main.cpp para ejecutarla.\n" << std::endl;
        
        // provocar_deadlock();  // ⚠️ DESCOMENTAR BAJO TU PROPIO RIESGO
        resolver_deadlock();
    }
    
    // 8. Resumen final
    std::cout << "\n================================================" << std::endl;
    std::cout << "  SIMULACIÓN FINALIZADA" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "Resumen:" << std::endl;
    std::cout << "- Cola de transacciones: Implementada con mutex y condition_variable" << std::endl;
    std::cout << "- Productor-Consumidor: Clientes y Motores Antifraude" << std::endl;
    std::cout << "- Lectores-Escritores: Analistas y Administradores" << std::endl;
    std::cout << "- Semáforo: Limitación de motores concurrentes (máx. 3)" << std::endl;
    std::cout << "- Monitor: Gestión de cuentas bancarias (demo opcional)" << std::endl;
    std::cout << "- Deadlock: Demostración de prevención (demo opcional)" << std::endl;
    std::cout << "================================================\n" << std::endl;
    
    return 0;
}
