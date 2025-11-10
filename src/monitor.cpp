#include "monitor.hpp"

MonitorCuentas::MonitorCuentas() {
    // Inicializar cuentas de prueba usando emplace
    cuentas.emplace("CTA-001", Cuenta(5000.0));
    cuentas.emplace("CTA-002", Cuenta(3000.0));
    cuentas.emplace("CTA-003", Cuenta(10000.0));
    cuentas.emplace("CTA-004", Cuenta(2000.0));
}

void MonitorCuentas::crear_cuenta(const std::string& cuenta_id, double saldo_inicial) {
    std::lock_guard<std::mutex> lock(mtx_global);

    if (cuentas.find(cuenta_id) == cuentas.end()) {
        cuentas.emplace(cuenta_id, Cuenta(saldo_inicial));
        std::cout << "[MONITOR] Cuenta " << cuenta_id
                  << " creada con saldo: $" << std::fixed << std::setprecision(2)
                  << saldo_inicial << std::endl;
    } else {
        std::cout << "[MONITOR] La cuenta " << cuenta_id << " ya existe\n";
    }
}

double MonitorCuentas::consultar_saldo(const std::string& cuenta_id) {
    std::lock_guard<std::mutex> lock(mtx_global);
    auto it = cuentas.find(cuenta_id);
    if (it != cuentas.end()) return it->second.saldo;
    std::cerr << "[MONITOR] Cuenta " << cuenta_id << " no existe\n";
    return -1.0;
}

bool MonitorCuentas::transferir(const std::string& origen,
                                const std::string& destino,
                                double monto) {
    std::unique_lock<std::mutex> lock(mtx_global);

    // Verificar que ambas cuentas existen
    if (cuentas.find(origen) == cuentas.end() ||
        cuentas.find(destino) == cuentas.end()) {
        std::cerr << "[MONITOR] Una de las cuentas no existe" << std::endl;
        return false;
    }

    // Esperar hasta que haya fondos o timeout (para detectar deadlock)
    if (!cv.wait_for(lock, std::chrono::seconds(3), [this, &origen, monto]() {
            return cuentas[origen].saldo >= monto;
        })) {
        std::cerr << "[MONITOR] ⚠ Timeout detectado (posible deadlock) entre "
                  << origen << " y " << destino << std::endl;
        return false;
    }

    // Si sale del bloqueo, se hace la transferencia
    cuentas[origen].saldo -= monto;
    cuentas[destino].saldo += monto;

    std::cout << "[MONITOR] Transferencia: " << origen << " -> " << destino
              << " | Monto: $" << std::fixed << std::setprecision(2) << monto
              << " | Saldo origen: $" << cuentas[origen].saldo
              << " | Saldo destino: $" << cuentas[destino].saldo << std::endl;

    cv.notify_all();
    return true;
}

void MonitorCuentas::depositar(const std::string& cuenta_id, double monto) {
    std::lock_guard<std::mutex> lock(mtx_global);

    auto it = cuentas.find(cuenta_id);
    if (it == cuentas.end()) {
        std::cerr << "[MONITOR] Cuenta " << cuenta_id << " no existe\n";
        return;
    }

    it->second.saldo += monto;
    std::cout << "[MONITOR] Depósito en " << cuenta_id << ": $" << monto
              << " | Nuevo saldo: $" << it->second.saldo << std::endl;
}

bool MonitorCuentas::retirar(const std::string& cuenta_id, double monto) {
    std::lock_guard<std::mutex> lock(mtx_global);

    auto it = cuentas.find(cuenta_id);
    if (it == cuentas.end()) {
        std::cerr << "[MONITOR] Cuenta " << cuenta_id << " no existe\n";
        return false;
    }

    if (it->second.saldo < monto) {
        std::cerr << "[MONITOR] Fondos insuficientes en " << cuenta_id << std::endl;
        return false;
    }

    it->second.saldo -= monto;
    std::cout << "[MONITOR] Retiro de " << cuenta_id << ": $" << monto
              << " | Nuevo saldo: $" << it->second.saldo << std::endl;
    return true;
}

void MonitorCuentas::mostrar_estado() {
    std::lock_guard<std::mutex> lock(mtx_global);
    std::cout << "\n========== ESTADO DE CUENTAS ==========\n";
    for (const auto &[id, cuenta] : cuentas)
        std::cout << id << ": $" << std::fixed << std::setprecision(2)
                  << cuenta.saldo << std::endl;
    std::cout << "========================================\n\n";
}

void MonitorCuentas::activar_deadlock() {
    modo_deadlock_activo = true;
    std::cout << "\n[MONITOR] ⚠️ Modo Deadlock ACTIVADO. Se crearán bloqueos cruzados.\n";
}

void MonitorCuentas::resolver_deadlock() {
    modo_deadlock_activo = false;
    std::cout << "\n[MONITOR] ✅ Deadlock RESUELTO (modo desactivado).\n";
}

void MonitorCuentas::transferir_con_deadlock(const std::string& origen,
                                             const std::string& destino,
                                             double monto) {
    std::thread([this, origen, destino, monto]() {
        auto &c1 = cuentas.at(origen);
        auto &c2 = cuentas.at(destino);

        // Orden aleatorio para simular interbloqueo
        bool invertir = (rand() % 2 == 0);

        if (invertir) {
            std::unique_lock<std::mutex> l2(*c2.mtx);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::unique_lock<std::mutex> l1(*c1.mtx);
        } else {
            std::unique_lock<std::mutex> l1(*c1.mtx);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::unique_lock<std::mutex> l2(*c2.mtx);
        }

        if (!modo_deadlock_activo) {
            // Solo se completará si el modo ya fue "resuelto"
            c1.saldo -= monto;
            c2.saldo += monto;
            std::cout << "[MONITOR] Transferencia (resuelta): " << origen << " → " << destino
                      << " | $" << monto << std::endl;
        }
    }).detach();
}
