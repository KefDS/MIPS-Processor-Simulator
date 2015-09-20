#include "nucleo.h"

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

}
