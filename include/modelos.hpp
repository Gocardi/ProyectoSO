#ifndef MODELOS_HPP
#define MODELOS_HPP

#include <string>
#include <chrono>

// Estructura para representar una transacción bancaria
struct Transaccion {
    int id;
    std::string cliente_id;
    std::string tipo;  // "TRANSFERENCIA", "RETIRO", "DEPOSITO"
    double monto;
    std::string cuenta_origen;
    std::string cuenta_destino;
    std::chrono::system_clock::time_point timestamp;
    bool es_sospechosa;
    
    Transaccion() 
        : id(0), monto(0.0), es_sospechosa(false), 
          timestamp(std::chrono::system_clock::now()) {}
    
    Transaccion(int id, std::string cliente, std::string tipo, double monto,
                std::string origen = "", std::string destino = "")
        : id(id), cliente_id(cliente), tipo(tipo), monto(monto),
          cuenta_origen(origen), cuenta_destino(destino),
          es_sospechosa(false), 
          timestamp(std::chrono::system_clock::now()) {}
};

#endif // MODELOS_HPP
