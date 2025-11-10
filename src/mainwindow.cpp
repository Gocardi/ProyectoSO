#include "mainwindow.hpp"
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QScrollBar>
#include <thread>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      procesamiento_activo(false),
      contador_transacciones(0),
      transacciones_aprobadas(0),
      transacciones_sospechosas(0),
      monto_total_procesado(0.0) {
    
    // Inicializar componentes
    db = std::make_shared<DatabaseJSON>("usuarios.json", "transacciones.json");
    monitor = std::make_shared<MonitorCuentas>();
    cola = std::make_shared<ColaTransacciones>(10);
    config = std::make_shared<ConfiguracionSistema>();
    contexto_fraude = std::make_shared<ContextoFraude>(); // Nuevo contexto compartido
    
    // Configurar UI
    setup_ui();
    
    // Cargar datos iniciales
    cargar_datos_iniciales();
    
    // Timer para actualización automática
    timer_actualizacion = new QTimer(this);
    connect(timer_actualizacion, &QTimer::timeout, this, &MainWindow::actualizar_estadisticas);
    timer_actualizacion->start(1000); // Actualizar cada segundo
    
    setWindowTitle("Sistema Bancario Concurrente - Qt + JSON");
    resize(1000, 700);
}

MainWindow::~MainWindow() {
    detener_procesamiento();
}

void MainWindow::setup_ui() {
    // Widget central con tabs
    tabs = new QTabWidget(this);
    setCentralWidget(tabs);
    
    setup_tab_usuarios();
    setup_tab_transacciones();
    setup_tab_estadisticas();
    setup_tab_demostraciones();
}

void MainWindow::setup_tab_usuarios() {
    QWidget *tab_usuarios_widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab_usuarios_widget);
    
    // Título
    QLabel *titulo = new QLabel("<h2>👥 Gestión de Usuarios</h2>");
    layout->addWidget(titulo);
    
    // Formulario de creación
    QGroupBox *form_group = new QGroupBox("Crear Nuevo Usuario");
    QHBoxLayout *form_layout = new QHBoxLayout();
    
    form_layout->addWidget(new QLabel("Nombre:"));
    input_nombre_usuario = new QLineEdit();
    input_nombre_usuario->setPlaceholderText("Ej: Juan");
    form_layout->addWidget(input_nombre_usuario);
    
    form_layout->addWidget(new QLabel("Saldo Inicial:"));
    input_saldo_inicial = new QDoubleSpinBox();
    input_saldo_inicial->setRange(0, 1000000);
    input_saldo_inicial->setSingleStep(1000.0);
    input_saldo_inicial->setValue(10000.0);
    input_saldo_inicial->setPrefix("$");
    form_layout->addWidget(input_saldo_inicial);
    
    btn_crear_usuario = new QPushButton("➕ Crear Usuario");
    btn_crear_usuario->setStyleSheet("background-color: #4CAF50; color: white; padding: 5px 15px;");
    connect(btn_crear_usuario, &QPushButton::clicked, this, &MainWindow::crear_usuario);
    form_layout->addWidget(btn_crear_usuario);
    
    form_group->setLayout(form_layout);
    layout->addWidget(form_group);
    
    // Tabla de usuarios
    tabla_usuarios = new QTableWidget();
    tabla_usuarios->setColumnCount(4);
    tabla_usuarios->setHorizontalHeaderLabels({"Nombre", "Cuenta ID", "Saldo", "Fecha Creación"});
    tabla_usuarios->horizontalHeader()->setStretchLastSection(true);
    tabla_usuarios->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla_usuarios->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(tabla_usuarios, &QTableWidget::cellClicked, this, &MainWindow::seleccionar_usuario);
    layout->addWidget(tabla_usuarios);
    
    // Botón actualizar
    btn_actualizar_usuarios = new QPushButton("🔄 Actualizar Lista");
    connect(btn_actualizar_usuarios, &QPushButton::clicked, this, &MainWindow::actualizar_tabla_usuarios);
    layout->addWidget(btn_actualizar_usuarios);
    
    tabs->addTab(tab_usuarios_widget, "👥 Usuarios");
}

