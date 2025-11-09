#ifndef DATABASE_JSON_HPP
#define DATABASE_JSON_HPP

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>

struct UsuarioDB {
    std::string nombre;
    std::string cuenta_id;
    double saldo;
    std::string fecha_creacion;
};

struct TransaccionDB {
    int id;
    std::string usuario_origen;
    std::string usuario_destino;
    double monto;
    std::string tipo;
    bool es_sospechosa;
    std::string fecha;
};

class DatabaseJSON {
private:
    std::string archivo_usuarios;
    std::string archivo_transacciones;
    mutable std::mutex mtx;
    
    std::string obtener_fecha_actual();
    std::string escapar_json(const std::string& str);
    std::string trim(const std::string& str);
    
public:
    DatabaseJSON(const std::string& archivo_usuarios = "usuarios.json", 
                 const std::string& archivo_transacciones = "transacciones.json");
    
    // Operaciones de usuarios
    bool guardar_usuario(const UsuarioDB& usuario);
    bool actualizar_saldo(const std::string& nombre, double nuevo_saldo);
    std::vector<UsuarioDB> cargar_usuarios();
    UsuarioDB obtener_usuario(const std::string& nombre);
    bool usuario_existe(const std::string& nombre);
    
    // Operaciones de transacciones
    bool guardar_transaccion(const TransaccionDB& transaccion);
    std::vector<TransaccionDB> cargar_transacciones(int limite = 100);
    std::vector<TransaccionDB> cargar_transacciones_usuario(const std::string& nombre, int limite = 50);
    int obtener_siguiente_id_transaccion();
    
    // Utilidades
    void inicializar_archivos();
    bool exportar_backup(const std::string& directorio);
};

#endif // DATABASE_JSON_HPP
