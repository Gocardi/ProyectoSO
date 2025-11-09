QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = simulador_qt
TEMPLATE = app

CONFIG += c++17

# Directorios
INCLUDEPATH += include
OBJECTS_DIR = obj
MOC_DIR = moc
UI_DIR = ui

# Archivos fuente
SOURCES += \
    src/main_qt.cpp \
    src/mainwindow.cpp \
    src/database_json.cpp \
    src/productor_consumidor.cpp \
    src/lectores_escritores.cpp \
    src/monitor.cpp \
    src/deadlock.cpp

# Archivos de cabecera
HEADERS += \
    include/mainwindow.hpp \
    include/database_json.hpp \
    include/modelos.hpp \
    include/productor_consumidor.hpp \
    include/lectores_escritores.hpp \
    include/monitor.hpp \
    include/deadlock.hpp \
    include/semaforo.hpp

# Flags del compilador
QMAKE_CXXFLAGS += -pthread
LIBS += -lpthread

# Configuración de compilación
DEFINES += QT_DEPRECATED_WARNINGS

# Deshabilitar warnings específicos si es necesario
#QMAKE_CXXFLAGS += -Wno-unused-parameter

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
