#include "controlador.h"
#include "mainwindow.h"

Controlador::Controlador (const QStringList& rutas_archivos, const Datos_usuario& datos, QObject* parent) :
	QObject (parent)
{
	Procesador* procesador	= new Procesador(rutas_archivos, datos.latencia_de_memoria, datos.trasferencia, datos.quatum);
	Nucleo* nucleo_1		= new Nucleo(*procesador, "Nucleo 1");
	Nucleo* nucleo_2		= new Nucleo(*procesador, "Nucleo 2");

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
    connect(nucleo_1, &Nucleo::reportar_estado, this, &Controlador::enviar_estado);
    connect(nucleo_2, &Nucleo::reportar_estado, this, &Controlador::enviar_estado);
	// Revisar!
	connect(this, &Controlador::enviar_estado, reinterpret_cast<MainWindow*>(parent), &MainWindow::imprimir_estado);
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
   // qDebug() << "empieza";
	m_thread_procesador.start();
	m_thread_nucleo_1.start();
	m_thread_nucleo_2.start();
}

void Controlador::imprimir_estado(QString estado) {
	qDebug() << estado;
}