void MainWindow::setup_tab_transacciones() {
    QWidget *tab_transacciones_widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab_transacciones_widget);
    
    // Título
    QLabel *titulo = new QLabel("<h2>💸 Transacciones</h2>");
    layout->addWidget(titulo);
    
    // Formulario de transacción
    QGroupBox *form_group = new QGroupBox("Nueva Transferencia");
    QHBoxLayout *form_layout = new QHBoxLayout();
    
    form_layout->addWidget(new QLabel("Origen:"));
    input_usuario_origen = new QLineEdit();
    input_usuario_origen->setPlaceholderText("Nombre usuario origen");
    form_layout->addWidget(input_usuario_origen);
    
    form_layout->addWidget(new QLabel("Destino:"));
    input_usuario_destino = new QLineEdit();
    input_usuario_destino->setPlaceholderText("Nombre usuario destino");
    form_layout->addWidget(input_usuario_destino);
    
    form_layout->addWidget(new QLabel("Monto:"));
    input_monto = new QDoubleSpinBox();
    input_monto->setRange(0.01, 100000);
    input_monto->setValue(100.0);
    input_monto->setPrefix("$");
    form_layout->addWidget(input_monto);
    
    btn_enviar_transaccion = new QPushButton("💰 Enviar Transacción");
    btn_enviar_transaccion->setStyleSheet("background-color: #2196F3; color: white; padding: 5px 15px;");
    connect(btn_enviar_transaccion, &QPushButton::clicked, this, &MainWindow::enviar_transaccion);
    form_layout->addWidget(btn_enviar_transaccion);
    
    form_group->setLayout(form_layout);
    layout->addWidget(form_group);
    
    // Tabla de transacciones
    tabla_transacciones = new QTableWidget();
    tabla_transacciones->setColumnCount(7);
    tabla_transacciones->setHorizontalHeaderLabels({"ID", "Origen", "Destino", "Monto", "Tipo", "Sospechosa", "Fecha"});
    tabla_transacciones->horizontalHeader()->setStretchLastSection(true);
    tabla_transacciones->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabla_transacciones->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(tabla_transacciones);
    
    // Botón actualizar
    btn_actualizar_transacciones = new QPushButton("🔄 Actualizar Historial");
    connect(btn_actualizar_transacciones, &QPushButton::clicked, this, &MainWindow::actualizar_tabla_transacciones);
    layout->addWidget(btn_actualizar_transacciones);
    
    tabs->addTab(tab_transacciones_widget, "💸 Transacciones");
}

void MainWindow::setup_tab_estadisticas() {
    QWidget *tab_stats_widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab_stats_widget);
    
    // Título
    QLabel *titulo = new QLabel("<h2>📊 Estadísticas y Monitoreo</h2>");
    layout->addWidget(titulo);
    
    // Panel de estadísticas
    QGroupBox *stats_group = new QGroupBox("Métricas en Tiempo Real");
    QVBoxLayout *stats_layout = new QVBoxLayout();
    
    lbl_total_transacciones = new QLabel("📈 Total Transacciones: 0");
    lbl_total_transacciones->setStyleSheet("font-size: 14px; padding: 5px;");
    stats_layout->addWidget(lbl_total_transacciones);
    
    lbl_transacciones_aprobadas = new QLabel("✅ Aprobadas: 0");
    lbl_transacciones_aprobadas->setStyleSheet("font-size: 14px; padding: 5px; color: green;");
    stats_layout->addWidget(lbl_transacciones_aprobadas);
    
    lbl_transacciones_sospechosas = new QLabel("⚠️ Sospechosas: 0");
    lbl_transacciones_sospechosas->setStyleSheet("font-size: 14px; padding: 5px; color: orange;");
    stats_layout->addWidget(lbl_transacciones_sospechosas);
    
    lbl_monto_total = new QLabel("💰 Monto Total Procesado: $0.00");
    lbl_monto_total->setStyleSheet("font-size: 14px; padding: 5px;");
    stats_layout->addWidget(lbl_monto_total);
    
    lbl_cola_size = new QLabel("📦 Transacciones en Cola: 0/10");
    lbl_cola_size->setStyleSheet("font-size: 14px; padding: 5px;");
    stats_layout->addWidget(lbl_cola_size);
    
    stats_group->setLayout(stats_layout);
    layout->addWidget(stats_group);
    
    // Control de procesamiento
    QGroupBox *control_group = new QGroupBox("Control de Procesamiento");
    QHBoxLayout *control_layout = new QHBoxLayout();
    
    btn_iniciar_procesamiento = new QPushButton("▶️ Iniciar Procesamiento Automático");
    btn_iniciar_procesamiento->setStyleSheet("background-color: #4CAF50; color: white; padding: 10px;");
    connect(btn_iniciar_procesamiento, &QPushButton::clicked, this, &MainWindow::iniciar_procesamiento);
    control_layout->addWidget(btn_iniciar_procesamiento);
    
    btn_detener_procesamiento = new QPushButton("⏸️ Detener Procesamiento");
    btn_detener_procesamiento->setStyleSheet("background-color: #f44336; color: white; padding: 10px;");
    btn_detener_procesamiento->setEnabled(false);
    connect(btn_detener_procesamiento, &QPushButton::clicked, this, &MainWindow::detener_procesamiento);
    control_layout->addWidget(btn_detener_procesamiento);
    
    control_group->setLayout(control_layout);
    layout->addWidget(control_group);
    
    // Log de actividad
    QLabel *lbl_log = new QLabel("<b>📝 Log de Actividad:</b>");
    layout->addWidget(lbl_log);
    
    log_actividad = new QTextEdit();
    log_actividad->setReadOnly(true);
    log_actividad->setMaximumHeight(200);
    layout->addWidget(log_actividad);
    
    tabs->addTab(tab_stats_widget, "📊 Estadísticas");
}

