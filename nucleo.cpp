#include "nucleo.h"
#include <QThread>
#include <QDebug>

Nucleo::Nucleo (Procesador& procesador, const QString& nombre, QObject* parent) :
	QObject (parent),
	nombre(nombre),
	m_procesador (procesador),
	m_cache_instrucciones(new int[CACHE]),
	m_registros(new int[NUMERO_REGISTROS])
{

}

Nucleo::~Nucleo() {
	delete[] m_registros;
}

void Nucleo::run() {
    QString estado = this->nombre + " Ejecutándose";
    emit reportar_estado(estado);

   //  int numeroBloque = PC/16;
   //  int numeroPagina = (PC%16)/4;
}
