#ifndef DEADLOCK_HPP
#define DEADLOCK_HPP

#include <mutex>

// Funciones de demostración de deadlock

// DEMO 1: Provoca un deadlock intencionalmente
// Dos hilos intentan bloquear dos mutex en orden inverso
void provocar_deadlock();

// DEMO 2: Resuelve el deadlock usando std::scoped_lock (C++17)
// Adquiere múltiples locks de forma atómica
void resolver_deadlock();

#endif // DEADLOCK_HPP
