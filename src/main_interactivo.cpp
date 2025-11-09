#include "simulador_interactivo.hpp"
#include <iostream>

int main() {
    std::cout << "\n";
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    std::cout << "        🏦 SISTEMA BANCARIO INTERACTIVO                 " << std::endl;
    std::cout << "        Versión con Control Manual                      " << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    std::cout << "\n";
    std::cout << "Características:" << std::endl;
    std::cout << "  ✅ Envía transacciones entre usuarios" << std::endl;
    std::cout << "  ✅ Ve saldos en tiempo real" << std::endl;
    std::cout << "  ✅ Crea nuevos usuarios" << std::endl;
    std::cout << "  ✅ Estadísticas de transacciones" << std::endl;
    std::cout << "  ✅ Sin internet - Todo local" << std::endl;
    std::cout << "\n";
    std::cout << "Presiona Enter para comenzar...";
    std::cin.get();
    
    ejecutar_simulador_interactivo();
    
    return 0;
}
