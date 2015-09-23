#-------------------------------------------------
#
# Project created by QtCreator 2015-09-15T09:08:09
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = Simulacion_Procesador_MIPS
CONFIG   += c++11

TEMPLATE = app

SOURCES += main.cpp \
    nucleo.cpp \
    procesador.cpp \
    controlador.cpp \
    mainwindow.cpp

HEADERS += \
    nucleo.h \
    procesador.h \
    controlador.h \
    mainwindow.h \
    definiciones.h

FORMS    += mainwindow.ui
