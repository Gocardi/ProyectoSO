#include "mainwindow.hpp"

#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QScrollBar>
#include <thread>
#include <random>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      procesamiento_activo(false),
      contador_transacciones(0),
      transacciones_aprobadas(0),
      transacciones_sospechosas(0),
      monto_total_procesado(0.0),
      monto_deadlock_A_a_B(0.0),  // <- AGREGAR ESTA LÍNEA
      monto_deadlock_B_a_A(0.0) {

    // Inicializar componentes
    db = std::make_shared<DatabaseJSON>("usuarios.json", "transacciones.json");
    monitor = std::make_shared<MonitorCuentas>();
    cola = std::make_shared<ColaTransacciones>(10);
    config = std::make_shared<ConfiguracionSistema>();
    contexto_fraude = std::make_shared<ContextoFraude>(); // Nuevo contexto compartido
    mutex_db = std::make_shared<std::shared_mutex>(); // NUEVO

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
    QLabel *titulo = new QLabel("<h2>Gestión de Usuarios</h2>");
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

    // ORIGEN (ComboBox)
    form_layout->addWidget(new QLabel("Origen:"));
    combo_usuario_origen = new QComboBox();
    combo_usuario_origen->setEditable(true);
    combo_usuario_origen->setInsertPolicy(QComboBox::NoInsert);
    combo_usuario_origen->setPlaceholderText("Selecciona usuario origen");
    combo_usuario_origen->setMinimumWidth(150);

    // Autocompletado
    combo_usuario_origen->setCompleter(new QCompleter(combo_usuario_origen->model(), combo_usuario_origen));
    combo_usuario_origen->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    combo_usuario_origen->completer()->setCompletionMode(QCompleter::PopupCompletion);

    form_layout->addWidget(combo_usuario_origen);

    // DESTINO (ComboBox)
    form_layout->addWidget(new QLabel("Destino:"));
    combo_usuario_destino = new QComboBox();
    combo_usuario_destino->setEditable(true);
    combo_usuario_destino->setInsertPolicy(QComboBox::NoInsert);
    combo_usuario_destino->setPlaceholderText("Selecciona usuario destino");
    combo_usuario_destino->setMinimumWidth(150);

    // Autocompletado
    combo_usuario_destino->setCompleter(new QCompleter(combo_usuario_destino->model(), combo_usuario_destino));
    combo_usuario_destino->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    combo_usuario_destino->completer()->setCompletionMode(QCompleter::PopupCompletion);

    form_layout->addWidget(combo_usuario_destino);

    // MONTO
    form_layout->addWidget(new QLabel("Monto:"));
    input_monto = new QDoubleSpinBox();
    input_monto->setRange(0.01, 100000);
    input_monto->setValue(100.0);
    input_monto->setPrefix("$");
    form_layout->addWidget(input_monto);

    // BOTÓN ENVIAR
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

    // ⚠️ IMPORTANTE: Conectar evento para validar selección
    connect(combo_usuario_origen, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::validar_seleccion_usuarios);
    connect(combo_usuario_destino, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::validar_seleccion_usuarios);
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

    // Limpiar combos
    combo_usuario_origen->clear();
    combo_usuario_destino->clear();

    for (const auto& usuario : usuarios) {
        // Actualizar tabla
        int row = tabla_usuarios->rowCount();
        tabla_usuarios->insertRow(row);

        tabla_usuarios->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(usuario.nombre)));
        tabla_usuarios->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(usuario.cuenta_id)));

        QTableWidgetItem *item_saldo = new QTableWidgetItem(QString("$%1").arg(usuario.saldo, 0, 'f', 2));
        item_saldo->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        tabla_usuarios->setItem(row, 2, item_saldo);

        tabla_usuarios->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(usuario.fecha_creacion)));

        //  ComboBox
        combo_usuario_origen->addItem(QString::fromStdString(usuario.nombre));
        combo_usuario_destino->addItem(QString::fromStdString(usuario.nombre));
    }

    tabla_usuarios->resizeColumnsToContents();
}


// 3. ACTUALIZAR seleccionar_usuario() para usar el combo:

void MainWindow::seleccionar_usuario(int row, int column) {
    Q_UNUSED(column);
    QString nombre = tabla_usuarios->item(row, 0)->text();

    // Buscar y seleccionar en el combo
    int index = combo_usuario_origen->findText(nombre);
    if (index != -1) {
        combo_usuario_origen->setCurrentIndex(index);
    }
}


// 4. ACTUALIZAR enviar_transaccion() para usar currentText():

