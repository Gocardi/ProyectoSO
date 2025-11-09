#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <memory>
#include "database_json.hpp"
#include "monitor.hpp"
#include "productor_consumidor.hpp"
#include "lectores_escritores.hpp"
#include "deadlock.hpp"
#include "semaforo.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots para usuarios
    void crear_usuario();
    void actualizar_tabla_usuarios();
    void seleccionar_usuario(int row, int column);
    
    // Slots para transacciones
    void enviar_transaccion();
    void actualizar_tabla_transacciones();
    
    // Slots para estadísticas
    void actualizar_estadisticas();
    
    // Slots de procesamiento
    void iniciar_procesamiento();
    void detener_procesamiento();
    void procesar_transacciones_background();
    
    // Slots para demostraciones
    void demostrar_deadlock();
    void resolver_deadlock_demo();
    void demostrar_semaforo();
    void demostrar_lectores_escritores();

private:
    // Componentes de UI
    QTabWidget *tabs;
    
    // Tab Usuarios
    QTableWidget *tabla_usuarios;
    QLineEdit *input_nombre_usuario;
    QDoubleSpinBox *input_saldo_inicial;
    QPushButton *btn_crear_usuario;
    QPushButton *btn_actualizar_usuarios;
    
    // Tab Transacciones
    QTableWidget *tabla_transacciones;
    QLineEdit *input_usuario_origen;
    QLineEdit *input_usuario_destino;
    QDoubleSpinBox *input_monto;
    QPushButton *btn_enviar_transaccion;
    QPushButton *btn_actualizar_transacciones;
    
    // Tab Estadísticas
    QLabel *lbl_total_transacciones;
    QLabel *lbl_transacciones_aprobadas;
    QLabel *lbl_transacciones_sospechosas;
    QLabel *lbl_monto_total;
    QLabel *lbl_cola_size;
    QTextEdit *log_actividad;
    QPushButton *btn_iniciar_procesamiento;
    QPushButton *btn_detener_procesamiento;
    
    // Tab Demostraciones
    QTextEdit *log_demostraciones;
    QPushButton *btn_demo_deadlock;
    QPushButton *btn_resolver_deadlock;
    QPushButton *btn_demo_semaforo;
    QPushButton *btn_demo_lectores_escritores;
    
    // Timer para actualización automática
    QTimer *timer_actualizacion;
    
    // Base de datos y componentes del sistema
    std::shared_ptr<DatabaseJSON> db;
    std::shared_ptr<MonitorCuentas> monitor;
    std::shared_ptr<ColaTransacciones> cola;
    std::shared_ptr<ConfiguracionSistema> config;
    
    // Control de estado
    bool procesamiento_activo;
    int contador_transacciones;
    int transacciones_aprobadas;
    int transacciones_sospechosas;
    double monto_total_procesado;
    
    // Métodos auxiliares
    void setup_ui();
    void setup_tab_usuarios();
    void setup_tab_transacciones();
    void setup_tab_estadisticas();
    void setup_tab_demostraciones();
    void cargar_datos_iniciales();
    void log_mensaje(const QString& mensaje, const QString& tipo = "info");
    void log_demo(const QString& mensaje, const QString& tipo = "info");
    void sincronizar_db_con_monitor();
};

#endif // MAINWINDOW_HPP
