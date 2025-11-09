#include "gui_basica.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

GUIBasica::GUIBasica() 
    : pantalla_actual(0), usuario_seleccionado(0), menu_seleccion(0) {
    simulador = std::make_shared<SimuladorInteractivo>();
}

void GUIBasica::limpiar_pantalla() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void GUIBasica::dibujar_encabezado() {
    std::cout << COLOR_CYAN << COLOR_BOLD;
    std::cout << "\n╔═══════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                       ║\n";
    std::cout << "║            🏦  SISTEMA BANCARIO INTERACTIVO - GUI                    ║\n";
    std::cout << "║                     Control Total de Usuarios                         ║\n";
    std::cout << "║                                                                       ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════════════════╝\n";
    std::cout << COLOR_RESET << "\n";
}

void GUIBasica::dibujar_barra_navegacion() {
    std::cout << COLOR_BLUE;
    std::cout << "\n┌───────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│  [1] Dashboard  │  [2] Usuarios  │  [3] Transacciones  │  [0] Salir  │\n";
    std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
    std::cout << COLOR_RESET;
}

void GUIBasica::dibujar_caja(int ancho, const std::string& titulo) {
    std::cout << COLOR_YELLOW;
    std::cout << "┌";
    for (int i = 0; i < ancho - 2; ++i) std::cout << "─";
    std::cout << "┐\n";
    
    std::cout << "│ " << COLOR_BOLD << titulo << COLOR_RESET << COLOR_YELLOW;
    int espacios = ancho - titulo.length() - 4;
    for (int i = 0; i < espacios; ++i) std::cout << " ";
    std::cout << " │\n";
    
    std::cout << "└";
    for (int i = 0; i < ancho - 2; ++i) std::cout << "─";
    std::cout << "┘\n";
    std::cout << COLOR_RESET;
}

void GUIBasica::mostrar_dashboard() {
    limpiar_pantalla();
    dibujar_encabezado();
    
    // Estadísticas
    std::cout << COLOR_GREEN << COLOR_BOLD;
    std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃                      📊  ESTADÍSTICAS GENERALES                  ┃\n";
    std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    std::cout << COLOR_RESET;
    
    std::cout << COLOR_CYAN;
    std::cout << "\n  📈 Transacciones Procesadas:  " << COLOR_BOLD << COLOR_GREEN 
              << simulador->get_transacciones_procesadas() << COLOR_RESET;
    std::cout << COLOR_CYAN << "\n  ✅ Aprobadas:                 " << COLOR_BOLD << COLOR_GREEN 
              << simulador->get_transacciones_aprobadas() << COLOR_RESET;
    std::cout << COLOR_CYAN << "\n  ⚠️  Sospechosas:              " << COLOR_BOLD << COLOR_YELLOW 
              << simulador->get_transacciones_sospechosas() << COLOR_RESET;
    std::cout << COLOR_CYAN << "\n  📦 En Cola:                   " << COLOR_BOLD 
              << simulador->get_tamanio_cola() << "/10" << COLOR_RESET;
    std::cout << "\n";
    
    // Lista de usuarios y saldos
    std::cout << "\n" << COLOR_MAGENTA << COLOR_BOLD;
    std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃                        💰  USUARIOS Y SALDOS                     ┃\n";
    std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    std::cout << COLOR_RESET;
    
    auto usuarios = simulador->listar_usuarios();
    std::cout << "\n";
    
    int contador = 1;
    for (const auto& usuario : usuarios) {
        double saldo = simulador->consultar_saldo(usuario);
        
        std::cout << "  " << COLOR_BOLD << COLOR_CYAN << contador << ". " << COLOR_RESET;
        std::cout << "👤 " << COLOR_BOLD << std::left << std::setw(15) << usuario << COLOR_RESET;
        std::cout << " │ ";
        
        // Color del saldo según el monto
        if (saldo > 10000) {
            std::cout << COLOR_GREEN << COLOR_BOLD;
        } else if (saldo > 5000) {
            std::cout << COLOR_YELLOW;
        } else {
            std::cout << COLOR_RED;
        }
        
        std::cout << "$" << std::fixed << std::setprecision(2) << std::right 
                  << std::setw(12) << saldo << COLOR_RESET << "\n";
        contador++;
    }
    
    dibujar_barra_navegacion();
}

