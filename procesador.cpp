#include "procesador.h"

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
    QObject(parent),
    m_quantum(quantum),
	m_duracion_transferencia_memoria_a_cache_instrucciones(4 * (2 * trasferencia + latencia_de_memoria)),
	m_memoria_instrucciones (new int[NUMERO_BYTES_MEMORIA_INSTRUCCIONES]),
	m_memoria_datos (new int[NUMERO_BYTES_MEMORIA_DATOS]),
	m_numero_de_nucleos(NUMERO_NUCLEOS),
	m_reloj(0),
	m_cuenta(m_numero_de_nucleos),
	m_indice_memoria_instrucciones(0),
    m_cache_datos(new Cache[m_numero_de_nucleos]),
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

    // Se inicializa la memoria de datos en '1', esto para operaciones relacionadas con LL y SC.
    for (int i = 0; i < NUMERO_BYTES_MEMORIA_DATOS; ++i) {
        m_memoria_datos[i] = 1;
    }
}

Procesador::~Procesador() {
	delete[] m_memoria_instrucciones;
	delete[] m_memoria_datos;
    delete[] m_cache_datos;
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
		qDebug() << "El valor del reloj es de: " << m_reloj;
		m_cuenta = m_numero_de_nucleos;
		m_condicion.wakeAll();
	}
	m_mutex_barrera.unlock();
    QThread::usleep(100);
}

void Procesador::fin_nucleo(int numero_nucleo) {
	QMutexLocker locker(&m_mutex_numero_de_nucleos);
	// Se resta del conteo de la barrera
	--m_cuenta;
	// Resta la cantidad de núcleos activos
	--m_numero_de_nucleos;

    // @todo administrar la consistencia de la chaché que sale de ejecucción.

	// Despierta a el otro núcleo si este se encontraba bloqueado
    m_condicion.wakeAll();
}

int Procesador::obtener_bloque(int numero_bloque, int numero_nucleo)
{
    ESTADO etiqueta;
    int indice = numero_bloque % NUMERO_BLOQUES_CACHE;
    //Caso 0: está en la caché local
    if (numero_bloque == m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice]) {
        etiqueta = m_cache_datos[numero_nucleo].estado_del_bloque[indice];
        if (etiqueta == ESTADO::INVALIDO) {
            // @todo: ver si está en la otra caché: compartido -> lo trae : modificado/inválido ir a memoria
        }
        else {
            // @todo: ver si mi caché está desbloqueada para devolver el bloque
        }

    }

    return 0;
}
