#ifndef SEMAFORO_HPP
#define SEMAFORO_HPP

#include <mutex>
#include <condition_variable>

// Implementación de semáforo contador para C++17
// (std::counting_semaphore está disponible en C++20)
class Semaforo {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int contador;
    const int max_contador;
    
public:
    explicit Semaforo(int count) 
        : contador(count), max_contador(count) {}
    
    // Adquiere un permiso (decrementa el contador)
    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() { return contador > 0; });
        --contador;
    }
    
    // Libera un permiso (incrementa el contador)
    void release() {
        std::lock_guard<std::mutex> lock(mtx);
        if (contador < max_contador) {
            ++contador;
            cv.notify_one();
        }
    }
    
    // Intenta adquirir sin bloquear
    bool try_acquire() {
        std::lock_guard<std::mutex> lock(mtx);
        if (contador > 0) {
            --contador;
            return true;
        }
        return false;
    }
};

#endif // SEMAFORO_HPP