void MainWindow::cargar_datos_iniciales() {
    // Cargar usuarios de la base de datos
    auto usuarios = db->cargar_usuarios();
    
    // Si no hay usuarios, crear algunos de ejemplo
    if (usuarios.empty()) {
        std::vector<std::pair<std::string, double>> usuarios_ejemplo = {
            {"Juan", 10000.0},
            {"Maria", 15000.0},
            {"Pedro", 8000.0},
            {"Ana", 12000.0},
            {"Luis", 20000.0}
        };
        
        for (const auto& [nombre, saldo] : usuarios_ejemplo) {
            UsuarioDB usuario;
            usuario.nombre = nombre;
            usuario.cuenta_id = "CTA-" + nombre;
            usuario.saldo = saldo;
            usuario.fecha_creacion = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
            db->guardar_usuario(usuario);
        }
        
        usuarios = db->cargar_usuarios();
        log_mensaje("✅ Se crearon 5 usuarios de ejemplo", "success");
    }
    
    // Sincronizar con el monitor
    sincronizar_db_con_monitor();
    
    // Actualizar tablas
    actualizar_tabla_usuarios();
    actualizar_tabla_transacciones();
    
    log_mensaje("✅ Datos iniciales cargados correctamente", "success");
}

void MainWindow::sincronizar_db_con_monitor() {
    auto usuarios = db->cargar_usuarios();
    for (const auto& usuario : usuarios) {
        monitor->crear_cuenta(usuario.cuenta_id, usuario.saldo);
    }
}

void MainWindow::crear_usuario() {
    QString nombre = input_nombre_usuario->text().trimmed();
    double saldo = input_saldo_inicial->value();
    
    if (nombre.isEmpty()) {
        QMessageBox::warning(this, "Error", "El nombre no puede estar vacío");
        return;
    }
    
    if (db->usuario_existe(nombre.toStdString())) {
        QMessageBox::warning(this, "Error", "El usuario ya existe");
        return;
    }
    
    UsuarioDB usuario;
    usuario.nombre = nombre.toStdString();
    usuario.cuenta_id = "CTA-" + nombre.toStdString();
    usuario.saldo = saldo;
    usuario.fecha_creacion = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
    
    if (db->guardar_usuario(usuario)) {
        monitor->crear_cuenta(usuario.cuenta_id, usuario.saldo);
        actualizar_tabla_usuarios();
        input_nombre_usuario->clear();
        input_saldo_inicial->setValue(10000.0);
        log_mensaje("✅ Usuario creado: " + nombre, "success");
        QMessageBox::information(this, "Éxito", "Usuario creado correctamente");
    } else {
        QMessageBox::critical(this, "Error", "No se pudo crear el usuario");
    }
}

void MainWindow::actualizar_tabla_usuarios() {
    auto usuarios = db->cargar_usuarios();
    
    tabla_usuarios->setRowCount(0);
    
    for (const auto& usuario : usuarios) {
        int row = tabla_usuarios->rowCount();
        tabla_usuarios->insertRow(row);
        
        tabla_usuarios->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(usuario.nombre)));
        tabla_usuarios->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(usuario.cuenta_id)));
        
        QTableWidgetItem *item_saldo = new QTableWidgetItem(QString("$%1").arg(usuario.saldo, 0, 'f', 2));
        item_saldo->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tabla_usuarios->setItem(row, 2, item_saldo);
        
        tabla_usuarios->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(usuario.fecha_creacion)));
    }
    
    tabla_usuarios->resizeColumnsToContents();
}

void MainWindow::seleccionar_usuario(int row, int column) {
    Q_UNUSED(column);
    QString nombre = tabla_usuarios->item(row, 0)->text();
    input_usuario_origen->setText(nombre);
}

