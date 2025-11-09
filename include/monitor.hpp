#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <map>
#include <string>
#include <mutex>
#include <condition_variable>

// Monitor: Gestiona cuentas bancarias con acceso seguro
// Encapsula el estado (saldos) y sincroniza el acceso mediante mutex
class MonitorCuentas {
private:
    std::map<std::string, double> saldos;
    std::mutex mtx;
    std::condition_variable cv;
    
public:
    MonitorCuentas();
    
    // Crea una cuenta con saldo inicial
    void crear_cuenta(const std::string& cuenta_id, double saldo_inicial);
    
    // Consulta el saldo de una cuenta
    double consultar_saldo(const std::string& cuenta_id);
    
    // Realiza una transferencia entre dos cuentas
    // Bloquea si no hay fondos suficientes
    bool transferir(const std::string& origen, const std::string& destino, 
                   double monto);
    
    // Deposita dinero en una cuenta
    void depositar(const std::string& cuenta_id, double monto);
    
    // Retira dinero de una cuenta (si hay fondos)
    bool retirar(const std::string& cuenta_id, double monto);
    
    // Muestra el estado de todas las cuentas
    void mostrar_estado();
};

#endif // MONITOR_HPP
