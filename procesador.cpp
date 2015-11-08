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

int Procesador::cache_remota(int cache_local)
{
    if(cache_local == 1) return 0;
    else return 1;
}

void Procesador::liberar_bus_de_memoria_datos()
{
    //@ todo: implementar esta lògica para liberar el recurso compartido: bus
}

int Procesador::obtener_bloque(int numero_bloque, int numero_nucleo)
{
    ESTADO etiqueta_local, etiqueta_remota;
    int indice = numero_bloque % NUMERO_BLOQUES_CACHE;

    //Caso 0: el bloque está en la caché local.
    if (numero_bloque == m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice]) {

        /*
        Para hacer la línea que sigue yo debo tener un lock para la caché local, pues sino deberìa volver a preguntar cuando tengo
        el bus pues la otra caché pudo haberla validado inclusive.
        */
        // @todo: obtener la cachè local.
        etiqueta_local = m_cache_datos[numero_nucleo].estado_del_bloque[indice];

        if (etiqueta_local == ESTADO::INVALIDO) {

            // @todo: solicitar el bus.

            // Se pregunta si el bloque está en la otra caché:
            if(m_cache_datos[cache_remota(numero_nucleo)].identificador_de_bloque_memoria[indice] == numero_bloque){

                // @todo: solicitar la cache remota
                etiqueta_remota = m_cache_datos[cache_remota(numero_nucleo)].estado_del_bloque[indice];

                switch (etiqueta_remota) {
                    case ESTADO::COMPARTIDO:
                        m_cache_datos[numero_nucleo].bloques[indice] = m_cache_datos[cache_remota(numero_nucleo)].bloques[indice];
                        m_cache_datos[numero_nucleo].estado_del_bloque[indice] = ESTADO::COMPARTIDO;
                        // @todo: liberar ambos recursos
                        liberar_bus_de_memoria_datos();
                        break;

                        //Cason por defecto es para INVALIDO y MODIFICADO
                    default:
                          // @todo: fallo cache datos: pondrìa el bloque en la cache remota con la etiqueta "Compartido"
                         m_cache_datos[numero_nucleo].bloques[indice] = m_cache_datos[cache_remota(numero_nucleo)].bloques[indice];
                         m_cache_datos[numero_nucleo].estado_del_bloque[indice] = ESTADO::COMPARTIDO;
                         // @todo: liberar ambos recursos
                         liberar_bus_de_memoria_datos();
                }
            }
            else
            {
                // @todo: fallo de cachè pero solo para la cachè local
            }
        }
        else
        {
            // @todo: ver si mi caché está desbloqueada para devolver el bloque
        }

    }
    else
    {
           //CASO 1: el bloque no está en la caché local pero sì està en la caché remota
           // Se pregunta si el bloque está en la otra caché:
           if(m_cache_datos[cache_remota(numero_nucleo)].identificador_de_bloque_memoria[indice] == numero_bloque){
                  //implementar los casos y demàs
           }
           else {
               //CASO 2, el bloque no está en ninguna caché, hay que traerlo de memoria principal.
               // @todo: fallo de cachè pero solo para la cachè local
           }
    }

    return 0;
}