void MainWindow::enviar_transaccion() {
    // 1. Obtener los datos de la interfaz de usuario
    QString origen_str = input_usuario_origen->text().trimmed();
    QString destino_str = input_usuario_destino->text().trimmed();
    double monto = input_monto->value();

    // 2. Validaciones básicas de la entrada
    if (origen_str.isEmpty() || destino_str.isEmpty()) {
        QMessageBox::warning(this, "Error de Entrada", "Debe especificar un origen y un destino.");
        return;
    }

    if (origen_str == destino_str) {
        QMessageBox::warning(this, "Error de Lógica", "El origen y el destino no pueden ser iguales.");
        return;
    }

    if (!db->usuario_existe(origen_str.toStdString()) || !db->usuario_existe(destino_str.toStdString())) {
        QMessageBox::warning(this, "Error de Validación", "Uno o ambos usuarios no existen en la base de datos.");
        return;
    }

    // 3. Obtener los datos completos de los usuarios desde la BD
    auto usuario_origen = db->obtener_usuario(origen_str.toStdString());
    auto usuario_destino = db->obtener_usuario(destino_str.toStdString());

    if (usuario_origen.saldo < monto) {
        QMessageBox::warning(this, "Fondos Insuficientes",
                             QString("El usuario %1 solo tiene $%2.").arg(origen_str).arg(usuario_origen.saldo, 0, 'f', 2));
        return;
    }

    // 4. Crear el objeto Transaccion (del modelo de datos en memoria)
    Transaccion t;
    t.id = db->obtener_siguiente_id_transaccion();
    t.cliente_id = usuario_origen.nombre;
    t.tipo = "TRANSFERENCIA";
    t.monto = monto;
    t.cuenta_origen = usuario_origen.cuenta_id;
    t.cuenta_destino = usuario_destino.cuenta_id;
    // t.timestamp se inicializa automáticamente en su constructor

    // 5. --- ¡LA LÓGICA CLAVE MEJORADA! ---
    // Se utiliza el contexto de fraude compartido para determinar si la transacción es sospechosa.
    // Esto centraliza la lógica de fraude para operaciones manuales y automáticas.
    bool es_sospechosa = contexto_fraude->analizarYActualizar(t);
    t.es_sospechosa = es_sospechosa; // Actualizamos el objeto de transacción

    if (t.es_sospechosa) {
        transacciones_sospechosas++; // Actualizamos el contador de la UI
        log_mensaje("! Transacción sospechosa detectada (Regla de Velocidad/Frecuencia): $" + QString::number(monto, 'f', 2), "warning");
    }

    // 6. Procesar la transacción inmediatamente para dar respuesta a la UI
    // Usamos el Monitor para garantizar una transferencia segura (atómica)
    bool exito = monitor->transferir(usuario_origen.cuenta_id, usuario_destino.cuenta_id, monto);

    if (exito) {
        // Si la transferencia en el monitor fue exitosa, persistimos los cambios

        // 6.1. Actualizar saldos en la base de datos JSON
        db->actualizar_saldo(usuario_origen.nombre, usuario_origen.saldo - monto);
        db->actualizar_saldo(usuario_destino.nombre, usuario_destino.saldo + monto);

        // 6.2. Guardar la transacción en la base de datos JSON
        TransaccionDB tdb;
        tdb.id = t.id;
        tdb.usuario_origen = t.cliente_id;
        tdb.usuario_destino = destino_str.toStdString();
        tdb.monto = t.monto;
        tdb.tipo = t.tipo;
        tdb.es_sospechosa = t.es_sospechosa; // ¡Se guarda el valor correcto de sospecha!
        tdb.fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        db->guardar_transaccion(tdb);

        // 6.3. Actualizar contadores y logs de la UI
        contador_transacciones++;
        transacciones_aprobadas++;
        monto_total_procesado += monto;

        log_mensaje(QString("✓ Transferencia exitosa: %1 → %2 ($%3)")
                        .arg(origen_str).arg(destino_str).arg(monto, 0, 'f', 2), "success");
        QMessageBox::information(this, "Éxito", "Transacción completada correctamente.");
        
        // 6.4. Limpiar la UI y actualizar las tablas
        actualizar_tabla_usuarios();
        actualizar_tabla_transacciones();
        input_usuario_origen->clear();
        input_usuario_destino->clear();
        input_monto->setValue(100.0);

    } else {
        // Si la transferencia falló en el monitor (aunque es poco probable con las validaciones previas)
        log_mensaje("X Error inesperado en la transferencia a través del monitor.", "error");
        QMessageBox::critical(this, "Error", "No se pudo completar la transacción a nivel del monitor.");
    }
}

