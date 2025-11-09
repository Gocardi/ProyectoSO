#include "lectores_escritores.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <mutex>
#include <shared_mutex>

// ============================================================================
// ConfiguracionSistema - Gestión de configuración con lectores-escritores
// ============================================================================

ConfiguracionSistema::ConfiguracionSistema() {
    // Configuración inicial
    configuracion["limite_transaccion"] = "10000";
    configuracion["num_motores_antifraude"] = "3";
    configuracion["capacidad_cola"] = "10";
    configuracion["timeout_analisis"] = "5000";
    configuracion["modo_debug"] = "false";
}

std::string ConfiguracionSistema::leer(const std::string& clave) const {
    // Lock compartido (shared_lock): Múltiples lectores pueden leer simultáneamente
    std::shared_lock<std::shared_mutex> lock(mtx);
    
    auto it = configuracion.find(clave);
    if (it != configuracion.end()) {
        return it->second;
    }
    return "";
}

std::map<std::string, std::string> ConfiguracionSistema::leer_todo() const {
    std::shared_lock<std::shared_mutex> lock(mtx);
    return configuracion;
}

void ConfiguracionSistema::escribir(const std::string& clave, const std::string& valor) {
    // Lock exclusivo (unique_lock): Solo un escritor, bloquea lectores
    std::unique_lock<std::shared_mutex> lock(mtx);
    
    std::cout << "[CONFIG] Actualizando " << clave << ": " 
              << configuracion[clave] << " -> " << valor << std::endl;
    
    configuracion[clave] = valor;
    
    // Simular escritura más lenta
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void ConfiguracionSistema::actualizar_multiples(
    const std::map<std::string, std::string>& nuevos_valores) {
    std::unique_lock<std::shared_mutex> lock(mtx);
    
    std::cout << "[CONFIG] Actualizando múltiples valores..." << std::endl;
    for (const auto& [clave, valor] : nuevos_valores) {
        configuracion[clave] = valor;
    }
}

// ============================================================================
// AnalistaFinanciero - Lector
// ============================================================================

AnalistaFinanciero::AnalistaFinanciero(int id, 
                                       std::shared_ptr<ConfiguracionSistema> config,
                                       std::atomic<bool>& activo, 
                                       int delay_ms)
    : id(id), config(config), activo(activo), delay_ms(delay_ms) {}

void AnalistaFinanciero::ejecutar() {
    std::vector<std::string> claves = {
        "limite_transaccion", 
        "num_motores_antifraude", 
        "capacidad_cola",
        "timeout_analisis"
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, claves.size() - 1);
    
    while (activo) {
        // Leer una clave aleatoria
        std::string clave = claves[dist(gen)];
        std::string valor = config->leer(clave);
        
        std::cout << "[LECTOR] Analista #" << id 
                  << " leyó " << clave << " = " << valor << std::endl;
        
        // Los lectores pueden leer simultáneamente
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    std::cout << "[ANALISTA] Analista Financiero #" << id << " finalizó." << std::endl;
}

// ============================================================================
// AdministradorSistema - Escritor
// ============================================================================

AdministradorSistema::AdministradorSistema(int id,
                                           std::shared_ptr<ConfiguracionSistema> config,
                                           std::atomic<bool>& activo,
                                           int delay_ms)
    : id(id), config(config), activo(activo), delay_ms(delay_ms) {}

void AdministradorSistema::ejecutar() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist_valor(5000, 15000);
    
    std::vector<std::string> claves = {
        "limite_transaccion",
        "timeout_analisis"
    };
    
    int contador = 0;
    
    while (activo) {
        contador++;
        
        // Modificar configuración
        std::string clave = claves[contador % claves.size()];
        std::string nuevo_valor = std::to_string(dist_valor(gen));
        
        std::cout << "[ESCRITOR] Administrador #" << id 
                  << " va a modificar " << clave << std::endl;
        
        config->escribir(clave, nuevo_valor);
        
        std::cout << "[ESCRITOR] Administrador #" << id 
                  << " completó la modificación" << std::endl;
        
        // Los escritores escriben menos frecuentemente
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    std::cout << "[ADMIN] Administrador #" << id << " finalizó." << std::endl;
}
