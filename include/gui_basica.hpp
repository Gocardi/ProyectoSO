#ifndef GUI_BASICA_HPP
#define GUI_BASICA_HPP

#include "simulador_interactivo.hpp"
#include <string>
#include <vector>
#include <memory>

// Clase para manejar la GUI básica con ASCII art
class GUIBasica {
private:
    std::shared_ptr<SimuladorInteractivo> simulador;
    int pantalla_actual;
    int usuario_seleccionado;
    int menu_seleccion;
    
    // Estado de formularios
    std::string form_usuario_origen;
    std::string form_usuario_destino;
    std::string form_monto;
    std::string form_nuevo_usuario;
    std::string form_saldo_inicial;
    
    // Colores ANSI
    const std::string COLOR_RESET = "\033[0m";
    const std::string COLOR_CYAN = "\033[36m";
    const std::string COLOR_GREEN = "\033[32m";
    const std::string COLOR_YELLOW = "\033[33m";
    const std::string COLOR_RED = "\033[31m";
    const std::string COLOR_MAGENTA = "\033[35m";
    const std::string COLOR_BLUE = "\033[34m";
    const std::string COLOR_BOLD = "\033[1m";
    
public:
    GUIBasica();
    
    // Pantallas principales
    void ejecutar();
    void mostrar_dashboard();
    void mostrar_panel_usuarios();
    void mostrar_panel_transacciones();
    void mostrar_formulario_transferencia();
    void mostrar_formulario_nuevo_usuario();
    
    // Utilidades de dibujo
    void limpiar_pantalla();
    void dibujar_encabezado();
    void dibujar_barra_navegacion();
    void dibujar_caja(int ancho, const std::string& titulo);
    void esperar_enter();
    
    // Utilidades de entrada
    std::string leer_input(const std::string& prompt);
    int leer_opcion();
};

#endif // GUI_BASICA_HPP