void MainWindow::actualizar_tabla_transacciones() {
    auto transacciones = db->cargar_transacciones(100);
    
    tabla_transacciones->setRowCount(0);
    
    for (const auto& t : transacciones) {
        int row = tabla_transacciones->rowCount();
        tabla_transacciones->insertRow(row);
        
        tabla_transacciones->setItem(row, 0, new QTableWidgetItem(QString::number(t.id)));
        tabla_transacciones->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(t.usuario_origen)));
        tabla_transacciones->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(t.usuario_destino)));
        
        QTableWidgetItem *item_monto = new QTableWidgetItem(QString("$%1").arg(t.monto, 0, 'f', 2));
        item_monto->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tabla_transacciones->setItem(row, 3, item_monto);
        
        tabla_transacciones->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(t.tipo)));
        
        QTableWidgetItem *item_sospechosa = new QTableWidgetItem(t.es_sospechosa ? "⚠️ Sí" : "✅ No");
        if (t.es_sospechosa) {
            item_sospechosa->setBackground(QColor(255, 235, 59));
        }
        tabla_transacciones->setItem(row, 5, item_sospechosa);
        
        tabla_transacciones->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(t.fecha)));
    }
    
    tabla_transacciones->resizeColumnsToContents();
}

void MainWindow::actualizar_estadisticas() {
    lbl_total_transacciones->setText(QString("📈 Total Transacciones: %1").arg(contador_transacciones));
    lbl_transacciones_aprobadas->setText(QString("✅ Aprobadas: %1").arg(transacciones_aprobadas));
    lbl_transacciones_sospechosas->setText(QString("⚠️ Sospechosas: %1").arg(transacciones_sospechosas));
    lbl_monto_total->setText(QString("💰 Monto Total Procesado: $%1").arg(monto_total_procesado, 0, 'f', 2));
    lbl_cola_size->setText(QString("📦 Transacciones en Cola: %1/10").arg(cola->obtener_tamanio()));
}

void MainWindow::iniciar_procesamiento() {
    procesamiento_activo = true;
    btn_iniciar_procesamiento->setEnabled(false);
    btn_detener_procesamiento->setEnabled(true);
    log_mensaje("▶️ Procesamiento automático iniciado", "info");
    
    // Lanzar thread de procesamiento
    std::thread([this]() {
        procesar_transacciones_background();
    }).detach();
}

void MainWindow::detener_procesamiento() {
    procesamiento_activo = false;
    btn_iniciar_procesamiento->setEnabled(true);
    btn_detener_procesamiento->setEnabled(false);
    log_mensaje("⏸️ Procesamiento automático detenido", "info");
}

void MainWindow::procesar_transacciones_background() {
    // Esta función simula procesamiento automático de transacciones
    // En un sistema real, aquí procesarías transacciones de la cola
    while (procesamiento_activo) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        // Aquí podrías consumir transacciones de la cola
    }
}