void GUIBasica::mostrar_panel_usuarios() {
    limpiar_pantalla();
    dibujar_encabezado();
    
    std::cout << COLOR_MAGENTA << COLOR_BOLD;
    std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃                    👥  GESTIÓN DE USUARIOS                       ┃\n";
    std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    std::cout << COLOR_RESET;
    
    auto usuarios = simulador->listar_usuarios();
    
    std::cout << "\n" << COLOR_CYAN << "Usuarios actuales:\n" << COLOR_RESET;
    std::cout << "\n";
    
    int contador = 1;
    for (const auto& usuario : usuarios) {
        double saldo = simulador->consultar_saldo(usuario);
        
        std::cout << "  " << COLOR_BOLD << "[" << contador << "]" << COLOR_RESET;
        std::cout << "  👤 " << COLOR_CYAN << std::setw(15) << std::left << usuario << COLOR_RESET;
        std::cout << "  │  Cuenta: CTA-" << usuario;
        std::cout << "  │  Saldo: " << COLOR_GREEN << "$" << std::fixed 
                  << std::setprecision(2) << saldo << COLOR_RESET << "\n";
        contador++;
    }
    
    std::cout << "\n" << COLOR_YELLOW;
    std::cout << "┌───────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│  Opciones:                                                            │\n";
    std::cout << "│    [A] Agregar nuevo usuario                                          │\n";
    std::cout << "│    [V] Ver detalles de usuario                                        │\n";
    std::cout << "│    [R] Regresar al menú principal                                     │\n";
    std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
    std::cout << COLOR_RESET;
}

void GUIBasica::mostrar_panel_transacciones() {
    limpiar_pantalla();
    dibujar_encabezado();
    
    std::cout << COLOR_GREEN << COLOR_BOLD;
    std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃                    💸  PANEL DE TRANSACCIONES                    ┃\n";
    std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    std::cout << COLOR_RESET;
    
    std::cout << "\n" << COLOR_CYAN << "Opciones disponibles:\n" << COLOR_RESET;
    std::cout << "\n";
    std::cout << "  " << COLOR_BOLD << "[1]" << COLOR_RESET << "  💰 Nueva Transferencia\n";
    std::cout << "  " << COLOR_BOLD << "[2]" << COLOR_RESET << "  📊 Ver estadísticas de transacciones\n";
    std::cout << "  " << COLOR_BOLD << "[3]" << COLOR_RESET << "  🔍 Consultar saldo de usuario\n";
    std::cout << "  " << COLOR_BOLD << "[0]" << COLOR_RESET << "  ← Regresar\n";
    
    std::cout << "\n" << COLOR_YELLOW;
    std::cout << "┌───────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│  Selecciona una opción:                                               │\n";
    std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
    std::cout << COLOR_RESET;
}

