#ifndef PRODUCTOR_CONSUMIDOR_HPP
#define PRODUCTOR_CONSUMIDOR_HPP

#include "modelos.hpp"
#include "semaforo.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

// Cola thread-safe con tamaño limitado (buffer limitado)
class ColaTransacciones {
private:
    std::queue<Transaccion> cola;
    size_t capacidad_maxima;
    mutable std::mutex mtx;  // mutable permite bloquear en métodos const
    std::condition_variable cv_productor;  // Para cuando la cola está llena
    std::condition_variable cv_consumidor; // Para cuando la cola está vacía
    std::atomic<bool> cerrada{false};  // Para indicar que no habrá más datos
    
public:
    ColaTransacciones(size_t capacidad);
    
    // Productor: Añade una transacción a la cola (bloquea si está llena)
    void producir(const Transaccion& transaccion);
    
    // Consumidor: Extrae una transacción de la cola (bloquea si está vacía)
    Transaccion consumir();
    
    // Cierra la cola (desbloquea a todos los consumidores)
    void cerrar();
    
    // Métodos de consulta
    size_t tamanio() const;
    bool esta_vacia() const;
    bool esta_llena() const;
};

// Productor: Cliente que genera transacciones
class Cliente {
private:
    std::string id;
    std::shared_ptr<ColaTransacciones> cola;
    std::atomic<bool>& activo;
    int delay_ms;
    
public:
    Cliente(std::string id, std::shared_ptr<ColaTransacciones> cola,
            std::atomic<bool>& activo, int delay_ms = 1000);
    
    void ejecutar();
};

// Consumidor: Motor antifraude que procesa transacciones
class MotorAntifraude {
private:
    int id;
    std::shared_ptr<ColaTransacciones> cola;
    std::atomic<bool>& activo;
    Semaforo& semaforo;  // Limita acceso concurrente (máximo 3)
    int delay_ms;
    
public:
    MotorAntifraude(int id, std::shared_ptr<ColaTransacciones> cola,
                    std::atomic<bool>& activo, 
                    Semaforo& semaforo,
                    int delay_ms = 1500);
    
    void ejecutar();
    
private:
    bool analizar_transaccion(const Transaccion& t);
};

#endif // PRODUCTOR_CONSUMIDOR_HPP
