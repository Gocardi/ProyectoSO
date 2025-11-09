#ifndef SIMULADOR_INTERACTIVO_HPP
#define SIMULADOR_INTERACTIVO_HPP

#include "productor_consumidor.hpp"
#include "lectores_escritores.hpp"
#include "monitor.hpp"
#include <memory>
#include <atomic>
#include <string>
#include <map>

// Simulador interactivo que permite control manual
class SimuladorInteractivo {
private:
    std::shared_ptr<ColaTransacciones> cola;
    std::shared_ptr<ConfiguracionSistema> config;
    std::shared_ptr<MonitorCuentas> monitor;
    std::atomic<bool> activo;
    std::atomic<bool> pausado;
    
    // Estadísticas en tiempo real
    std::atomic<int> transacciones_procesadas{0};
    std::atomic<int> transacciones_sospechosas{0};
    std::atomic<int> transacciones_aprobadas{0};
    
    // Cuentas de usuarios (simuladas en memoria)
    std::map<std::string, std::string> usuarios; // username -> cuenta_id
    
public:
    SimuladorInteractivo();
    
    // Iniciar/detener simulación automática
    void iniciar_modo_automatico(int duracion_segundos);
    void detener();
    void pausar();
    void reanudar();
    
    // Envío manual de transacciones
    bool enviar_transaccion_manual(const std::string& usuario_origen,
                                   const std::string& usuario_destino,
                                   double monto,
                                   const std::string& tipo);
    
    // Consultas
    double consultar_saldo(const std::string& usuario);
    void mostrar_estado_completo();
    void mostrar_cuentas();
    void mostrar_estadisticas();
    
    // Gestión de usuarios
    void crear_usuario(const std::string& nombre, double saldo_inicial);
    std::vector<std::string> listar_usuarios();
    
    // Getters para estadísticas
    int get_transacciones_procesadas() const { return transacciones_procesadas; }
    int get_transacciones_sospechosas() const { return transacciones_sospechosas; }
    int get_transacciones_aprobadas() const { return transacciones_aprobadas; }
    size_t get_tamanio_cola() const { return cola->tamanio(); }
    bool esta_activo() const { return activo; }
    bool esta_pausado() const { return pausado; }
};

// Menú interactivo en consola
void mostrar_menu_principal();
void ejecutar_simulador_interactivo();

#endif // SIMULADOR_INTERACTIVO_HPP
