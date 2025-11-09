#include "productor_consumidor.hpp"
#include <iostream>
#include <thread>
#include <random>
#include <iomanip>
#include <mutex>
#include <condition_variable>

// ============================================================================
// ColaTransacciones - Buffer limitado thread-safe
// ============================================================================

ColaTransacciones::ColaTransacciones(size_t capacidad) 
    : capacidad_maxima(capacidad) {}

void ColaTransacciones::producir(const Transaccion& transaccion) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Esperar si la cola está llena
    cv_productor.wait(lock, [this]() { 
        return cola.size() < capacidad_maxima; 
    });
    
    cola.push(transaccion);
    std::cout << "[PRODUCTOR] Cliente " << transaccion.cliente_id 
              << " añadió transacción #" << transaccion.id 
              << " | Cola: " << cola.size() << "/" << capacidad_maxima << std::endl;
    
    // Notificar a los consumidores que hay datos disponibles
    cv_consumidor.notify_one();
}

Transaccion ColaTransacciones::consumir() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Esperar si la cola está vacía
    cv_consumidor.wait(lock, [this]() { 
        return !cola.empty(); 
    });
    
    Transaccion t = cola.front();
    cola.pop();
    
    // Notificar a los productores que hay espacio disponible
    cv_productor.notify_one();
    
    return t;
}

size_t ColaTransacciones::tamanio() const {
    std::lock_guard<std::mutex> lock(mtx);
    return cola.size();
}

bool ColaTransacciones::esta_vacia() const {
    std::lock_guard<std::mutex> lock(mtx);
    return cola.empty();
}

bool ColaTransacciones::esta_llena() const {
    std::lock_guard<std::mutex> lock(mtx);
    return cola.size() >= capacidad_maxima;
}

// ============================================================================
// Cliente - Productor de transacciones
// ============================================================================

Cliente::Cliente(std::string id, std::shared_ptr<ColaTransacciones> cola,
                 std::atomic<bool>& activo, int delay_ms)
    : id(id), cola(cola), activo(activo), delay_ms(delay_ms) {}

void Cliente::ejecutar() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist_monto(100.0, 10000.0);
    std::uniform_int_distribution<> dist_tipo(0, 2);
    
    int transaccion_id = 0;
    std::vector<std::string> tipos = {"TRANSFERENCIA", "RETIRO", "DEPOSITO"};
    
    while (activo) {
        // Generar transacción aleatoria
        transaccion_id++;
        double monto = dist_monto(gen);
        std::string tipo = tipos[dist_tipo(gen)];
        
        Transaccion t(transaccion_id, id, tipo, monto, 
                      "CTA-" + id, "CTA-DEST-" + std::to_string(transaccion_id));
        
        // Producir (añadir a la cola)
        cola->producir(t);
        
        // Esperar antes de generar otra transacción
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    
    std::cout << "[CLIENTE] Cliente " << id << " finalizó." << std::endl;
}

// ============================================================================
// MotorAntifraude - Consumidor de transacciones
// ============================================================================

MotorAntifraude::MotorAntifraude(int id, std::shared_ptr<ColaTransacciones> cola,
                                 std::atomic<bool>& activo,
                                 Semaforo& semaforo,
                                 int delay_ms)
    : id(id), cola(cola), activo(activo), semaforo(semaforo), delay_ms(delay_ms) {}

void MotorAntifraude::ejecutar() {
    while (activo) {
        try {
            // Adquirir permiso del semáforo (máximo 3 motores concurrentes)
            semaforo.acquire();
            
            // Consumir transacción de la cola
            Transaccion t = cola->consumir();
            
            std::cout << "[CONSUMIDOR] Motor #" << id 
                      << " procesando transacción #" << t.id 
                      << " de cliente " << t.cliente_id << std::endl;
            
            // Analizar la transacción
            bool es_sospechosa = analizar_transaccion(t);
            
            if (es_sospechosa) {
                std::cout << "[ALERTA] Motor #" << id 
                          << " detectó transacción sospechosa #" << t.id 
                          << " | Monto: $" << std::fixed << std::setprecision(2) 
                          << t.monto << std::endl;
            } else {
                std::cout << "[OK] Motor #" << id 
                          << " aprobó transacción #" << t.id << std::endl;
            }
            
            // Simular tiempo de procesamiento
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            
            // Liberar permiso del semáforo
            semaforo.release();
            
        } catch (const std::exception& e) {
            semaforo.release();
            if (activo) {
                std::cerr << "[ERROR] Motor #" << id << ": " << e.what() << std::endl;
            }
        }
    }
    
    std::cout << "[MOTOR] Motor Antifraude #" << id << " finalizó." << std::endl;
}

bool MotorAntifraude::analizar_transaccion(const Transaccion& t) {
    // Lógica simple de detección de fraude:
    // - Transacciones mayores a $8000 son sospechosas
    // - Transacciones de tipo RETIRO mayores a $5000 son sospechosas
    
    if (t.monto > 8000.0) {
        return true;
    }
    
    if (t.tipo == "RETIRO" && t.monto > 5000.0) {
        return true;
    }
    
    return false;
}