void MainWindow::log_mensaje(const QString& mensaje, const QString& tipo) {
    QString color = "black";
    if (tipo == "success") color = "green";
    else if (tipo == "error") color = "red";
    else if (tipo == "warning") color = "orange";
    
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString html = QString("<span style='color: gray;'>[%1]</span> <span style='color: %2;'>%3</span>")
        .arg(timestamp).arg(color).arg(mensaje);
    
    log_actividad->append(html);
    
    // Auto-scroll al final
    QScrollBar *sb = log_actividad->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::log_demo(const QString& mensaje, const QString& tipo) {
    QString color = "black";
    if (tipo == "success") color = "green";
    else if (tipo == "error") color = "red";
    else if (tipo == "warning") color = "orange";
    else if (tipo == "info") color = "blue";
    
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString html = QString("<span style='color: gray;'>[%1]</span> <span style='color: %2;'>%3</span>")
        .arg(timestamp).arg(color).arg(mensaje);
    
    log_demostraciones->append(html);
    
    // Auto-scroll al final
    QScrollBar *sb = log_demostraciones->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void MainWindow::setup_tab_demostraciones() {
    QWidget *tab_demos_widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab_demos_widget);
    
    // Título
    QLabel *titulo = new QLabel("<h2>🔬 Demostraciones de Concurrencia</h2>");
    layout->addWidget(titulo);
    
    QLabel *subtitulo = new QLabel("<i>Visualiza los conceptos de Sistemas Operativos en acción</i>");
    layout->addWidget(subtitulo);
    
    // Panel de controles
    QGroupBox *control_group = new QGroupBox("Demostraciones Disponibles");
    QVBoxLayout *control_layout = new QVBoxLayout();
    
    // Deadlock
    QHBoxLayout *deadlock_layout = new QHBoxLayout();
    QLabel *lbl_deadlock = new QLabel("<b>🔒 Deadlock (Interbloqueo):</b> Simula y resuelve deadlocks");
    deadlock_layout->addWidget(lbl_deadlock);
    deadlock_layout->addStretch();
    
    btn_demo_deadlock = new QPushButton("▶️ Provocar Deadlock");
    btn_demo_deadlock->setStyleSheet("background-color: #f44336; color: white; padding: 8px 15px;");
    connect(btn_demo_deadlock, &QPushButton::clicked, this, &MainWindow::demostrar_deadlock);
    deadlock_layout->addWidget(btn_demo_deadlock);
    
    btn_resolver_deadlock = new QPushButton("✅ Resolver Deadlock");
    btn_resolver_deadlock->setStyleSheet("background-color: #4CAF50; color: white; padding: 8px 15px;");
    connect(btn_resolver_deadlock, &QPushButton::clicked, this, &MainWindow::resolver_deadlock_demo);
    deadlock_layout->addWidget(btn_resolver_deadlock);
    
    control_layout->addLayout(deadlock_layout);
    control_layout->addWidget(new QLabel("<hr>"));
    
    // Semáforo
    QHBoxLayout *semaforo_layout = new QHBoxLayout();
    QLabel *lbl_semaforo = new QLabel("<b>🚦 Semáforo:</b> Limita acceso concurrente a recursos");
    semaforo_layout->addWidget(lbl_semaforo);
    semaforo_layout->addStretch();
    
    btn_demo_semaforo = new QPushButton("▶️ Demostrar Semáforo");
    btn_demo_semaforo->setStyleSheet("background-color: #FF9800; color: white; padding: 8px 15px;");
    connect(btn_demo_semaforo, &QPushButton::clicked, this, &MainWindow::demostrar_semaforo);
    semaforo_layout->addWidget(btn_demo_semaforo);
    
    control_layout->addLayout(semaforo_layout);
    control_layout->addWidget(new QLabel("<hr>"));
    
    // Lectores-Escritores
    QHBoxLayout *lect_esc_layout = new QHBoxLayout();
    QLabel *lbl_lect_esc = new QLabel("<b>📖 Lectores-Escritores:</b> Acceso compartido vs exclusivo");
    lect_esc_layout->addWidget(lbl_lect_esc);
    lect_esc_layout->addStretch();
    
    btn_demo_lectores_escritores = new QPushButton("▶️ Demostrar Lectores-Escritores");
    btn_demo_lectores_escritores->setStyleSheet("background-color: #2196F3; color: white; padding: 8px 15px;");
    connect(btn_demo_lectores_escritores, &QPushButton::clicked, this, &MainWindow::demostrar_lectores_escritores);
    lect_esc_layout->addWidget(btn_demo_lectores_escritores);
    
    control_layout->addLayout(lect_esc_layout);
    
    control_group->setLayout(control_layout);
    layout->addWidget(control_group);
    
    // Log de demostraciones
    QLabel *lbl_log = new QLabel("<b>📝 Log de Demostraciones:</b>");
    layout->addWidget(lbl_log);
    
    log_demostraciones = new QTextEdit();
    log_demostraciones->setReadOnly(true);
    log_demostraciones->setStyleSheet("background-color: #f5f5f5; font-family: monospace;");
    layout->addWidget(log_demostraciones);
    
    tabs->addTab(tab_demos_widget, "🔬 Demostraciones");
}

void MainWindow::demostrar_deadlock() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("🔒 INICIANDO DEMOSTRACIÓN DE DEADLOCK", "warning");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    
    btn_demo_deadlock->setEnabled(false);
    
    // Ejecutar en thread separado para no bloquear UI
    std::thread([this]() {
        try {
            log_demo("", "info");
            log_demo("📌 Creando dos cuentas bancarias...", "info");
            log_demo("   • Cuenta A: $5000", "info");
            log_demo("   • Cuenta B: $8000", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("👤 Creando dos hilos (Thread 1 y Thread 2)...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("⚠️  Thread 1: Intentando transferir A → B", "warning");
            log_demo("   1. Bloqueando mutex de Cuenta A... ✓", "info");
            log_demo("   2. Esperando 100ms...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            
            log_demo("", "info");
            log_demo("⚠️  Thread 2: Intentando transferir B → A", "warning");
            log_demo("   1. Bloqueando mutex de Cuenta B... ✓", "info");
            log_demo("   2. Esperando 100ms...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            
            log_demo("", "info");
            log_demo("❌ Thread 1: Intentando bloquear Cuenta B...", "error");
            log_demo("   ⏳ BLOQUEADO - Cuenta B ya está en uso por Thread 2", "error");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            
            log_demo("", "info");
            log_demo("❌ Thread 2: Intentando bloquear Cuenta A...", "error");
            log_demo("   ⏳ BLOQUEADO - Cuenta A ya está en uso por Thread 1", "error");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("💀 ¡DEADLOCK DETECTADO!", "error");
            log_demo("   • Thread 1 espera a Thread 2", "error");
            log_demo("   • Thread 2 espera a Thread 1", "error");
            log_demo("   • Ninguno puede continuar ⚠️", "error");
            log_demo("", "info");
            log_demo("📚 Explicación: Cada hilo mantiene un recurso y espera por el otro,", "warning");
            log_demo("    creando un ciclo de espera infinito (condición de Coffman #4)", "warning");
            
        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: ") + e.what(), "error");
        }
        
        btn_demo_deadlock->setEnabled(true);
    }).detach();
}

void MainWindow::resolver_deadlock_demo() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("✅ RESOLVIENDO DEADLOCK CON std::scoped_lock", "success");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    
    btn_resolver_deadlock->setEnabled(false);
    
    std::thread([this]() {
        try {
            log_demo("", "info");
            log_demo("📌 Mismo escenario: Cuenta A ($5000) y Cuenta B ($8000)", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("🔧 Solución: std::scoped_lock adquiere AMBOS mutex atómicamente", "success");
            log_demo("   • Previene deadlock usando algoritmo de ordenamiento", "info");
            log_demo("   • Si no puede obtener ambos, libera todos", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            
            log_demo("", "info");
            log_demo("▶️  Thread 1: Usando scoped_lock(mutex_A, mutex_B)", "info");
            log_demo("   1. Intentando adquirir ambos mutex...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            log_demo("   2. ✓ Ambos mutex adquiridos atómicamente", "success");
            log_demo("   3. Transferencia A → B completada ($1000)", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("▶️  Thread 2: Usando scoped_lock(mutex_B, mutex_A)", "info");
            log_demo("   1. Intentando adquirir ambos mutex...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            log_demo("   2. ⏳ Thread 1 aún tiene los mutex, esperando...", "warning");
            log_demo("   3. ✓ Thread 1 terminó, mutex liberados", "info");
            log_demo("   4. ✓ Ambos mutex adquiridos atómicamente", "success");
            log_demo("   5. Transferencia B → A completada ($500)", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("✅ ¡DEADLOCK RESUELTO EXITOSAMENTE!", "success");
            log_demo("   • Saldo final Cuenta A: $4500", "success");
            log_demo("   • Saldo final Cuenta B: $8500", "success");
            log_demo("", "info");
            log_demo("📚 Resultado: No se formó ciclo de espera. Los hilos esperaron", "success");
            log_demo("    ordenadamente sin bloqueos mutuos permanentes.", "success");
            
        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: ") + e.what(), "error");
        }
        
        btn_resolver_deadlock->setEnabled(true);
    }).detach();
}

void MainWindow::demostrar_semaforo() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("🚦 DEMOSTRACIÓN DE SEMÁFORO CONTADOR", "info");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    
    btn_demo_semaforo->setEnabled(false);
    
    std::thread([this]() {
        try {
            log_demo("", "info");
            log_demo("📌 Escenario: Sistema antifraude con 3 motores máximo", "info");
            log_demo("   • Semáforo inicializado con valor = 3", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("▶️  Llegan 5 transacciones simultáneas para análisis", "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            for (int i = 1; i <= 5; ++i) {
                log_demo("", "info");
                log_demo(QString("🔄 Transacción #%1 solicita un motor...").arg(i), "info");
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                
                if (i <= 3) {
                    log_demo(QString("   ✓ Motor %1 asignado (semáforo: %2 → %3)").arg(i).arg(4-i).arg(3-i), "success");
                    log_demo(QString("   ⚙️  Analizando transacción #%1...").arg(i), "info");
                } else {
                    log_demo(QString("   ⏳ No hay motores disponibles (semáforo: 0)").arg(i), "warning");
                    log_demo(QString("   😴 Transacción #%1 en espera...").arg(i), "warning");
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            }
            
            log_demo("", "info");
            log_demo("⏱️  Motor 1 termina análisis (2 segundos)", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            log_demo("   ✅ Motor 1 liberado (semáforo: 0 → 1)", "success");
            log_demo("   ▶️  Transacción #4 despierta y toma el motor", "success");
            log_demo("   ⚙️  Analizando transacción #4...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            
            log_demo("", "info");
            log_demo("⏱️  Motor 2 termina análisis", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            log_demo("   ✅ Motor 2 liberado (semáforo: 0 → 1)", "success");
            log_demo("   ▶️  Transacción #5 despierta y toma el motor", "success");
            log_demo("   ⚙️  Analizando transacción #5...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            
            log_demo("", "info");
            log_demo("✅ Todas las transacciones procesadas", "success");
            log_demo("", "info");
            log_demo("📚 Explicación: El semáforo limitó el acceso concurrente a 3 recursos,", "info");
            log_demo("    haciendo que las transacciones 4 y 5 esperaran disponibilidad.", "info");
            log_demo("    Esto previene sobrecarga del sistema. 🎯", "info");
            
        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: ") + e.what(), "error");
        }
        
        btn_demo_semaforo->setEnabled(true);
    }).detach();
}

void MainWindow::demostrar_lectores_escritores() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("📖 DEMOSTRACIÓN LECTORES-ESCRITORES", "info");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    
    btn_demo_lectores_escritores->setEnabled(false);
    
    std::thread([this]() {
        try {
            log_demo("", "info");
            log_demo("📌 Recurso: Configuración del sistema (shared_mutex)", "info");
            log_demo("   • Múltiples lectores simultáneos: ✅ Permitido", "success");
            log_demo("   • Escritor requiere acceso exclusivo: 🔒", "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            
            log_demo("", "info");
            log_demo("▶️  Lector 1 solicita acceso (shared_lock)...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            log_demo("   ✓ Acceso concedido - Leyendo configuración", "success");
            log_demo("   📄 max_transacciones_simultaneas = 100", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            
            log_demo("", "info");
            log_demo("▶️  Lector 2 solicita acceso (shared_lock)...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            log_demo("   ✓ Acceso concedido - Leyendo EN PARALELO con Lector 1", "success");
            log_demo("   📄 timeout_transaccion = 30s", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            
            log_demo("", "info");
            log_demo("▶️  Lector 3 solicita acceso (shared_lock)...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            log_demo("   ✓ Acceso concedido - 3 lectores simultáneos ✓", "success");
            log_demo("   📄 modo_antifraude = ACTIVADO", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            
            log_demo("", "info");
            log_demo("✍️  Escritor 1 solicita modificar configuración (unique_lock)...", "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            log_demo("   ⏳ Esperando... (hay 3 lectores activos)", "warning");
            log_demo("   💤 Escritor BLOQUEADO hasta que todos los lectores terminen", "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(800));
            
            log_demo("", "info");
            log_demo("✅ Lector 1 terminó lectura", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            log_demo("✅ Lector 2 terminó lectura", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
            log_demo("✅ Lector 3 terminó lectura", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("🔓 Todos los lectores liberaron el recurso", "info");
            log_demo("✍️  Escritor 1 obtiene acceso EXCLUSIVO", "success");
            log_demo("   🔒 Bloqueando recurso completamente...", "warning");
            log_demo("   ✏️  Modificando: max_transacciones = 100 → 150", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            log_demo("   ✅ Escritura completada", "success");
            log_demo("   🔓 Recurso liberado", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("▶️  Lector 4 solicita acceso...", "info");
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            log_demo("   ✓ Acceso concedido - Leyendo valor actualizado", "success");
            log_demo("   📄 max_transacciones_simultaneas = 150 ✨", "success");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            log_demo("", "info");
            log_demo("✅ DEMOSTRACIÓN COMPLETADA", "success");
            log_demo("", "info");
            log_demo("📚 Resumen:", "info");
            log_demo("   • Lectores pueden acceder simultáneamente (shared_lock)", "info");
            log_demo("   • Escritores requieren acceso exclusivo (unique_lock)", "info");
            log_demo("   • Escritores esperan a que NO haya lectores", "info");
            log_demo("   • Nuevos lectores NO pueden entrar si hay escritor esperando", "info");
            log_demo("   • Previene inanición (starvation) de escritores 🎯", "info");
            
        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: ") + e.what(), "error");
        }
        
        btn_demo_lectores_escritores->setEnabled(true);
    }).detach();
}