void GUIBasica::mostrar_formulario_transferencia() {
    limpiar_pantalla();
    dibujar_encabezado();
    
    std::cout << COLOR_GREEN << COLOR_BOLD;
    std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃                   💸  NUEVA TRANSFERENCIA                        ┃\n";
    std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    std::cout << COLOR_RESET;
    
    // Mostrar usuarios disponibles
    std::cout << "\n" << COLOR_CYAN << "Usuarios disponibles:\n" << COLOR_RESET;
    auto usuarios = simulador->listar_usuarios();
    
    int contador = 1;
    for (const auto& usuario : usuarios) {
        double saldo = simulador->consultar_saldo(usuario);
        std::cout << "  " << COLOR_BOLD << contador << "." << COLOR_RESET;
        std::cout << " " << usuario << " - Saldo: " << COLOR_GREEN << "$" 
                  << std::fixed << std::setprecision(2) << saldo << COLOR_RESET << "\n";
        contador++;
    }
    
    std::cout << "\n" << COLOR_YELLOW << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << COLOR_RESET;
    
    // Formulario
    std::cout << "\n" << COLOR_CYAN << "Usuario origen: " << COLOR_RESET;
    std::string origen;
    std::getline(std::cin, origen);
    
    std::cout << COLOR_CYAN << "Usuario destino: " << COLOR_RESET;
    std::string destino;
    std::getline(std::cin, destino);
    
    std::cout << COLOR_CYAN << "Monto: $" << COLOR_RESET;
    double monto;
    std::cin >> monto;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    std::cout << "\n" << COLOR_YELLOW << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n" << COLOR_RESET;
    
    // Confirmación
    std::cout << "\n" << COLOR_BOLD << "Resumen de la transacción:\n" << COLOR_RESET;
    std::cout << "  Origen:  " << COLOR_CYAN << origen << COLOR_RESET << "\n";
    std::cout << "  Destino: " << COLOR_CYAN << destino << COLOR_RESET << "\n";
    std::cout << "  Monto:   " << COLOR_GREEN << "$" << std::fixed << std::setprecision(2) << monto << COLOR_RESET << "\n";
    
    std::cout << "\n" << COLOR_YELLOW << "¿Confirmar transacción? (s/n): " << COLOR_RESET;
    char confirmar;
    std::cin >> confirmar;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    if (confirmar == 's' || confirmar == 'S') {
        bool exito = simulador->enviar_transaccion_manual(origen, destino, monto, "TRANSFERENCIA");
        
        if (exito) {
            std::cout << "\n" << COLOR_GREEN << COLOR_BOLD;
            std::cout << "┌───────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                  ✅  TRANSACCIÓN COMPLETADA                           │\n";
            std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
            std::cout << COLOR_RESET;
        } else {
            std::cout << "\n" << COLOR_RED << COLOR_BOLD;
            std::cout << "┌───────────────────────────────────────────────────────────────────────┐\n";
            std::cout << "│                  ❌  ERROR EN LA TRANSACCIÓN                         │\n";
            std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
            std::cout << COLOR_RESET;
        }
    } else {
        std::cout << "\n" << COLOR_YELLOW << "Transacción cancelada.\n" << COLOR_RESET;
    }
    
    esperar_enter();
}

void GUIBasica::mostrar_formulario_nuevo_usuario() {
    limpiar_pantalla();
    dibujar_encabezado();
    
    std::cout << COLOR_MAGENTA << COLOR_BOLD;
    std::cout << "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\n";
    std::cout << "┃                    👤  CREAR NUEVO USUARIO                       ┃\n";
    std::cout << "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\n";
    std::cout << COLOR_RESET;
    
    std::cout << "\n" << COLOR_CYAN << "Nombre del usuario: " << COLOR_RESET;
    std::string nombre;
    std::getline(std::cin, nombre);
    
    std::cout << COLOR_CYAN << "Saldo inicial: $" << COLOR_RESET;
    double saldo;
    std::cin >> saldo;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    
    simulador->crear_usuario(nombre, saldo);
    
    std::cout << "\n" << COLOR_GREEN << COLOR_BOLD;
    std::cout << "┌───────────────────────────────────────────────────────────────────────┐\n";
    std::cout << "│               ✅  USUARIO CREADO EXITOSAMENTE                        │\n";
    std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
    std::cout << COLOR_RESET;
    
    std::cout << "\n  Usuario: " << COLOR_CYAN << nombre << COLOR_RESET;
    std::cout << "\n  Cuenta:  CTA-" << nombre;
    std::cout << "\n  Saldo:   " << COLOR_GREEN << "$" << std::fixed << std::setprecision(2) << saldo << COLOR_RESET << "\n";
    
    esperar_enter();
}