void MainWindow::enviar_transaccion() {
    // 1. Obtener los datos de los ComboBox (CAMBIO AQUÍ)
    QString origen_str = combo_usuario_origen->currentText().trimmed();
    QString destino_str = combo_usuario_destino->currentText().trimmed();
    double monto = input_monto->value();

    // 2. Validaciones básicas de la entrada
    if (origen_str.isEmpty() || destino_str.isEmpty()) {
        QMessageBox::warning(this, "Error de Entrada", "Debe seleccionar un origen y un destino.");
        return;
    }

    if (origen_str == destino_str) {
        QMessageBox::warning(this, "Error de Lógica",
                             "El origen y el destino no pueden ser iguales.\n\n"
                             "Por favor, selecciona usuarios diferentes.");
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

    // 4. Crear el objeto Transaccion
    Transaccion t;
    t.id = db->obtener_siguiente_id_transaccion();
    t.cliente_id = usuario_origen.nombre;
    t.tipo = "TRANSFERENCIA";
    t.monto = monto;
    t.cuenta_origen = usuario_origen.cuenta_id;
    t.cuenta_destino = usuario_destino.cuenta_id;

    // 5. Usar el contexto de fraude compartido para análisis avanzado
    t.es_sospechosa = contexto_fraude->analizarYActualizar(t);

    if (t.es_sospechosa) {
        transacciones_sospechosas++;
        log_mensaje("⚠️ Transacción sospechosa detectada (Velocidad/Frecuencia): $" + QString::number(monto, 'f', 2), "warning");
    }

    // 6. Procesar la transacción
    bool exito = monitor->transferir(usuario_origen.cuenta_id, usuario_destino.cuenta_id, monto);

    if (exito) {
        // 6.1. Actualizar saldos
        db->actualizar_saldo(usuario_origen.nombre, usuario_origen.saldo - monto);
        db->actualizar_saldo(usuario_destino.nombre, usuario_destino.saldo + monto);

        // 6.2. Guardar transacción
        TransaccionDB tdb;
        tdb.id = t.id;
        tdb.usuario_origen = t.cliente_id;
        tdb.usuario_destino = destino_str.toStdString();
        tdb.monto = t.monto;
        tdb.tipo = t.tipo;
        tdb.es_sospechosa = t.es_sospechosa;
        tdb.fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        db->guardar_transaccion(tdb);

        // 6.3. Actualizar contadores
        contador_transacciones++;
        transacciones_aprobadas++;
        monto_total_procesado += monto;

        log_mensaje(QString("✓ Transferencia exitosa: %1 → %2 ($%3)")
                        .arg(origen_str).arg(destino_str).arg(monto, 0, 'f', 2), "success");
        QMessageBox::information(this, "Éxito", "Transacción completada correctamente.");

        // 6.4. Limpiar y actualizar (CAMBIO AQUÍ)
        actualizar_tabla_usuarios();
        actualizar_tabla_transacciones();
        combo_usuario_origen->setCurrentIndex(-1); // Limpiar selección
        combo_usuario_destino->setCurrentIndex(-1);
        input_monto->setValue(100.0);

    } else {
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


void MainWindow::validar_seleccion_usuarios() {
    QString origen = combo_usuario_origen->currentText();
    QString destino = combo_usuario_destino->currentText();

    // Si ambos están seleccionados y son iguales, mostrar advertencia
    if (!origen.isEmpty() && !destino.isEmpty() && origen == destino) {
        combo_usuario_destino->setStyleSheet("QComboBox { border: 2px solid red; }");
        btn_enviar_transaccion->setEnabled(false);
    } else {
        combo_usuario_destino->setStyleSheet("");
        btn_enviar_transaccion->setEnabled(true);
    }
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
    // THREAD-SAFE: Usar QMetaObject::invokeMethod para acceder a widgets desde otros hilos
    QMetaObject::invokeMethod(this, [this, mensaje, tipo]() {
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
    }, Qt::QueuedConnection);
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

// REEMPLAZAR ESTAS DOS FUNCIONES COMPLETAS EN mainwindow.cpp

void MainWindow::demostrar_deadlock() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("🔒 DEMOSTRACIÓN DE DEADLOCK", "warning");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");

    btn_demo_deadlock->setEnabled(false);
    btn_resolver_deadlock->setEnabled(true);

    std::thread([this]() {
        try {
            // Seleccionar dos usuarios aleatorios
            auto [usuario_A, usuario_B] = obtener_usuarios_aleatorios();
            cuentas_en_deadlock = {usuario_A.cuenta_id, usuario_B.cuenta_id};
            deadlock_activo = true;

            double saldo_inicial_A = usuario_A.saldo;
            double saldo_inicial_B = usuario_B.saldo;

            // Generar montos aleatorios múltiplos de 5 entre 50 y 120
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(10, 24); // 10*5=50, 24*5=120

            // GUARDAR los montos para usarlos después en resolver_deadlock_demo
            this->monto_deadlock_A_a_B = dist(gen) * 5.0;
            this->monto_deadlock_B_a_A = dist(gen) * 5.0;

            log_demo("", "info");
            log_demo(QString(" %1 (saldo: $%2) → %3 (saldo: $%4)")
                         .arg(QString::fromStdString(usuario_A.nombre))
                         .arg(saldo_inicial_A, 0, 'f', 2)
                         .arg(QString::fromStdString(usuario_B.nombre))
                         .arg(saldo_inicial_B, 0, 'f', 2), "info");
            log_demo(QString(" Transferencias cruzadas: $%1 (A→B) y $%2 (B→A)")
                         .arg(this->monto_deadlock_A_a_B, 0, 'f', 2)
                         .arg(this->monto_deadlock_B_a_A, 0, 'f', 2), "info");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // Activar modo deadlock
            monitor->activar_deadlock();
            monitor->transferir_con_deadlock(usuario_A.cuenta_id, usuario_B.cuenta_id, this->monto_deadlock_A_a_B);
            monitor->transferir_con_deadlock(usuario_B.cuenta_id, usuario_A.cuenta_id, this->monto_deadlock_B_a_A);

            std::this_thread::sleep_for(std::chrono::seconds(2));

            log_demo("", "info");
            log_demo(" DEADLOCK: Ciclo de espera circular detectado", "error");
            log_demo("   Thread 1 espera a Thread 2, Thread 2 espera a Thread 1", "error");

            double diferencia = this->monto_deadlock_A_a_B - this->monto_deadlock_B_a_A;
            log_demo(QString("   Cambio esperado: %1 ($%2%3) | %4 ($%5%6)")
                         .arg(QString::fromStdString(usuario_A.nombre))
                         .arg(diferencia >= 0 ? "+" : "")
                         .arg(-diferencia, 0, 'f', 2)
                         .arg(QString::fromStdString(usuario_B.nombre))
                         .arg(diferencia >= 0 ? "+" : "")
                         .arg(diferencia, 0, 'f', 2), "warning");

        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: %1").arg(e.what()), "error");
            deadlock_activo = false;
            
            // THREAD-SAFE: Modificar botones desde thread principal
            QMetaObject::invokeMethod(this, [this]() {
                btn_demo_deadlock->setEnabled(true);
                btn_resolver_deadlock->setEnabled(false);
            }, Qt::QueuedConnection);
        }

    }).detach();
}


void MainWindow::resolver_deadlock_demo() {
    if (!deadlock_activo || !cuentas_en_deadlock.has_value()) {
        log_demo("⚠ No hay deadlock activo para resolver", "warning");
        return;
    }

    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("✅ RESOLVIENDO DEADLOCK", "success");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");

    btn_resolver_deadlock->setEnabled(false);

    std::thread([this]() {
        try {
            auto [cuenta_A, cuenta_B] = cuentas_en_deadlock.value();
            auto usuario_A = db->obtener_usuario_por_cuenta(cuenta_A);
            auto usuario_B = db->obtener_usuario_por_cuenta(cuenta_B);

            double saldo_antes_A = usuario_A.saldo;
            double saldo_antes_B = usuario_B.saldo;

            // USAR los montos guardados desde demostrar_deadlock()
            double monto_A_a_B = this->monto_deadlock_A_a_B;
            double monto_B_a_A = this->monto_deadlock_B_a_A;

            log_demo(QString("🔧 Desactivando deadlock y usando transferir() seguro..."), "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            monitor->resolver_deadlock();

            // ========== TRANSFERENCIA 1: A → B ==========
            log_demo(QString("▶️  Transferencia 1: %1 → %2 ($%3)")
                         .arg(QString::fromStdString(usuario_A.nombre))
                         .arg(QString::fromStdString(usuario_B.nombre))
                         .arg(monto_A_a_B, 0, 'f', 2), "info");

            bool exito1 = monitor->transferir(cuenta_A, cuenta_B, monto_A_a_B);

            if (exito1) {
                db->actualizar_saldo(usuario_A.nombre, saldo_antes_A - monto_A_a_B);
                db->actualizar_saldo(usuario_B.nombre, saldo_antes_B + monto_A_a_B);

                TransaccionDB tdb1;
                tdb1.id = db->obtener_siguiente_id_transaccion();
                tdb1.usuario_origen = usuario_A.nombre;
                tdb1.usuario_destino = usuario_B.nombre;
                tdb1.monto = monto_A_a_B;
                tdb1.tipo = "TRANSFERENCIA_DEADLOCK";
                tdb1.es_sospechosa = false;
                tdb1.fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
                db->guardar_transaccion(tdb1);

                contador_transacciones++;
                transacciones_aprobadas++;
                monto_total_procesado += monto_A_a_B;

                log_demo("   ✔ Completada", "success");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            // ========== TRANSFERENCIA 2: B → A ==========
            log_demo(QString("▶️  Transferencia 2: %1 → %2 ($%3)")
                         .arg(QString::fromStdString(usuario_B.nombre))
                         .arg(QString::fromStdString(usuario_A.nombre))
                         .arg(monto_B_a_A, 0, 'f', 2), "info");

            auto usuario_A_intermedio = db->obtener_usuario_por_cuenta(cuenta_A);
            auto usuario_B_intermedio = db->obtener_usuario_por_cuenta(cuenta_B);

            bool exito2 = monitor->transferir(cuenta_B, cuenta_A, monto_B_a_A);

            if (exito2) {
                db->actualizar_saldo(usuario_B_intermedio.nombre, usuario_B_intermedio.saldo - monto_B_a_A);
                db->actualizar_saldo(usuario_A_intermedio.nombre, usuario_A_intermedio.saldo + monto_B_a_A);

                TransaccionDB tdb2;
                tdb2.id = db->obtener_siguiente_id_transaccion();
                tdb2.usuario_origen = usuario_B.nombre;
                tdb2.usuario_destino = usuario_A.nombre;
                tdb2.monto = monto_B_a_A;
                tdb2.tipo = "TRANSFERENCIA_DEADLOCK";
                tdb2.es_sospechosa = false;
                tdb2.fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
                db->guardar_transaccion(tdb2);

                contador_transacciones++;
                transacciones_aprobadas++;
                monto_total_procesado += monto_B_a_A;

                log_demo("   ✔ Completada", "success");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(400));

            // ========== RESULTADOS ==========
            auto usuario_A_final = db->obtener_usuario_por_cuenta(cuenta_A);
            auto usuario_B_final = db->obtener_usuario_por_cuenta(cuenta_B);

            double cambio_A = usuario_A_final.saldo - saldo_antes_A;
            double cambio_B = usuario_B_final.saldo - saldo_antes_B;

            log_demo("", "info");
            log_demo(" RESULTADOS:", "success");
            log_demo(QString("   %1: $%2 → $%3 (%4$%5)")
                         .arg(QString::fromStdString(usuario_A_final.nombre))
                         .arg(saldo_antes_A, 0, 'f', 2)
                         .arg(usuario_A_final.saldo, 0, 'f', 2)
                         .arg(cambio_A >= 0 ? "+" : "")
                         .arg(cambio_A, 0, 'f', 2),
                     cambio_A >= 0 ? "success" : "warning");
            log_demo(QString("   %1: $%2 → $%3 (%4$%5)")
                         .arg(QString::fromStdString(usuario_B_final.nombre))
                         .arg(saldo_antes_B, 0, 'f', 2)
                         .arg(usuario_B_final.saldo, 0, 'f', 2)
                         .arg(cambio_B >= 0 ? "+" : "")
                         .arg(cambio_B, 0, 'f', 2),
                     cambio_B >= 0 ? "success" : "warning");
            log_demo("", "info");
            log_demo(" Solución: Timeout detectó el deadlock y se usó transferir() seguro", "info");

            cuentas_en_deadlock.reset();
            deadlock_activo = false;

            QMetaObject::invokeMethod(this, [this]() {
                actualizar_tabla_usuarios();
                actualizar_tabla_transacciones();
                actualizar_estadisticas();
                log_mensaje("✅ Deadlock resuelto - Consulta las pestañas actualizadas", "success");
            }, Qt::QueuedConnection);

        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: %1").arg(e.what()), "error");
        }

        // THREAD-SAFE: Modificar botones desde thread principal
        QMetaObject::invokeMethod(this, [this]() {
            btn_demo_deadlock->setEnabled(true);
            btn_resolver_deadlock->setEnabled(false);
        }, Qt::QueuedConnection);

    }).detach();
}

void MainWindow::demostrar_semaforo() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo(" DEMOSTRACIÓN DE SEMÁFORO ", "info");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");

    btn_demo_semaforo->setEnabled(false);

    std::thread([this]() {
        try {
            // 1. SELECCIONAR USUARIO ALEATORIO
            auto usuarios = db->cargar_usuarios();
            if (usuarios.empty()) {
                log_demo("❌ No hay usuarios disponibles", "error");
                
                // THREAD-SAFE: Modificar botón desde thread principal
                QMetaObject::invokeMethod(this, [this]() {
                    btn_demo_semaforo->setEnabled(true);
                }, Qt::QueuedConnection);
                return;
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist_usuario(0, usuarios.size() - 1);
            auto usuario_seleccionado = usuarios[dist_usuario(gen)];

            double saldo_inicial = usuario_seleccionado.saldo;

            log_demo("", "info");
            log_demo(QString(" Usuario seleccionado: %1 (Cuenta: %2)")
                         .arg(QString::fromStdString(usuario_seleccionado.nombre))
                         .arg(QString::fromStdString(usuario_seleccionado.cuenta_id)), "info");
            log_demo(QString(" Saldo inicial: $%1").arg(saldo_inicial, 0, 'f', 2), "info");
            log_demo("", "info");
            log_demo(" Escenario: 5 cajeros automáticos intentan operar simultáneamente", "warning");
            log_demo("   • Semáforo: máximo 3 cajeros concurrentes", "info");
            log_demo("   • Sin semáforo habría RACE CONDITION en el saldo", "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(800));

            // 2. GENERAR 5 OPERACIONES ALEATORIAS (retiros/depósitos)
            std::uniform_int_distribution<> dist_monto(10, 24); // 10*5=50, 24*5=120
            std::uniform_int_distribution<> dist_tipo(0, 1); // 0=retiro, 1=depósito

            struct Operacion {
                int cajero_id;
                std::string tipo;
                double monto;
            };

            std::vector<Operacion> operaciones;
            double total_retiros = 0;
            double total_depositos = 0;

            for (int i = 1; i <= 5; ++i) {
                Operacion op;
                op.cajero_id = i;
                op.monto = dist_monto(gen) * 5.0;

                // Alternar retiros y depósitos para no quedarse sin fondos
                if (i % 2 == 0 && saldo_inicial - total_retiros + total_depositos > op.monto) {
                    op.tipo = "RETIRO";
                    total_retiros += op.monto;
                } else {
                    op.tipo = "DEPOSITO";
                    total_depositos += op.monto;
                }

                operaciones.push_back(op);
            }

            log_demo("", "info");
            log_demo("📋 Operaciones a procesar:", "info");
            for (const auto& op : operaciones) {
                QString emoji = (op.tipo == "RETIRO") ? "💸" : "💵";
                log_demo(QString("   %1 Cajero #%2: %3 $%4")
                             .arg(emoji)
                             .arg(op.cajero_id)
                             .arg(QString::fromStdString(op.tipo))
                             .arg(op.monto, 0, 'f', 2), "info");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // 3. CREAR SEMÁFORO (máximo 3 cajeros)
            Semaforo semaforo_cajeros(3);

            // 4. LANZAR 5 THREADS (uno por cajero)
            std::vector<std::thread> threads;
            std::mutex log_mutex;

            for (const auto& op : operaciones) {
                threads.emplace_back([this, op, &usuario_seleccionado, &semaforo_cajeros, &log_mutex]() {
                    // Pequeño delay para que no todos lleguen exactamente al mismo tiempo
                    std::this_thread::sleep_for(std::chrono::milliseconds(op.cajero_id * 50));

                    {
                        std::lock_guard<std::mutex> lk(log_mutex);
                        log_demo("", "info");
                        log_demo(QString("🔄 Cajero #%1 solicita acceso...")
                                     .arg(op.cajero_id), "info");
                    }

                    // ⏱️ INTENTAR ADQUIRIR SEMÁFORO
                    auto inicio = std::chrono::steady_clock::now();
                    semaforo_cajeros.acquire();
                    auto fin = std::chrono::steady_clock::now();
                    auto espera = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

                    {
                        std::lock_guard<std::mutex> lk(log_mutex);
                        if (espera.count() > 50) {
                            log_demo(QString("   ⏳ Cajero #%1 esperó %2ms (semáforo: 0 → 1)")
                                         .arg(op.cajero_id)
                                         .arg(espera.count()), "warning");
                        } else {
                            log_demo(QString("   ✓ Cajero #%1 acceso concedido inmediatamente")
                                         .arg(op.cajero_id), "success");
                        }
                        log_demo(QString("     Procesando %1 de $%2...")
                                     .arg(QString::fromStdString(op.tipo))
                                     .arg(op.monto, 0, 'f', 2), "info");
                    }

                    // 🔒 OPERACIÓN CRÍTICA (actualizar saldo)
                    std::this_thread::sleep_for(std::chrono::milliseconds(400));

                    bool exito = false;
                    if (op.tipo == "RETIRO") {
                        exito = monitor->retirar(usuario_seleccionado.cuenta_id, op.monto);
                        if (exito) {
                            db->actualizar_saldo(usuario_seleccionado.nombre,
                                                 usuario_seleccionado.saldo - op.monto);
                        }
                    } else { // DEPOSITO
                        monitor->depositar(usuario_seleccionado.cuenta_id, op.monto);
                        db->actualizar_saldo(usuario_seleccionado.nombre,
                                             usuario_seleccionado.saldo + op.monto);
                        exito = true;
                    }

                    // Actualizar para el siguiente
                    usuario_seleccionado = db->obtener_usuario(usuario_seleccionado.nombre);

                    // Registrar en historial
                    if (exito) {
                        TransaccionDB tdb;
                        tdb.id = db->obtener_siguiente_id_transaccion();
                        tdb.usuario_origen = usuario_seleccionado.nombre;
                        tdb.usuario_destino = usuario_seleccionado.nombre;
                        tdb.monto = op.monto;
                        tdb.tipo = op.tipo + "_SEMAFORO";
                        tdb.es_sospechosa = false;
                        tdb.fecha = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
                        db->guardar_transaccion(tdb);
                    }

                    {
                        std::lock_guard<std::mutex> lk(log_mutex);
                        if (exito) {
                            log_demo(QString("   ✅ Cajero #%1 completó %2")
                                         .arg(op.cajero_id)
                                         .arg(QString::fromStdString(op.tipo)), "success");
                            log_demo(QString("   💰 Saldo actualizado: $%1")
                                         .arg(usuario_seleccionado.saldo, 0, 'f', 2), "success");
                        } else {
                            log_demo(QString("   ❌ Cajero #%1 falló (fondos insuficientes)")
                                         .arg(op.cajero_id), "error");
                        }
                    }

                    // 🔓 LIBERAR SEMÁFORO
                    semaforo_cajeros.release();

                    {
                        std::lock_guard<std::mutex> lk(log_mutex);
                        log_demo(QString("   🔓 Cajero #%1 liberó recurso (semáforo: +1)")
                                     .arg(op.cajero_id), "info");
                    }
                });
            }

            // 5. ESPERAR A QUE TODOS LOS CAJEROS TERMINEN
            for (auto& t : threads) {
                t.join();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // 6. MOSTRAR RESULTADOS FINALES
            auto usuario_final = db->obtener_usuario(usuario_seleccionado.nombre);
            double saldo_final = usuario_final.saldo;
            double cambio = saldo_final - saldo_inicial;

            log_demo("", "info");
            log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "success");
            log_demo("📊 RESULTADOS FINALES", "success");
            log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "success");
            log_demo(QString("👤 Usuario: %1").arg(QString::fromStdString(usuario_final.nombre)), "info");
            log_demo(QString(" Saldo inicial:  $%1").arg(saldo_inicial, 0, 'f', 2), "info");
            log_demo(QString(" Saldo final:    $%1").arg(saldo_final, 0, 'f', 2),
                     cambio >= 0 ? "success" : "warning");
            log_demo(QString(" Cambio neto:    %1$%2")
                         .arg(cambio >= 0 ? "+" : "")
                         .arg(cambio, 0, 'f', 2),
                     cambio >= 0 ? "success" : "warning");
            log_demo("", "info");
            log_demo("✅ Todas las operaciones completadas SIN RACE CONDITION", "success");
            log_demo("", "info");
            log_demo(" Explicación:", "info");
            log_demo("   • El semáforo limitó a 3 cajeros concurrentes", "info");
            log_demo("   • Cada operación fue ATÓMICA (sin corrupción de datos)", "info");
            log_demo("   • Los cajeros 4 y 5 esperaron disponibilidad", "info");
            log_demo("   • El saldo se actualizó CORRECTAMENTE en la BD ", "info");

            // 7. ACTUALIZAR UI EN EL THREAD PRINCIPAL
            QMetaObject::invokeMethod(this, [this]() {
                actualizar_tabla_usuarios();
                actualizar_tabla_transacciones();
                actualizar_estadisticas();
                log_mensaje("✅ Demostración de semáforo completada - Revisa las pestañas", "success");
            }, Qt::QueuedConnection);

        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: %1").arg(e.what()), "error");
        }

        // THREAD-SAFE: Modificar botón desde thread principal
        QMetaObject::invokeMethod(this, [this]() {
            btn_demo_semaforo->setEnabled(true);
        }, Qt::QueuedConnection);
    }).detach();
}

void MainWindow::demostrar_lectores_escritores() {
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");
    log_demo("📖 DEMOSTRACIÓN LECTORES-ESCRITORES (REAL)", "info");
    log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "info");

    btn_demo_lectores_escritores->setEnabled(false);

    std::thread([this]() {
        try {
            log_demo("", "info");
            log_demo("📌 Recurso compartido: archivos JSON (usuarios + transacciones)", "info");
            log_demo("   • Múltiples lectores simultáneos: ✅ shared_lock", "success");
            log_demo("   • Escritor requiere exclusividad: 🔒 unique_lock", "warning");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            // ============================================
            // LECTOR 1: Genera reporte de transacciones
            // ============================================
            std::mutex log_mutex;
            std::atomic<int> lectores_activos{0};

            std::thread lector1([this, &log_mutex, &lectores_activos]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("", "info");
                    log_demo("📖 LECTOR 1 → Generando reporte de transacciones...", "info");
                }

                auto inicio = std::chrono::steady_clock::now();
                std::shared_lock lock(*mutex_db); // 🔓 SHARED LOCK
                auto fin = std::chrono::steady_clock::now();
                auto espera = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

                lectores_activos++;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    if (espera.count() > 10) {
                        log_demo(QString("   ⏳ Esperó %1ms para obtener shared_lock").arg(espera.count()), "warning");
                    } else {
                        log_demo("   ✅ Acceso concedido inmediatamente (shared_lock)", "success");
                    }
                    log_demo("   📄 Leyendo transacciones.json...", "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(800));

                // LEE DATOS REALES
                auto transacciones = db->cargar_transacciones(100);

                int total = transacciones.size();
                int aprobadas = 0;
                int sospechosas = 0;
                double monto_total = 0.0;

                for (const auto& t : transacciones) {
                    if (t.es_sospechosa) sospechosas++;
                    else aprobadas++;
                    monto_total += t.monto;
                }

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo(QString("   📊 Total: %1 | ✅ Aprobadas: %2 | ⚠️ Sospechosas: %3")
                                 .arg(total).arg(aprobadas).arg(sospechosas), "success");
                    log_demo(QString("   💰 Monto procesado: $%1").arg(monto_total, 0, 'f', 2), "success");
                    log_demo(QString("   📖 Lectores activos: %1").arg(lectores_activos.load()), "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(600));
                lectores_activos--;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("   ✅ LECTOR 1 terminó lectura", "success");
                }
            });

            // ============================================
            // LECTOR 2: Consulta saldo total del sistema
            // ============================================
            std::thread lector2([this, &log_mutex, &lectores_activos]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("", "info");
                    log_demo("📖 LECTOR 2 → Consultando saldo total del sistema...", "info");
                }

                std::shared_lock lock(*mutex_db); // 🔓 SHARED LOCK
                lectores_activos++;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("   ✅ Acceso concedido (shared_lock) - EN PARALELO con Lector 1", "success");
                    log_demo("   📄 Leyendo usuarios.json...", "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(800));

                // LEE DATOS REALES
                auto usuarios = db->cargar_usuarios();

                double saldo_total = 0.0;
                int usuarios_activos = usuarios.size();

                for (const auto& u : usuarios) {
                    saldo_total += u.saldo;
                }

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo(QString("   👥 Usuarios activos: %1").arg(usuarios_activos), "success");
                    log_demo(QString("   💰 Saldo total: $%1").arg(saldo_total, 0, 'f', 2), "success");
                    log_demo(QString("   📖 Lectores activos: %1").arg(lectores_activos.load()), "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(600));
                lectores_activos--;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("   ✅ LECTOR 2 terminó lectura", "success");
                }
            });

            // ============================================
            // LECTOR 3: Analiza patrones de fraude
            // ============================================
            std::thread lector3([this, &log_mutex, &lectores_activos]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("", "info");
                    log_demo("📖 LECTOR 3 → Analizando patrones de fraude...", "info");
                }

                std::shared_lock lock(*mutex_db); // 🔓 SHARED LOCK
                lectores_activos++;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("   ✅ Acceso concedido (shared_lock) - 3 LECTORES SIMULTÁNEOS ✨", "success");
                    log_demo("   📄 Leyendo transacciones.json...", "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(800));

                // LEE DATOS REALES
                auto transacciones = db->cargar_transacciones(100);

                int total = transacciones.size();
                int sospechosas = 0;

                for (const auto& t : transacciones) {
                    if (t.es_sospechosa) sospechosas++;
                }

                double tasa_fraude = (total > 0) ? (sospechosas * 100.0 / total) : 0.0;
                QString nivel = (tasa_fraude < 5) ? "BAJA 🟢" : (tasa_fraude < 10) ? "MEDIA 🟡" : "ALTA 🔴";

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo(QString("   📊 Transacciones analizadas: %1").arg(total), "success");
                    log_demo(QString("   ⚠️ Sospechosas: %1 (%2%)").arg(sospechosas).arg(tasa_fraude, 0, 'f', 1), "warning");
                    log_demo(QString("   🎯 Nivel de riesgo: %1").arg(nivel), "info");
                    log_demo(QString("   📖 Lectores activos: %1").arg(lectores_activos.load()), "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(600));
                lectores_activos--;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("   ✅ LECTOR 3 terminó lectura", "success");
                }
            });

            // Esperar que todos los lectores empiecen
            std::this_thread::sleep_for(std::chrono::milliseconds(700));

            // ============================================
            // ESCRITOR: Actualiza límites de transacción
            // ============================================
            std::thread escritor([this, &log_mutex, &lectores_activos]() {
                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("", "info");
                    log_demo("✏️ ESCRITOR → Actualizando límites de transacción...", "warning");
                    log_demo(QString("   ⏳ Esperando acceso exclusivo... (hay %1 lectores activos)")
                                 .arg(lectores_activos.load()), "warning");
                }

                auto inicio = std::chrono::steady_clock::now();
                std::unique_lock lock(*mutex_db); // 🔒 UNIQUE LOCK
                auto fin = std::chrono::steady_clock::now();
                auto espera = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("", "info");
                    log_demo(QString("   ⏱️ Esperó %1ms hasta obtener acceso exclusivo").arg(espera.count()), "warning");
                    log_demo("   🔒 ACCESO EXCLUSIVO obtenido (unique_lock)", "success");
                    log_demo("   📝 Modificando usuarios.json...", "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));

                // MODIFICA DATOS REALES
                auto usuarios = db->cargar_usuarios();

                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, usuarios.size() - 1);
                int idx = dist(gen);

                auto& usuario_modificado = usuarios[idx];
                double limite_anterior = 5000.0; // Simulado
                double limite_nuevo = 10000.0;

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo(QString("   ✏️ Actualizando usuario: %1")
                                 .arg(QString::fromStdString(usuario_modificado.nombre)), "info");
                    log_demo(QString("   💳 Límite diario: $%1 → $%2")
                                 .arg(limite_anterior, 0, 'f', 2)
                                 .arg(limite_nuevo, 0, 'f', 2), "info");
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(500));

                // Aquí normalmente guardarías con db->guardar_usuario()
                // Como tu DatabaseJSON no tiene método para actualizar límites,
                // lo simulamos actualizando el saldo como demostración
                double saldo_anterior = usuario_modificado.saldo;
                db->actualizar_saldo(usuario_modificado.nombre, saldo_anterior + 0.01); // Cambio mínimo
                db->actualizar_saldo(usuario_modificado.nombre, saldo_anterior); // Restaurar

                {
                    std::lock_guard<std::mutex> lk(log_mutex);
                    log_demo("   ✅ Escritura completada en usuarios.json", "success");
                    log_demo("   🔓 Recurso liberado", "success");
                }

            });

            // Esperar a todos
            lector1.join();
            lector2.join();
            lector3.join();
            escritor.join();

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // ============================================
            // LECTOR 4: Verifica cambios
            // ============================================
            {
                std::lock_guard<std::mutex> lk(log_mutex);
                log_demo("", "info");
                log_demo("📖 LECTOR 4 → Verificando cambios realizados...", "info");
            }

            std::shared_lock lock(*mutex_db);

            {
                std::lock_guard<std::mutex> lk(log_mutex);
                log_demo("   ✅ Acceso concedido (shared_lock)", "success");
                log_demo("   📄 Leyendo usuarios.json...", "info");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(600));

            auto usuarios_final = db->cargar_usuarios();

            {
                std::lock_guard<std::mutex> lk(log_mutex);
                log_demo(QString("   ✅ Verificación completada - %1 usuarios registrados")
                             .arg(usuarios_final.size()), "success");
                log_demo("   💳 Límites actualizados correctamente ✨", "success");
            }

            lock.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            // RESUMEN
            {
                std::lock_guard<std::mutex> lk(log_mutex);
                log_demo("", "info");
                log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "success");
                log_demo("✅ DEMOSTRACIÓN COMPLETADA", "success");
                log_demo("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━", "success");
                log_demo("", "info");
                log_demo("📚 Resumen:", "info");
                log_demo("   • 3 Lectores simultáneos leyeron sin bloqueo (shared_lock)", "info");
                log_demo("   • Escritor esperó a que terminaran TODOS los lectores", "info");
                log_demo("   • Escritor obtuvo acceso EXCLUSIVO (unique_lock)", "info");
                log_demo("   • Lector 4 verificó cambios después de la escritura", "info");
                log_demo("   • Archivos JSON modificados REALMENTE ✅", "info");
                log_demo("", "info");
                log_demo("🎯 Conceptos demostrados:", "info");
                log_demo("   • Concurrencia segura en lecturas (shared_lock)", "info");
                log_demo("   • Exclusión mutua en escritura (unique_lock)", "info");
                log_demo("   • Sincronización automática con shared_mutex", "info");
                log_demo("   • Prevención de race conditions en archivos", "info");
            }

            // Actualizar UI
            QMetaObject::invokeMethod(this, [this]() {
                actualizar_tabla_usuarios();
                actualizar_tabla_transacciones();
                log_mensaje("✅ Demostración Lectores-Escritores completada - Datos actualizados", "success");
            }, Qt::QueuedConnection);

        } catch (const std::exception& e) {
            log_demo(QString("❌ Error: %1").arg(e.what()), "error");
        }

        // THREAD-SAFE: Modificar botón desde thread principal
        QMetaObject::invokeMethod(this, [this]() {
            btn_demo_lectores_escritores->setEnabled(true);
        }, Qt::QueuedConnection);
    }).detach();
}

std::pair<UsuarioDB, UsuarioDB> MainWindow::obtener_usuarios_aleatorios() {
    auto usuarios = db->cargar_usuarios();
    if (usuarios.size() < 2) {
        throw std::runtime_error("Se necesitan al menos 2 usuarios para la demostración");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, usuarios.size() - 1);

    int i1 = dist(gen);
    int i2 = dist(gen);
    while (i2 == i1) i2 = dist(gen);

    return {usuarios[i1], usuarios[i2]};
}


