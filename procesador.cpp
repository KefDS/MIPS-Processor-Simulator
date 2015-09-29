#include "procesador.h"

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
	QObject (parent),
	m_quantum (quantum),
	m_duracion_transferencia_memoria_a_cache_instrucciones(4 * (2 * trasferencia + latencia_de_memoria) ),
	m_memoria_instrucciones (new int[NUMERO_BYTES_MEMORIA_INSTRUCCIONES]),
	m_numero_de_nucleos(NUMERO_NUCLEOS),
	m_reloj(0),
	m_cuenta(m_numero_de_nucleos),
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
				QString instruccion = in.readLine().trimmed();
				QStringList partes_de_instruccion = instruccion.split(' ');

				for (const auto& parte_de_instruccion : partes_de_instruccion) {
					m_memoria_instrucciones[m_indice_memoria_instrucciones] = parte_de_instruccion.toInt();
					++m_indice_memoria_instrucciones;
				}
			}

			// Crea el proceso para enviarlo a la cola
			m_cola_procesos.enqueue(Proceso(m_pid++, inicio_hilo));
		}
	}
}

Procesador::~Procesador() {
	delete[] m_memoria_instrucciones;
}

bool Procesador::cola_vacia() {
	QMutexLocker locker(&m_mutex_cola_procesos);
	return m_cola_procesos.empty();
}

Proceso Procesador::tomar_proceso() {
	QMutexLocker locker(&m_mutex_cola_procesos);
	return m_cola_procesos.dequeue();
}

void Procesador::encolar_proceso(const Proceso& proceso_a_encolar) {
	QMutexLocker locker(&m_mutex_cola_procesos);
	m_cola_procesos.enqueue(proceso_a_encolar);
}

int Procesador::obtener_quatum() const {
	return m_quantum;
}

int Procesador::obtener_duracion_transferencia_memoria_a_cache_instrucciones() const {
	return m_duracion_transferencia_memoria_a_cache_instrucciones;
}

Bloque Procesador::obtener_bloque(int numero_bloque) const {
	// Interpreta la memoria principal como un vector de bloques
	Bloque* tmp = reinterpret_cast<Bloque*>(m_memoria_instrucciones);

	return tmp[numero_bloque];
}

bool Procesador::bus_de_memoria_instrucciones_libre() {
	return m_mutex_memoria_instrucciones.tryLock();
}

void Procesador::liberar_bus_de_memoria_instrucciones() {
	m_mutex_memoria_instrucciones.unlock();
}

void Procesador::aumentar_reloj() {
	m_mutex_barrera.lock();
	--m_cuenta;
	if (m_cuenta > 0) {
		m_condicion.wait(&m_mutex_barrera);
	}
	else {
		++m_reloj;
		m_cuenta = m_numero_de_nucleos;
		m_condicion.wakeAll();
		qDebug() << "Se aumentó reloj" << m_reloj;
	}
	m_mutex_barrera.unlock();
}

void Procesador::fin_nucleo() {
	QMutexLocker locker(&m_mutex_numero_de_nucleos);
	// Se resta del conteo de la barrera
	--m_cuenta;
	// Resta la cantidad de núcleos activos
	--m_numero_de_nucleos;
	// Despierta a el otro núcleo si este se encontraba bloqueado
	m_condicion.wakeAll();
}
