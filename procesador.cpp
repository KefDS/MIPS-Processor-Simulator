#include "procesador.h"

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
	QObject(parent),
	m_quantum(quantum),
	m_duracion_transferencia_memoria_a_cache(4 * (2 * trasferencia + latencia_de_memoria)),
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

int Procesador::obtener_duracion_transferencia_memoria_a_cache() const {
	return m_duracion_transferencia_memoria_a_cache;
}

Bloque Procesador::obtener_bloque_instrucciones(int numero_bloque) const {
	// Interpreta la memoria principal como un vector de bloques
	Bloque* tmp = reinterpret_cast<Bloque*>(m_memoria_instrucciones);

	return tmp[numero_bloque];
}

Bloque Procesador::obtener_bloque_datos(int numero_bloque) const {
	// Interpreta la memoria principal como un vector de bloques
	Bloque* tmp = reinterpret_cast<Bloque*>(m_memoria_datos);

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

int Procesador::obtener_bloque_cache_datos(int direccion_fisica, int numero_nucleo, bool store, int dato) {
	// Notas acerca de cuando cambiar reloj:
	// Cambio de reloj
	// Cuando tomo bus cambio ciclo tick
	// Cuando tomo la otra cache tick

    int numero_bloque = direccion_fisica / 16;

    int palabra_bloque = (numero_bloque % 16)/ NUMERO_BYTES_PALABRA;

	int indice = numero_bloque % NUMERO_BLOQUES_CACHE;



	// Bloqueo mi propia caché
	while (!m_cache_datos[numero_nucleo].mutex.tryLock()) {
		aumentar_reloj();
	}

    // Caso 0: El bloque está en la caché local. (Si el bloque está como inválido se asume que "no está").
	if (numero_bloque == m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice] &&
		m_cache_datos[numero_nucleo].estado_del_bloque[indice] != ESTADO::INVALIDO) {

		// @todo devolver resultado
		// DUDA: ¿En este método se deberia hacer absolutamente todo? (load, regresar el dato)
	}
	else {
		// Caso 1: El bloque que ando buscando se encuentra en alguna de las otras cachés.

		// Tomo el bus de datos
		while (!m_mutex_bus_cache_datos.tryLock()) {
			aumentar_reloj();
		}

		// DUDA: Situación: tengo mi caché local bloqueada y estoy tratando de tomar el bus.
		// Si el bus lo tiene la otra caché y ocupa bloquear mi caché, pasa un deadlock
		// ¿Cómo soluciono esto?

        while(!m_mutex_bus_cache_datos.tryLock())
        {
            m_cache_datos[numero_nucleo].mutex.unlock();
            aumentar_reloj();
        }
        m_cache_datos[numero_nucleo].mutex.lock();


		// Tengo el bus, debo esperar un ciclo de reloj
		aumentar_reloj();

		// Reviso en la demás cachés
		for(int indice_cache = 0; indice_cache < NUMERO_NUCLEOS; ++indice_cache) {
			if(indice_cache != numero_nucleo) {

				// Candado a la caché foránea
				while (!m_cache_datos[indice_cache].mutex.tryLock()) {
					aumentar_reloj();
				}

				// Pregunta si el bloque está en la otra caché.
                // Si el estado del bloque de la caché foránea está modificada, se guarda en memoria y se le cambia el estado
				// Cuando está compartida o inválida no se hace nada con ella.
				if(m_cache_datos[indice_cache].identificador_de_bloque_memoria[indice] == numero_bloque &&
				   m_cache_datos[indice_cache].estado_del_bloque[indice] == ESTADO::MODIFICADO) {

					//Procedo como si fuera un fallo de caché
					for (int i = 0; i < obtener_duracion_transferencia_memoria_a_cache(); ++i) {
						aumentar_reloj();
					}
					guardar_bloque_en_memoria_datos(m_cache_datos[indice_cache].bloques[indice]);

                    if(store){
                        m_cache_datos[indice_cache].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::INVALIDO;
                    }
                    else {
                        m_cache_datos[indice_cache].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::COMPARTIDO;
                    }
				}

				m_cache_datos[indice_cache].mutex.unlock();
			}
		}

		// Trae el bloque desde la memoria de datos

		// @todo Guardar en memoria si se le cae encima a un bloque que estaba ahí.

		m_cache_datos[numero_nucleo].bloques[indice] = obtener_bloque_datos(numero_bloque);

        if(store){
            m_cache_datos[numero_nucleo].bloque_dato[numero_bloque].palabra[palabra_bloque].dato = dato;
            m_cache_datos[numero_nucleo].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::MODIFICADO;
        }
        else {
            m_cache_datos[numero_nucleo].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::COMPARTIDO;
        }

		m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice] = numero_bloque;

		// Suelta el bus de datos
		m_mutex_bus_cache_datos.unlock();
	}

	m_cache_datos[numero_bloque].mutex.unlock();
	return 0;
}

void Procesador::guardar_bloque_en_memoria_datos(const Bloque& bloque_a_guardar)
{

}