void GUIBasica::esperar_enter() {
    std::cout << "\n" << COLOR_YELLOW << "Presiona Enter para continuar..." << COLOR_RESET;
    std::cin.get();
}

std::string GUIBasica::leer_input(const std::string& prompt) {
    std::cout << COLOR_CYAN << prompt << COLOR_RESET;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int GUIBasica::leer_opcion() {
    std::cout << "\n" << COLOR_CYAN << "Selecciona una opción: " << COLOR_RESET;
    int opcion;
    std::cin >> opcion;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return opcion;
}

void GUIBasica::ejecutar() {
    bool continuar = true;
    
    while (continuar) {
        mostrar_dashboard();
        int opcion = leer_opcion();
        
        switch (opcion) {
            case 1: // Dashboard (ya mostrado)
                break;
                
            case 2: { // Panel de Usuarios
                mostrar_panel_usuarios();
                std::cout << "\n" << COLOR_CYAN << "Opción: " << COLOR_RESET;
                char op_usuario;
                std::cin >> op_usuario;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                
                if (op_usuario == 'A' || op_usuario == 'a') {
                    mostrar_formulario_nuevo_usuario();
                } else if (op_usuario == 'V' || op_usuario == 'v') {
                    std::cout << "\n" << COLOR_CYAN << "Nombre del usuario: " << COLOR_RESET;
                    std::string nombre;
                    std::getline(std::cin, nombre);
                    double saldo = simulador->consultar_saldo(nombre);
                    
                    if (saldo >= 0) {
                        std::cout << "\n" << COLOR_GREEN;
                        std::cout << "Usuario: " << nombre << "\n";
                        std::cout << "Cuenta: CTA-" << nombre << "\n";
                        std::cout << "Saldo: $" << std::fixed << std::setprecision(2) << saldo << "\n";
                        std::cout << COLOR_RESET;
                    } else {
                        std::cout << "\n" << COLOR_RED << "Usuario no encontrado.\n" << COLOR_RESET;
                    }
                    esperar_enter();
                }
                break;
            }
            
            case 3: { // Panel de Transacciones
                mostrar_panel_transacciones();
                int op_trans = leer_opcion();
                
                if (op_trans == 1) {
                    mostrar_formulario_transferencia();
                } else if (op_trans == 2) {
                    limpiar_pantalla();
                    simulador->mostrar_estadisticas();
                    esperar_enter();
                } else if (op_trans == 3) {
                    std::cout << "\n" << COLOR_CYAN << "Nombre del usuario: " << COLOR_RESET;
                    std::string nombre;
                    std::getline(std::cin, nombre);
                    double saldo = simulador->consultar_saldo(nombre);
                    
                    if (saldo >= 0) {
                        std::cout << COLOR_GREEN << "Saldo de " << nombre << ": $" 
                                  << std::fixed << std::setprecision(2) << saldo << COLOR_RESET << "\n";
                    } else {
                        std::cout << COLOR_RED << "Usuario no encontrado.\n" << COLOR_RESET;
                    }
                    esperar_enter();
                }
                break;
            }
            
            case 0: // Salir
                limpiar_pantalla();
                std::cout << "\n" << COLOR_CYAN << COLOR_BOLD;
                std::cout << "┌───────────────────────────────────────────────────────────────────────┐\n";
                std::cout << "│                                                                       │\n";
                std::cout << "│                    👋  ¡HASTA LUEGO!                                 │\n";
                std::cout << "│              Gracias por usar el Sistema Bancario                     │\n";
                std::cout << "│                                                                       │\n";
                std::cout << "└───────────────────────────────────────────────────────────────────────┘\n";
                std::cout << COLOR_RESET << "\n";
                continuar = false;
                break;
                
            default:
                std::cout << COLOR_RED << "\n❌ Opción inválida\n" << COLOR_RESET;
                esperar_enter();
        }
    }
}
