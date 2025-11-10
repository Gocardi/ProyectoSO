#ifndef PRODUCTOR_CONSUMIDOR_HPP
#define PRODUCTOR_CONSUMIDOR_HPP

#include "modelos.hpp"
#include "semaforo.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>
#include <chrono>
#include <map>         
#include <vector>      
#include <algorithm>   

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
    size_t obtener_tamanio() const; // Alias para Qt
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

class ContextoFraude {
private:
    mutable std::mutex mtx;
    std::map<std::string, std::chrono::steady_clock::time_point> ultima_transaccion_usuario;
    std::map<std::string, std::vector<std::chrono::steady_clock::time_point>> historial_transacciones_recientes;

public:
    ContextoFraude() = default;

    // Analiza una transacción y actualiza el contexto de forma atómica.
    // Devuelve 'true' si la transacción es sospechosa.
    bool analizarYActualizar(const Transaccion& t) {
        // Bloqueo exclusivo para garantizar la atomicidad de la operación
        std::lock_guard<std::mutex> lock(mtx);

        bool es_sospechosa = false;
        const std::string& usuario = t.cliente_id;
        auto ahora = std::chrono::steady_clock::now();

        // --- REGLA 1: Velocidad ---
        // Si hay un registro previo para este usuario
        if (ultima_transaccion_usuario.count(usuario)) {
            auto ultima_vez = ultima_transaccion_usuario.at(usuario);
            auto duracion = std::chrono::duration_cast<std::chrono::seconds>(ahora - ultima_vez);
            if (duracion.count() < 20) {
                es_sospechosa = true;
            }
        }

        // --- REGLA 2: Frecuencia ---
        auto& historial = historial_transacciones_recientes[usuario];
        
        // Eliminar timestamps más antiguos que 1 minuto
        auto un_minuto_atras = ahora - std::chrono::minutes(1);
        historial.erase(
            std::remove_if(historial.begin(), historial.end(), 
                           [&](const auto& timestamp){ return timestamp < un_minuto_atras; }),
            historial.end()
        );

        // Si después de limpiar, hay más de 5 transacciones, es sospechoso
        if (historial.size() >= 5) {
            es_sospechosa = true;
        }

        // --- Actualizar el estado con la transacción actual ---
        ultima_transaccion_usuario[usuario] = ahora;
        historial.push_back(ahora);

        return es_sospechosa;
    }
};


// Consumidor: Motor antifraude que procesa transacciones
class MotorAntifraude {
private:
    int id;
    std::shared_ptr<ColaTransacciones> cola;
    std::atomic<bool>& activo;
    Semaforo& semaforo;  // Limita acceso concurrente (máximo 3)
    int delay_ms;

    std::shared_ptr<ContextoFraude> contexto_fraude; // Puntero  al contexto de fraude compartido
    
public:
    MotorAntifraude(int id, std::shared_ptr<ColaTransacciones> cola,
                    std::atomic<bool>& activo, 
                    Semaforo& semaforo,
                    std::shared_ptr<ContextoFraude> contexto, // El nuevo parámetro
                    int delay_ms = 1500);
    
    void ejecutar();
    
private:
    // La logica ya no está aquí, se ha movido a ContextoFraude
    //bool analizar_transaccion(const Transaccion& t);
};

#endif // PRODUCTOR_CONSUMIDOR_HPP
