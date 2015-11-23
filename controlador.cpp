#include "controlador.h"
#include "mainwindow.h"

Controlador::Controlador (const QStringList& rutas_archivos, const Datos_usuario& datos, QObject* parent) :
	QObject (parent),
	m_procesador(new Procesador(rutas_archivos, datos.latencia_de_memoria, datos.trasferencia, datos.quatum))
{
	Nucleo* nucleo_0 = new Nucleo(*m_procesador, 0);
	Nucleo* nucleo_1 = new Nucleo(*m_procesador, 1);

	// Se mueven el procesador y los núcleos a hilos diferentes
	nucleo_0->moveToThread (&m_thread_nucleo_0);
	nucleo_1->moveToThread (&m_thread_nucleo_1);


	// Conexiones

	// Cuando el hilo comienze, el procesador y los núcleos comenzarán su ejecucción
	connect (&m_thread_nucleo_0, &QThread::started, nucleo_0, &Nucleo::run);
	connect (&m_thread_nucleo_1, &QThread::started, nucleo_1, &Nucleo::run);

	// Cuando termine de ejecutarse el hilo, se eliminarán los objetos contenidos en ellos
	connect(&m_thread_nucleo_0, &QThread::finished, nucleo_0, &QObject::deleteLater);
	connect(&m_thread_nucleo_1, &QThread::finished, nucleo_1, &QObject::deleteLater);

	// Conecta el estado que envía el núcleo a la interfaz gráfica
	connect(nucleo_0, &Nucleo::reportar_estado, this, &Controlador::enviar_estado);
	connect(nucleo_1, &Nucleo::reportar_estado, this, &Controlador::enviar_estado);

	// Conecta el estado que envía el procesador a la interfaz gráfica
	connect(m_procesador, &Procesador::aumenta_reloj, this, &Controlador::aumenta_reloj);
	connect(m_procesador, &Procesador::reportar_estado, this, &Controlador::enviar_estado);

	// Conecta los signals de los núcleos con el textedit widget de la intefaz gráfica
	connect(this, &Controlador::enviar_estado, reinterpret_cast<MainWindow*>(parent), &MainWindow::imprimir_estado);
	connect(this, &Controlador::aumenta_reloj, reinterpret_cast<MainWindow*>(parent), &MainWindow::aumenta_reloj);
}

Controlador::~Controlador() {
	m_thread_nucleo_0.quit();
	m_thread_nucleo_0.wait();

	m_thread_nucleo_1.quit();
	m_thread_nucleo_1.wait();

	delete m_procesador;
}

void Controlador::comenzar_simulacion() {
	m_thread_nucleo_0.start();
	m_thread_nucleo_1.start();
}
