#include "procesador.h"

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
	QObject (parent),
	m_latencia_de_memoria (latencia_de_memoria),
	m_quantum (quantum),
	m_trasferencia_memoria_cache(trasferencia),
	m_memoria_instrucciones (new int[NUMERO_BYTES_MEMORIA_INSTRUCCIONES]),
	m_cola_procesos(QQueue<Proceso>()),
	m_indice_memoria_instrucciones(0),
	m_pid(0)
{
	// Cargar las instrucciones de los archivos a memoria
	for (const auto& nombre_archivo : nombre_archivos) {
		QFile archivo (nombre_archivo);

		if (archivo.open (QIODevice::ReadOnly | QIODevice::Text) ) {
			QTextStream in (&archivo);
			int inicio_hilo = m_indice_memoria_instrucciones;

			// Lee las instrucciones y las coloca en la memoria
			while ( !in.atEnd() ) {
				QString instruccion = in.readLine();
				QStringList partes_de_instruccion = instruccion.split(' ');

				for (const auto& parte_de_instruccion : partes_de_instruccion) {
					m_memoria_instrucciones[m_indice_memoria_instrucciones] = parte_de_instruccion.toInt();
					++m_indice_memoria_instrucciones;
				}
			}

			// Crea el proceso para enviarlo a la cola
			Proceso* proc = new Proceso();
			proc->pid = m_pid++;
			proc->registros[PC] = inicio_hilo;
			m_cola_procesos.enqueue(*proc);
		}
	}
}

Procesador::~Procesador() {
	delete[] m_memoria_instrucciones;
}

bool Procesador::colaVacia() {
	QMutexLocker locker(&mutex_cola_procesos);
	return m_cola_procesos.empty();
}

Proceso Procesador::tomarProceso() {
	QMutexLocker locker(&mutex_cola_procesos);
	return m_cola_procesos.dequeue();
}

int Procesador::obtenerQuatum() const {
	// Mutex?
	return m_quantum;
}

void Procesador::imprimirMemoria() {
	for (int i = 0; i < 30; ++i) {
		qDebug() << m_memoria_instrucciones[i];
    }
}

void Procesador::encolarProceso(Proceso aEncolar)
{
    m_cola_procesos.enqueue(aEncolar);
}
