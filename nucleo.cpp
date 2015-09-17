#include "nucleo.h"

Nucleo::Nucleo (Procesador& procesador, QObject* parent) :
	QObject (parent),
	m_procesador (procesador),
	m_registros(new int[NUMERO_REGISTROS])
{

}

Nucleo::~Nucleo()
{
	delete[] m_registros;
}

