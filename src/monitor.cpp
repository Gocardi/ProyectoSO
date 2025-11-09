#include "monitor.hpp"
#include <iostream>
#include <iomanip>

// ============================================================================
// MonitorCuentas - Monitor para gestión de cuentas bancarias
// ============================================================================

MonitorCuentas::MonitorCuentas() {
    // Crear algunas cuentas de prueba
    saldos["CTA-001"] = 5000.0;
    saldos["CTA-002"] = 3000.0;
    saldos["CTA-003"] = 10000.0;
    saldos["CTA-004"] = 2000.0;
}

void MonitorCuentas::crear_cuenta(const std::string& cuenta_id, double saldo_inicial) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (saldos.find(cuenta_id) == saldos.end()) {
        saldos[cuenta_id] = saldo_inicial;
        std::cout << "[MONITOR] Cuenta " << cuenta_id 
                  << " creada con saldo: $" << std::fixed 
                  << std::setprecision(2) << saldo_inicial << std::endl;
    } else {
        std::cout << "[MONITOR] La cuenta " << cuenta_id 
                  << " ya existe" << std::endl;
    }
}

double MonitorCuentas::consultar_saldo(const std::string& cuenta_id) {
    std::lock_guard<std::mutex> lock(mtx);
    
    auto it = saldos.find(cuenta_id);
    if (it != saldos.end()) {
        return it->second;
    }
    
    std::cerr << "[MONITOR] Cuenta " << cuenta_id << " no existe" << std::endl;
    return -1.0;
}

bool MonitorCuentas::transferir(const std::string& origen, 
                                const std::string& destino, 
                                double monto) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Verificar que ambas cuentas existen
    if (saldos.find(origen) == saldos.end() || 
        saldos.find(destino) == saldos.end()) {
        std::cerr << "[MONITOR] Una de las cuentas no existe" << std::endl;
        return false;
    }
    
    // Esperar hasta que haya fondos suficientes en la cuenta origen
    cv.wait(lock, [this, &origen, monto]() {
        return saldos[origen] >= monto;
    });
    
    // Realizar la transferencia
    saldos[origen] -= monto;
    saldos[destino] += monto;
    
    std::cout << "[MONITOR] Transferencia: " << origen << " -> " << destino
              << " | Monto: $" << std::fixed << std::setprecision(2) << monto
              << " | Saldo origen: $" << saldos[origen]
              << " | Saldo destino: $" << saldos[destino] << std::endl;
    
    // Notificar a otros hilos que pueden estar esperando
    cv.notify_all();
    
    return true;
}

void MonitorCuentas::depositar(const std::string& cuenta_id, double monto) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (saldos.find(cuenta_id) == saldos.end()) {
        std::cerr << "[MONITOR] Cuenta " << cuenta_id << " no existe" << std::endl;
        return;
    }
    
    saldos[cuenta_id] += monto;
    
    std::cout << "[MONITOR] Depósito en " << cuenta_id 
              << " | Monto: $" << std::fixed << std::setprecision(2) << monto
              << " | Nuevo saldo: $" << saldos[cuenta_id] << std::endl;
    
    cv.notify_all();
}

bool MonitorCuentas::retirar(const std::string& cuenta_id, double monto) {
    std::lock_guard<std::mutex> lock(mtx);
    
    if (saldos.find(cuenta_id) == saldos.end()) {
        std::cerr << "[MONITOR] Cuenta " << cuenta_id << " no existe" << std::endl;
        return false;
    }
    
    if (saldos[cuenta_id] < monto) {
        std::cerr << "[MONITOR] Fondos insuficientes en " << cuenta_id << std::endl;
        return false;
    }
    
    saldos[cuenta_id] -= monto;
    
    std::cout << "[MONITOR] Retiro de " << cuenta_id 
              << " | Monto: $" << std::fixed << std::setprecision(2) << monto
              << " | Nuevo saldo: $" << saldos[cuenta_id] << std::endl;
    
    return true;
}

void MonitorCuentas::mostrar_estado() {
    std::lock_guard<std::mutex> lock(mtx);
    
    std::cout << "\n========== ESTADO DE CUENTAS ==========" << std::endl;
    for (const auto& [cuenta, saldo] : saldos) {
        std::cout << cuenta << ": $" << std::fixed 
                  << std::setprecision(2) << saldo << std::endl;
    }
    std::cout << "========================================\n" << std::endl;
}
