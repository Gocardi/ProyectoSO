#include "simulador_interactivo.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <sstream>
#include <limits>

SimuladorInteractivo::SimuladorInteractivo() 
    : activo(false), pausado(false) {
    
    // Crear componentes compartidos
    cola = std::make_shared<ColaTransacciones>(10);
    config = std::make_shared<ConfiguracionSistema>();
    monitor = std::make_shared<MonitorCuentas>();
    
    // Crear usuarios de prueba
    crear_usuario("Juan", 10000.0);
    crear_usuario("Maria", 8000.0);
    crear_usuario("Pedro", 15000.0);
    crear_usuario("Ana", 5000.0);
    crear_usuario("Luis", 12000.0);
}

void SimuladorInteractivo::crear_usuario(const std::string& nombre, double saldo_inicial) {
    std::string cuenta_id = "CTA-" + nombre;
    usuarios[nombre] = cuenta_id;
    monitor->crear_cuenta(cuenta_id, saldo_inicial);
}

std::vector<std::string> SimuladorInteractivo::listar_usuarios() {
    std::vector<std::string> lista;
    for (const auto& [nombre, _] : usuarios) {
        lista.push_back(nombre);
    }
    return lista;
}

double SimuladorInteractivo::consultar_saldo(const std::string& usuario) {
    auto it = usuarios.find(usuario);
    if (it != usuarios.end()) {
        return monitor->consultar_saldo(it->second);
    }
    return -1.0;
}

bool SimuladorInteractivo::enviar_transaccion_manual(
    const std::string& usuario_origen,
    const std::string& usuario_destino,
    double monto,
    const std::string& tipo) {
    
    // Verificar que los usuarios existen
    auto it_origen = usuarios.find(usuario_origen);
    auto it_destino = usuarios.find(usuario_destino);
    
    if (it_origen == usuarios.end() || it_destino == usuarios.end()) {
        std::cerr << "\n❌ Error: Usuario no encontrado" << std::endl;
        return false;
    }
    
    // Verificar saldo suficiente
    double saldo_actual = monitor->consultar_saldo(it_origen->second);
    if (saldo_actual < monto) {
        std::cerr << "\n❌ Error: Saldo insuficiente" << std::endl;
        std::cerr << "   Saldo actual: $" << std::fixed << std::setprecision(2) 
                  << saldo_actual << std::endl;
        return false;
    }
    
    // Crear la transacción
    static int tx_id = 1000;
    Transaccion t(tx_id++, usuario_origen, tipo, monto, 
                  it_origen->second, it_destino->second);
    
    std::cout << "\n📤 Enviando transacción..." << std::endl;
    std::cout << "   De: " << usuario_origen << " (" << it_origen->second << ")" << std::endl;
    std::cout << "   A: " << usuario_destino << " (" << it_destino->second << ")" << std::endl;
    std::cout << "   Monto: $" << std::fixed << std::setprecision(2) << monto << std::endl;
    
    // Realizar la transferencia directamente en el monitor
    bool exito = monitor->transferir(it_origen->second, it_destino->second, monto);
    
    if (exito) {
        transacciones_procesadas++;
        
        // Verificar si es sospechosa
        if (monto > 8000.0 || (tipo == "RETIRO" && monto > 5000.0)) {
            transacciones_sospechosas++;
            std::cout << "\n⚠️  ALERTA: Transacción marcada como sospechosa" << std::endl;
        } else {
            transacciones_aprobadas++;
            std::cout << "\n✅ Transacción completada exitosamente" << std::endl;
        }
        
        return true;
    }
    
    return false;
}

void SimuladorInteractivo::mostrar_cuentas() {
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║       💰 CUENTAS Y SALDOS              ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    
    for (const auto& [nombre, cuenta_id] : usuarios) {
        double saldo = monitor->consultar_saldo(cuenta_id);
        std::cout << "  👤 " << std::left << std::setw(10) << nombre 
                  << " │ " << cuenta_id 
                  << " │ $" << std::fixed << std::setprecision(2) 
                  << std::right << std::setw(10) << saldo << std::endl;
    }
    std::cout << std::endl;
}

void SimuladorInteractivo::mostrar_estadisticas() {
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║       📊 ESTADÍSTICAS                  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << "  Total procesadas:     " << transacciones_procesadas << std::endl;
    std::cout << "  ✅ Aprobadas:         " << transacciones_aprobadas << std::endl;
    std::cout << "  ⚠️  Sospechosas:      " << transacciones_sospechosas << std::endl;
    std::cout << "  📦 En cola:           " << cola->tamanio() << "/10" << std::endl;
    std::cout << std::endl;
}

