#include "database_json.hpp"
#include <iostream>
#include <algorithm>

DatabaseJSON::DatabaseJSON(const std::string& archivo_usuarios, 
                           const std::string& archivo_transacciones)
    : archivo_usuarios(archivo_usuarios), 
      archivo_transacciones(archivo_transacciones) {
    inicializar_archivos();
}

std::string DatabaseJSON::obtener_fecha_actual() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string DatabaseJSON::escapar_json(const std::string& str) {
    std::string resultado;
    for (char c : str) {
        if (c == '"' || c == '\\') {
            resultado += '\\';
        }
        resultado += c;
    }
    return resultado;
}

std::string DatabaseJSON::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

void DatabaseJSON::inicializar_archivos() {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Inicializar archivo de usuarios si no existe
    std::ifstream test_usuarios(archivo_usuarios);
    if (!test_usuarios.good()) {
        std::ofstream usuarios(archivo_usuarios);
        usuarios << "[\n]\n";
        usuarios.close();
    }
    test_usuarios.close();
    
    // Inicializar archivo de transacciones si no existe
    std::ifstream test_transacciones(archivo_transacciones);
    if (!test_transacciones.good()) {
        std::ofstream transacciones(archivo_transacciones);
        transacciones << "[\n]\n";
        transacciones.close();
    }
    test_transacciones.close();
}

bool DatabaseJSON::guardar_usuario(const UsuarioDB& usuario) {
    std::lock_guard<std::mutex> lock(mtx);
    
    // Cargar usuarios existentes
    auto usuarios = cargar_usuarios();
    
    // Verificar si ya existe
    for (const auto& u : usuarios) {
        if (u.nombre == usuario.nombre) {
            return false; // Usuario ya existe
        }
    }
    
    // Agregar nuevo usuario
    usuarios.push_back(usuario);
    
    // Guardar archivo
    std::ofstream archivo(archivo_usuarios);
    if (!archivo.is_open()) return false;
    
    archivo << "[\n";
    for (size_t i = 0; i < usuarios.size(); ++i) {
        archivo << "  {\n";
        archivo << "    \"nombre\": \"" << escapar_json(usuarios[i].nombre) << "\",\n";
        archivo << "    \"cuenta_id\": \"" << escapar_json(usuarios[i].cuenta_id) << "\",\n";
        archivo << "    \"saldo\": " << std::fixed << std::setprecision(2) << usuarios[i].saldo << ",\n";
        archivo << "    \"fecha_creacion\": \"" << escapar_json(usuarios[i].fecha_creacion) << "\"\n";
        archivo << "  }";
        if (i < usuarios.size() - 1) archivo << ",";
        archivo << "\n";
    }
    archivo << "]\n";
    archivo.close();
    
    return true;
}

bool DatabaseJSON::actualizar_saldo(const std::string& nombre, double nuevo_saldo) {
    std::lock_guard<std::mutex> lock(mtx);
    
    auto usuarios = cargar_usuarios();
    bool encontrado = false;
    
    for (auto& u : usuarios) {
        if (u.nombre == nombre) {
            u.saldo = nuevo_saldo;
            encontrado = true;
            break;
        }
    }
    
    if (!encontrado) return false;
    
    // Guardar archivo actualizado
    std::ofstream archivo(archivo_usuarios);
    if (!archivo.is_open()) return false;
    
    archivo << "[\n";
    for (size_t i = 0; i < usuarios.size(); ++i) {
        archivo << "  {\n";
        archivo << "    \"nombre\": \"" << escapar_json(usuarios[i].nombre) << "\",\n";
        archivo << "    \"cuenta_id\": \"" << escapar_json(usuarios[i].cuenta_id) << "\",\n";
        archivo << "    \"saldo\": " << std::fixed << std::setprecision(2) << usuarios[i].saldo << ",\n";
        archivo << "    \"fecha_creacion\": \"" << escapar_json(usuarios[i].fecha_creacion) << "\"\n";
        archivo << "  }";
        if (i < usuarios.size() - 1) archivo << ",";
        archivo << "\n";
    }
    archivo << "]\n";
    archivo.close();
    
    return true;
}

