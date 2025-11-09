#include "gui_basica.hpp"
#include <iostream>
#include <memory>

int main() {
    try {
        auto gui = std::make_shared<GUIBasica>();
        gui->ejecutar();
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
