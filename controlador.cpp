#include "controlador.h"

Controlador::Controlador (const QStringList& informacion, QObject* parent) :
	QObject (parent)
{
	QStringList nombres_archivos;
	for (int i = 2; i < informacion.length (); ++i) {
		nombres_archivos << informacion.at(i);
	}

	Procesador* procesador = new Procesador(nombres_archivos, informacion.at(0).toInt(), informacion.at(1).toInt());
	Nucleo* nucleo_1 = new Nucleo(*procesador, "Nucleo 1");
	Nucleo* nucleo_2 = new Nucleo(*procesador, "Nucleo 2");

	// Se mueven el procesador y los núcleos a hilos diferentes
	procesador->moveToThread (&m_thread_procesador);
	nucleo_1->moveToThread (&m_thread_nucleo_1);
	nucleo_2->moveToThread (&m_thread_nucleo_2);

	// Conexiones

	// Cuando el hilo comienze, el procesador y los núcleos comenzarán su ejecucción
	connect (&m_thread_procesador, &QThread::started, procesador, &Procesador::run);
	connect (&m_thread_nucleo_1, &QThread::started, nucleo_1, &Nucleo::run);
	connect (&m_thread_nucleo_2, &QThread::started, nucleo_2, &Nucleo::run);

	// Cuando termine de ejecutarse el hilo, se eliminarán los objetos contenidos en ellos
	connect(&m_thread_procesador, &QThread::finished, procesador, &QObject::deleteLater);
	connect(&m_thread_nucleo_1, &QThread::finished, nucleo_1, &QObject::deleteLater);
	connect(&m_thread_nucleo_2, &QThread::finished, nucleo_2, &QObject::deleteLater);
}

Controlador::~Controlador() {
	m_thread_procesador.quit();
	m_thread_procesador.wait();

	m_thread_nucleo_1.quit();
	m_thread_nucleo_1.wait();

	m_thread_nucleo_2.quit();
	m_thread_nucleo_2.wait();
}

void Controlador::comenzar_simulacion() {
	// Empieza la ejecucción de los hilos
	m_thread_procesador.start();
	m_thread_nucleo_1.start();
	m_thread_nucleo_2.start();
}

void Controlador::imprimir_estado(QString estado) {
	qDebug() << estado;
}