void SimuladorInteractivo::mostrar_estado_completo() {
    // Limpiar pantalla (compatible con Linux y Windows)
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    
    std::cout << "\n";
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    std::cout << "        🏦 SISTEMA BANCARIO INTERACTIVO                 " << std::endl;
    std::cout << "════════════════════════════════════════════════════════" << std::endl;
    
    mostrar_estadisticas();
    mostrar_cuentas();
}

// ============================================================================
// Menú Interactivo
// ============================================================================

void mostrar_menu_principal() {
    std::cout << "\n╔════════════════════════════════════════╗" << std::endl;
    std::cout << "║           MENÚ PRINCIPAL               ║" << std::endl;
    std::cout << "╠════════════════════════════════════════╣" << std::endl;
    std::cout << "║  1. 💸 Enviar transacción             ║" << std::endl;
    std::cout << "║  2. 💰 Ver cuentas y saldos           ║" << std::endl;
    std::cout << "║  3. 📊 Ver estadísticas               ║" << std::endl;
    std::cout << "║  4. 👤 Crear nuevo usuario            ║" << std::endl;
    std::cout << "║  5. 🔄 Actualizar pantalla            ║" << std::endl;
    std::cout << "║  6. ❌ Salir                          ║" << std::endl;
    std::cout << "╚════════════════════════════════════════╝" << std::endl;
    std::cout << "\nOpción: ";
}

void ejecutar_simulador_interactivo() {
    SimuladorInteractivo simulador;
    
    simulador.mostrar_estado_completo();
    
    bool continuar = true;
    while (continuar) {
        mostrar_menu_principal();
        
        int opcion;
        std::cin >> opcion;
        
        // Limpiar el buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        switch (opcion) {
            case 1: { // Enviar transacción
                std::cout << "\n┌─────────────────────────────────────┐" << std::endl;
                std::cout << "│    💸 ENVIAR TRANSACCIÓN            │" << std::endl;
                std::cout << "└─────────────────────────────────────┘" << std::endl;
                
                // Mostrar usuarios disponibles
                std::cout << "\nUsuarios disponibles:" << std::endl;
                auto usuarios = simulador.listar_usuarios();
                for (size_t i = 0; i < usuarios.size(); ++i) {
                    double saldo = simulador.consultar_saldo(usuarios[i]);
                    std::cout << "  " << (i+1) << ". " << usuarios[i] 
                              << " (Saldo: $" << std::fixed << std::setprecision(2) 
                              << saldo << ")" << std::endl;
                }
                
                std::cout << "\nUsuario origen (nombre): ";
                std::string origen;
                std::getline(std::cin, origen);
                
                std::cout << "Usuario destino (nombre): ";
                std::string destino;
                std::getline(std::cin, destino);
                
                std::cout << "Monto: $";
                double monto;
                std::cin >> monto;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                simulador.enviar_transaccion_manual(origen, destino, monto, "TRANSFERENCIA");
                
                std::cout << "\nPresiona Enter para continuar...";
                std::cin.get();
                simulador.mostrar_estado_completo();
                break;
            }
            
            case 2: { // Ver cuentas
                simulador.mostrar_estado_completo();
                std::cout << "\nPresiona Enter para continuar...";
                std::cin.get();
                break;
            }
            
            case 3: { // Ver estadísticas
                simulador.mostrar_estadisticas();
                std::cout << "\nPresiona Enter para continuar...";
                std::cin.get();
                simulador.mostrar_estado_completo();
                break;
            }
            
            case 4: { // Crear usuario
                std::cout << "\n┌─────────────────────────────────────┐" << std::endl;
                std::cout << "│    👤 CREAR NUEVO USUARIO           │" << std::endl;
                std::cout << "└─────────────────────────────────────┘" << std::endl;
                
                std::cout << "\nNombre del usuario: ";
                std::string nombre;
                std::getline(std::cin, nombre);
                
                std::cout << "Saldo inicial: $";
                double saldo;
                std::cin >> saldo;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                simulador.crear_usuario(nombre, saldo);
                std::cout << "\n✅ Usuario creado exitosamente" << std::endl;
                
                std::cout << "\nPresiona Enter para continuar...";
                std::cin.get();
                simulador.mostrar_estado_completo();
                break;
            }
            
            case 5: { // Actualizar pantalla
                simulador.mostrar_estado_completo();
                break;
            }
            
            case 6: { // Salir
                std::cout << "\n👋 ¡Hasta luego!" << std::endl;
                continuar = false;
                break;
            }
            
            default:
                std::cout << "\n❌ Opción inválida" << std::endl;
                std::cout << "\nPresiona Enter para continuar...";
                std::cin.get();
                simulador.mostrar_estado_completo();
        }
    }
}
