#include "controller.h"

Controller::Controller (const QStringList& information, QObject* parent) :
	QObject (parent)
{
	QStringList file_names;
	for (int i = 2; i < information.length (); ++i) {
		file_names << information.at(i);
	}

	m_procesador = new Procesador(file_names, information.at(0).toInt(), information.at(1).toInt(), this);
	m_nucleo_1 = new Nucleo(*m_procesador);
	m_nucleo_2 = new Nucleo(*m_procesador);
}