std::vector<UsuarioDB> DatabaseJSON::cargar_usuarios() {
    std::vector<UsuarioDB> usuarios;
    
    std::ifstream archivo(archivo_usuarios);
    if (!archivo.is_open()) return usuarios;
    
    std::string linea;
    UsuarioDB usuario_actual;
    bool leyendo_usuario = false;
    
    while (std::getline(archivo, linea)) {
        linea = trim(linea);
        
        if (linea == "{") {
            leyendo_usuario = true;
            usuario_actual = UsuarioDB();
        } else if (linea == "}" || linea == "},") {
            if (leyendo_usuario) {
                usuarios.push_back(usuario_actual);
                leyendo_usuario = false;
            }
        } else if (leyendo_usuario) {
            size_t pos_colon = linea.find(':');
            if (pos_colon != std::string::npos) {
                std::string clave = trim(linea.substr(0, pos_colon));
                std::string valor = trim(linea.substr(pos_colon + 1));
                
                // Remover comillas y comas finales
                clave.erase(std::remove(clave.begin(), clave.end(), '"'), clave.end());
                if (!valor.empty() && valor.back() == ',') {
                    valor = valor.substr(0, valor.length() - 1);
                }
                valor = trim(valor);
                
                if (clave == "nombre") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    usuario_actual.nombre = valor;
                } else if (clave == "cuenta_id") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    usuario_actual.cuenta_id = valor;
                } else if (clave == "saldo") {
                    usuario_actual.saldo = std::stod(valor);
                } else if (clave == "fecha_creacion") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    usuario_actual.fecha_creacion = valor;
                }
            }
        }
    }
    
    archivo.close();
    return usuarios;
}

UsuarioDB DatabaseJSON::obtener_usuario(const std::string& nombre) {
    auto usuarios = cargar_usuarios();
    for (const auto& u : usuarios) {
        if (u.nombre == nombre) {
            return u;
        }
    }
    return UsuarioDB{"", "", -1.0, ""};
}

bool DatabaseJSON::usuario_existe(const std::string& nombre) {
    auto usuarios = cargar_usuarios();
    for (const auto& u : usuarios) {
        if (u.nombre == nombre) {
            return true;
        }
    }
    return false;
}

bool DatabaseJSON::guardar_transaccion(const TransaccionDB& transaccion) {
    std::lock_guard<std::mutex> lock(mtx);
    
    auto transacciones = cargar_transacciones();
    transacciones.push_back(transaccion);
    
    std::ofstream archivo(archivo_transacciones);
    if (!archivo.is_open()) return false;
    
    archivo << "[\n";
    for (size_t i = 0; i < transacciones.size(); ++i) {
        archivo << "  {\n";
        archivo << "    \"id\": " << transacciones[i].id << ",\n";
        archivo << "    \"usuario_origen\": \"" << escapar_json(transacciones[i].usuario_origen) << "\",\n";
        archivo << "    \"usuario_destino\": \"" << escapar_json(transacciones[i].usuario_destino) << "\",\n";
        archivo << "    \"monto\": " << std::fixed << std::setprecision(2) << transacciones[i].monto << ",\n";
        archivo << "    \"tipo\": \"" << escapar_json(transacciones[i].tipo) << "\",\n";
        archivo << "    \"es_sospechosa\": " << (transacciones[i].es_sospechosa ? "true" : "false") << ",\n";
        archivo << "    \"fecha\": \"" << escapar_json(transacciones[i].fecha) << "\"\n";
        archivo << "  }";
        if (i < transacciones.size() - 1) archivo << ",";
        archivo << "\n";
    }
    archivo << "]\n";
    archivo.close();
    
    return true;
}

