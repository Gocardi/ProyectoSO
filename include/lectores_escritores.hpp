#ifndef LECTORES_ESCRITORES_HPP
#define LECTORES_ESCRITORES_HPP

#include <map>
#include <string>
#include <shared_mutex>
#include <atomic>
#include <memory>

// Clase que gestiona la configuración del sistema con acceso concurrente
// Múltiples lectores pueden leer simultáneamente
// Un solo escritor puede escribir (bloqueando lectores y otros escritores)
class ConfiguracionSistema {
private:
    std::map<std::string, std::string> configuracion;
    mutable std::shared_mutex mtx;  // C++17 shared_mutex
    
public:
    ConfiguracionSistema();
    
    // Lectura: Permite múltiples lectores concurrentes
    std::string leer(const std::string& clave) const;
    std::map<std::string, std::string> leer_todo() const;
    
    // Escritura: Exclusiva (bloquea todo)
    void escribir(const std::string& clave, const std::string& valor);
    void actualizar_multiples(const std::map<std::string, std::string>& nuevos_valores);
};

// Lector: Analista Financiero que lee la configuración
class AnalistaFinanciero {
private:
    int id;
    std::shared_ptr<ConfiguracionSistema> config;
    std::atomic<bool>& activo;
    int delay_ms;
    
public:
    AnalistaFinanciero(int id, std::shared_ptr<ConfiguracionSistema> config,
                       std::atomic<bool>& activo, int delay_ms = 2000);
    
    void ejecutar();
};

// Escritor: Administrador que modifica la configuración
class AdministradorSistema {
private:
    int id;
    std::shared_ptr<ConfiguracionSistema> config;
    std::atomic<bool>& activo;
    int delay_ms;
    
public:
    AdministradorSistema(int id, std::shared_ptr<ConfiguracionSistema> config,
                         std::atomic<bool>& activo, int delay_ms = 5000);
    
    void ejecutar();
};

#endif // LECTORES_ESCRITORES_HPP
