#include "deadlock.hpp"
#include <iostream>
#include <thread>
#include <chrono>

// ============================================================================
// DEMO DE DEADLOCK
// ============================================================================

// Dos mutex globales para las demostraciones
static std::mutex mutex_A;
static std::mutex mutex_B;

// ----------------------------------------------------------------------------
// DEMO 1: Provocar Deadlock
// ----------------------------------------------------------------------------

void provocar_deadlock() {
    std::cout << "\n============================================" << std::endl;
    std::cout << "DEMO 1: PROVOCANDO DEADLOCK" << std::endl;
    std::cout << "============================================\n" << std::endl;
    
    std::cout << "Iniciando dos hilos que bloquearán mutex en orden inverso..." << std::endl;
    std::cout << "ADVERTENCIA: Este programa se CONGELARÁ en deadlock." << std::endl;
    std::cout << "Usa Ctrl+C para terminar el programa si se congela.\n" << std::endl;
    
    // Hilo 1: Bloquea A, luego B
    std::thread hilo1([]() {
        std::cout << "[HILO 1] Intentando bloquear MUTEX A..." << std::endl;
        std::lock_guard<std::mutex> lock_a(mutex_A);
        std::cout << "[HILO 1] MUTEX A bloqueado ✓" << std::endl;
        
        // Simular trabajo
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "[HILO 1] Intentando bloquear MUTEX B..." << std::endl;
        std::lock_guard<std::mutex> lock_b(mutex_B);
        std::cout << "[HILO 1] MUTEX B bloqueado ✓" << std::endl;
        
        std::cout << "[HILO 1] Trabajo completado." << std::endl;
    });
    
    // Hilo 2: Bloquea B, luego A (ORDEN INVERSO -> DEADLOCK)
    std::thread hilo2([]() {
        std::cout << "[HILO 2] Intentando bloquear MUTEX B..." << std::endl;
        std::lock_guard<std::mutex> lock_b(mutex_B);
        std::cout << "[HILO 2] MUTEX B bloqueado ✓" << std::endl;
        
        // Simular trabajo
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::cout << "[HILO 2] Intentando bloquear MUTEX A..." << std::endl;
        std::lock_guard<std::mutex> lock_a(mutex_A);
        std::cout << "[HILO 2] MUTEX A bloqueado ✓" << std::endl;
        
        std::cout << "[HILO 2] Trabajo completado." << std::endl;
    });
    
    // Esperar a que terminen (nunca terminarán si hay deadlock)
    std::cout << "\nEsperando a que los hilos terminen..." << std::endl;
    std::cout << "(Si el programa se congela aquí, hay DEADLOCK)\n" << std::endl;
    
    // Timeout de 5 segundos
    auto inicio = std::chrono::steady_clock::now();
    bool hilo1_terminado = false;
    bool hilo2_terminado = false;
    
    while (true) {
        auto ahora = std::chrono::steady_clock::now();
        auto duracion = std::chrono::duration_cast<std::chrono::seconds>(ahora - inicio);
        
        if (duracion.count() > 5) {
            std::cout << "\n⚠️  DEADLOCK DETECTADO ⚠️" << std::endl;
            std::cout << "Los hilos están bloqueados esperando mutuamente." << std::endl;
            std::cout << "Hilo 1 tiene MUTEX A y espera MUTEX B" << std::endl;
            std::cout << "Hilo 2 tiene MUTEX B y espera MUTEX A" << std::endl;
            std::cout << "\nPara evitar finalizar el programa, no esperaremos más." << std::endl;
            
            // Detach para no bloquear el programa principal
            hilo1.detach();
            hilo2.detach();
            break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n============================================\n" << std::endl;
}

// ----------------------------------------------------------------------------
// DEMO 2: Resolver Deadlock con std::scoped_lock (C++17)
// ----------------------------------------------------------------------------

void resolver_deadlock() {
    std::cout << "\n============================================" << std::endl;
    std::cout << "DEMO 2: RESOLVIENDO DEADLOCK" << std::endl;
    std::cout << "============================================\n" << std::endl;
    
    std::cout << "Usando std::scoped_lock para bloquear múltiples mutex atómicamente." << std::endl;
    std::cout << "Esto previene deadlock independientemente del orden.\n" << std::endl;
    
    // Reiniciar mutex (pueden estar en estado indefinido del demo anterior)
    // Nota: En producción, sería mejor usar mutex nuevos
    
    // Hilo 1: Bloquea A y B usando scoped_lock
    std::thread hilo1([]() {
        std::cout << "[HILO 1] Intentando bloquear MUTEX A y B simultáneamente..." << std::endl;
        
        // std::scoped_lock adquiere AMBOS locks de forma atómica
        // Previene deadlock automáticamente
        std::scoped_lock lock(mutex_A, mutex_B);
        
        std::cout << "[HILO 1] MUTEX A y B bloqueados ✓" << std::endl;
        
        // Simular trabajo
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "[HILO 1] Trabajo completado. Liberando locks." << std::endl;
        // scoped_lock libera automáticamente al salir del scope
    });
    
    // Hilo 2: Bloquea B y A usando scoped_lock (orden diferente, pero sin deadlock)
    std::thread hilo2([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        std::cout << "[HILO 2] Intentando bloquear MUTEX B y A simultáneamente..." << std::endl;
        
        // Aunque el orden sea diferente, scoped_lock previene deadlock
        std::scoped_lock lock(mutex_B, mutex_A);
        
        std::cout << "[HILO 2] MUTEX B y A bloqueados ✓" << std::endl;
        
        // Simular trabajo
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "[HILO 2] Trabajo completado. Liberando locks." << std::endl;
    });
    
    // Esperar a que terminen
    hilo1.join();
    hilo2.join();
    
    std::cout << "\n✅ Ambos hilos completaron exitosamente." << std::endl;
    std::cout << "No hubo deadlock gracias a std::scoped_lock.\n" << std::endl;
    
    std::cout << "============================================\n" << std::endl;
}