std::vector<TransaccionDB> DatabaseJSON::cargar_transacciones(int limite) {
    std::vector<TransaccionDB> transacciones;
    
    std::ifstream archivo(archivo_transacciones);
    if (!archivo.is_open()) return transacciones;
    
    std::string linea;
    TransaccionDB transaccion_actual;
    bool leyendo_transaccion = false;
    
    while (std::getline(archivo, linea)) {
        linea = trim(linea);
        
        if (linea == "{") {
            leyendo_transaccion = true;
            transaccion_actual = TransaccionDB();
        } else if (linea == "}" || linea == "},") {
            if (leyendo_transaccion) {
                transacciones.push_back(transaccion_actual);
                leyendo_transaccion = false;
            }
        } else if (leyendo_transaccion) {
            size_t pos_colon = linea.find(':');
            if (pos_colon != std::string::npos) {
                std::string clave = trim(linea.substr(0, pos_colon));
                std::string valor = trim(linea.substr(pos_colon + 1));
                
                clave.erase(std::remove(clave.begin(), clave.end(), '"'), clave.end());
                if (!valor.empty() && valor.back() == ',') {
                    valor = valor.substr(0, valor.length() - 1);
                }
                valor = trim(valor);
                
                if (clave == "id") {
                    transaccion_actual.id = std::stoi(valor);
                } else if (clave == "usuario_origen") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    transaccion_actual.usuario_origen = valor;
                } else if (clave == "usuario_destino") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    transaccion_actual.usuario_destino = valor;
                } else if (clave == "monto") {
                    transaccion_actual.monto = std::stod(valor);
                } else if (clave == "tipo") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    transaccion_actual.tipo = valor;
                } else if (clave == "es_sospechosa") {
                    transaccion_actual.es_sospechosa = (valor == "true");
                } else if (clave == "fecha") {
                    valor.erase(std::remove(valor.begin(), valor.end(), '"'), valor.end());
                    transaccion_actual.fecha = valor;
                }
            }
        }
    }
    
    archivo.close();
    
    // Limitar resultados
    if (limite > 0 && transacciones.size() > static_cast<size_t>(limite)) {
        transacciones.erase(transacciones.begin(), transacciones.end() - limite);
    }
    
    return transacciones;
}

std::vector<TransaccionDB> DatabaseJSON::cargar_transacciones_usuario(const std::string& nombre, int limite) {
    auto todas = cargar_transacciones(0);
    std::vector<TransaccionDB> filtradas;
    
    for (const auto& t : todas) {
        if (t.usuario_origen == nombre || t.usuario_destino == nombre) {
            filtradas.push_back(t);
        }
    }
    
    if (limite > 0 && filtradas.size() > static_cast<size_t>(limite)) {
        filtradas.erase(filtradas.begin(), filtradas.end() - limite);
    }
    
    return filtradas;
}

int DatabaseJSON::obtener_siguiente_id_transaccion() {
    auto transacciones = cargar_transacciones(0);
    if (transacciones.empty()) return 1;
    
    int max_id = 0;
    for (const auto& t : transacciones) {
        if (t.id > max_id) max_id = t.id;
    }
    
    return max_id + 1;
}

bool DatabaseJSON::exportar_backup(const std::string& directorio) {
    // Implementación simple de backup
    std::string fecha = obtener_fecha_actual();
    std::replace(fecha.begin(), fecha.end(), ' ', '_');
    std::replace(fecha.begin(), fecha.end(), ':', '-');
    
    // Copiar archivos con timestamp
    std::string backup_usuarios = directorio + "/usuarios_backup_" + fecha + ".json";
    std::string backup_transacciones = directorio + "/transacciones_backup_" + fecha + ".json";
    
    std::ifstream src1(archivo_usuarios, std::ios::binary);
    std::ofstream dst1(backup_usuarios, std::ios::binary);
    dst1 << src1.rdbuf();
    
    std::ifstream src2(archivo_transacciones, std::ios::binary);
    std::ofstream dst2(backup_transacciones, std::ios::binary);
    dst2 << src2.rdbuf();
    
    return true;
}
