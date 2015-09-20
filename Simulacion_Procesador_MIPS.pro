#-------------------------------------------------
#
# Project created by QtCreator 2015-09-15T09:08:09
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET = Simulacion_Procesador_MIPS
CONFIG   += c++11
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    nucleo.cpp \
    procesador.cpp \
    controlador.cpp

HEADERS += \
    nucleo.h \
    procesador.h \
    controlador.h
