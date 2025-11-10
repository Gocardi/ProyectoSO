#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <map>
#include <string>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <iomanip>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>

// SOLUCIÓN: Usar puntero a mutex en lugar de mutex directo
struct Cuenta {
    double saldo;
    std::unique_ptr<std::mutex> mtx;

    // Constructor por defecto
    Cuenta() : saldo(0.0), mtx(std::make_unique<std::mutex>()) {}

    // Constructor con saldo inicial
    explicit Cuenta(double s) : saldo(s), mtx(std::make_unique<std::mutex>()) {}

    // Deshabilitar copia (por el mutex)
    Cuenta(const Cuenta&) = delete;
    Cuenta& operator=(const Cuenta&) = delete;

    // Habilitar movimiento
    Cuenta(Cuenta&&) = default;
    Cuenta& operator=(Cuenta&&) = default;
};

class MonitorCuentas {
private:
    std::map<std::string, Cuenta> cuentas;
    std::mutex mtx_global;
    std::condition_variable cv;
    std::atomic<bool> modo_deadlock_activo{false};

public:
    MonitorCuentas();

    // Crea una cuenta con saldo inicial
    void crear_cuenta(const std::string& cuenta_id, double saldo_inicial);

    // Consulta el saldo de una cuenta
    double consultar_saldo(const std::string& cuenta_id);

    // Transferencia segura (sin deadlock)
    bool transferir(const std::string& origen, const std::string& destino, double monto);

    // Depositar / Retirar
    void depositar(const std::string& cuenta_id, double monto);
    bool retirar(const std::string& cuenta_id, double monto);

    // Mostrar estado
    void mostrar_estado();

    // --- Modo Deadlock ---
    void activar_deadlock();
    void resolver_deadlock();
    void transferir_con_deadlock(const std::string& origen, const std::string& destino, double monto);
};

#endif // MONITOR_HPP
