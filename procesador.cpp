#include "procesador.h"

bool Procesador::m_bandera_impresion = false;

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
	QObject(parent),
	m_quantum(quantum),
	m_duracion_transferencia_memoria_a_cache(4 * (2 * trasferencia + latencia_de_memoria)),
	m_memoria_instrucciones (new int[NUMERO_BYTES_MEMORIA_INSTRUCCIONES]),
	m_memoria_datos (new int[NUMERO_BYTES_MEMORIA_DATOS]),
	m_numero_de_nucleos(NUMERO_NUCLEOS),
	m_reloj(0),
	m_cuenta(m_numero_de_nucleos),
	m_cache_datos(new Cache[m_numero_de_nucleos]),
	m_indice_memoria_instrucciones(0),
	m_pid(0),
	m_bloques_RL(new int[NUMERO_NUCLEOS]),
	m_bloques_RL_siguiente_ciclo_reloj(new int[NUMERO_NUCLEOS]),
	m_bandera (new bool[NUMERO_NUCLEOS]),
	m_registros_RL(new int[NUMERO_NUCLEOS]),
	m_registros_RL_siguiente_ciclo_reloj(new int[NUMERO_NUCLEOS])
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

	// Se inicializa el arreglo compartido para LL-SC en -1
	for(int i = 0; i < NUMERO_NUCLEOS; ++i) {
		m_bloques_RL[i] = -1;
		m_bloques_RL_siguiente_ciclo_reloj[i] = -1;
		m_bandera[i] = false;
		m_registros_RL[i] = -1;
		m_registros_RL_siguiente_ciclo_reloj[i] = -2;
	}
}

Procesador::~Procesador() {
	delete[] m_memoria_instrucciones;
	delete[] m_memoria_datos;
	delete[] m_cache_datos;
	delete[] m_bloques_RL;
	delete[] m_bandera;
	delete[] m_registros_RL;
	delete[] m_registros_RL_siguiente_ciclo_reloj;
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

BloqueInstruccion Procesador::obtener_bloque_instrucciones(int numero_bloque) const {
	// Interpreta la memoria principal como un vector de bloques
	BloqueInstruccion* tmp = reinterpret_cast<BloqueInstruccion*>(m_memoria_instrucciones);

	return tmp[numero_bloque];
}

BloqueDato Procesador::obtener_bloque_datos(int numero_bloque) const {
	// Interpreta la memoria principal como un vector de bloques
	BloqueDato* tmp = reinterpret_cast<BloqueDato*>(m_memoria_datos);

	return tmp[numero_bloque - NUMERO_BLOQUES_INSTRUCCIONES];
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
		emit aumenta_reloj(QString("%1").arg(m_reloj));
		// Actualizar estados
		actualizar_estados_cache_datos();
		actualizar_registros_RL();
		actualizar_bloques_RL();
		m_cuenta = m_numero_de_nucleos;
		m_condicion.wakeAll();
	}
	m_mutex_barrera.unlock();
}

void Procesador::fin_nucleo() {
	QMutexLocker locker(&m_mutex_barrera);
	// Se resta del conteo de la barrera
	--m_cuenta;
	// Resta la cantidad de núcleos activos
	--m_numero_de_nucleos;

	// Se hace para que imprima la memoria una vez
	if (Procesador::m_bandera_impresion) {
		imprimir_memoria_y_cache();
	}
	else {
		Procesador::m_bandera_impresion = true;
	}

	// Despierta a el otro núcleo si este se encontraba bloqueado
	m_condicion.wakeAll();
}

int Procesador::realiza_operacion_cache_datos(int direccion_fisica, int numero_nucleo, bool es_store, int dato) {
	int resultado;
	int numero_bloque = direccion_fisica / 16;
	int numero_palabra = (direccion_fisica % 16)/ NUMERO_BYTES_PALABRA;
	int indice = numero_bloque % NUMERO_BLOQUES_CACHE;

	// Bloqueo mi propia caché
	while (!m_cache_datos[numero_nucleo].mutex.tryLock()) {
		aumentar_reloj();
	}

	// Caso 0: El bloque está en la caché local. (Si el bloque está como inválido se asume que "no está").
	if (numero_bloque == m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice] &&
		m_cache_datos[numero_nucleo].estado_del_bloque[indice] != ESTADO::INVALIDO)
	{
		if (es_store) {
			// Si el bloque en mi caché local está modificado, entonces no invalido a los bloques en las cachés foráneas.
			if (m_cache_datos[numero_nucleo].estado_del_bloque[indice] == ESTADO::COMPARTIDO) {
				// Invalidar el bloque (si está) en las demás cachés
				obtener_bus_cache_datos(numero_nucleo);
				// Mientras no pude tomar el bus, me pudieron inválidar el bloque, por lo que si esto ocurre debo llamar a la función de nuevo
				if (m_cache_datos[numero_nucleo].estado_del_bloque[indice] == ESTADO::INVALIDO) {
					liberar_bus_cache_datos();
					return realiza_operacion_cache_datos(direccion_fisica, numero_nucleo, true, dato);
				}

				for(int indice_cache = 0; indice_cache < NUMERO_NUCLEOS; ++indice_cache) {
					if(indice_cache != numero_nucleo) {
						obtener_cache_foranea(indice_cache);
						if (numero_bloque == m_cache_datos[indice_cache].identificador_de_bloque_memoria[indice]) {
							m_cache_datos[indice_cache].estado_del_bloque_siguiente_ciclo_reloj[indice_cache] = ESTADO::INVALIDO;

							// Si en la otra caché está activo el LL, inválido el registro RL
							if(obtener_bandera(indice_cache)) {
								//Si el número de bloque en la caché foránea tiene candado LL, se inválida el registro RL
								if(obtener_bloque_candado_RL(indice_cache) == numero_bloque) {
									guardar_registro_RL(indice_cache, -1);
								}
							}
						}
						liberar_cache_foranea(indice_cache);
					}
				}
				liberar_bus_cache_datos();
			}

			// Guarda el dato y cambia el estado
			m_cache_datos[numero_nucleo].bloque_dato[indice].palabra[numero_palabra] = dato;
			m_cache_datos[numero_nucleo].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::MODIFICADO;
			resultado = -1;
		}
		else {
			resultado = m_cache_datos[numero_nucleo].bloque_dato[indice].palabra[numero_palabra];
		}
	}
	else {
		// Caso 1: El bloque que ando buscando se encuentra en alguna de las otras cachés o debo traerla de memoria principal.
		obtener_bus_cache_datos(numero_nucleo);
		// Reviso en la demás cachés
		for(int indice_cache = 0; indice_cache < NUMERO_NUCLEOS; ++indice_cache) {
			if(indice_cache != numero_nucleo) {
				obtener_cache_foranea(indice_cache);

				// Pregunta si el bloque está en la otra caché.
				// Si el estado del bloque de la caché foránea está modificada, se guarda en memoria y se le cambia el estado
				// Cuando está compartida y lo que deseo hacer es un store, entonces se debe invalidar el bloque.
				if(numero_bloque == m_cache_datos[indice_cache].identificador_de_bloque_memoria[indice]) {

					switch (m_cache_datos[indice_cache].estado_del_bloque[indice]) {
						case ESTADO::COMPARTIDO:
							if(es_store) {
								m_cache_datos[indice_cache].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::INVALIDO;
							}
							break;

						case ESTADO::MODIFICADO:
							guardar_bloque_en_memoria_datos(numero_bloque, m_cache_datos[indice_cache].bloque_dato[indice]);

							if(es_store) {
								m_cache_datos[indice_cache].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::INVALIDO;
							}
							else {
								m_cache_datos[indice_cache].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::COMPARTIDO;
							}
							break;
					}
				}
				liberar_cache_foranea(indice_cache);
			}
		}

		// Guardar en memoria si se le cae encima a un bloque que estaba ahí con estado modificado.
		if(m_cache_datos[numero_nucleo].estado_del_bloque[indice] == ESTADO::MODIFICADO) {
			guardar_bloque_en_memoria_datos(m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice], m_cache_datos[numero_nucleo].bloque_dato[indice]);
		}

		// Trae el bloque desde la memoria de datos
		m_cache_datos[numero_nucleo].bloque_dato[indice] = obtener_bloque_datos(numero_bloque);
		m_cache_datos[numero_nucleo].identificador_de_bloque_memoria[indice] = numero_bloque;

		if(es_store) {
			m_cache_datos[numero_nucleo].bloque_dato[indice].palabra[numero_palabra] = dato;
			m_cache_datos[numero_nucleo].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::MODIFICADO;
			resultado = -1;
		}
		else {
			resultado = m_cache_datos[numero_nucleo].bloque_dato[indice].palabra[numero_palabra];
			m_cache_datos[numero_nucleo].estado_del_bloque_siguiente_ciclo_reloj[indice] = ESTADO::COMPARTIDO;
		}

		// Libera el bus de datos
		liberar_bus_cache_datos();
	}

	m_cache_datos[numero_nucleo].mutex.unlock();
	return resultado;
}

void Procesador::guardar_bloque_en_memoria_datos(int numero_bloque, const BloqueDato& bloque_a_guardar) {
	for (int i = 0; i < obtener_duracion_transferencia_memoria_a_cache(); ++i) {
		aumentar_reloj();
	}
	BloqueDato* tmp = reinterpret_cast<BloqueDato*>(m_memoria_datos);
	tmp[numero_bloque - NUMERO_BLOQUES_INSTRUCCIONES] = bloque_a_guardar;
	aumentar_reloj();
}

void Procesador::actualizar_bloques_RL() {
	for(int i = 0; i < NUMERO_NUCLEOS; ++i) {
		m_bloques_RL[i] = m_bloques_RL_siguiente_ciclo_reloj[i];
	}
}

void Procesador::actualizar_registros_RL() {
	for (int i = 0; i < NUMERO_NUCLEOS; ++i) {
		if (m_registros_RL_siguiente_ciclo_reloj[i] != -2) {
			m_registros_RL[i] = m_registros_RL_siguiente_ciclo_reloj[i];
			m_registros_RL_siguiente_ciclo_reloj[i] = -2;
		}
	}
}

void Procesador::imprimir_memoria_y_cache() {
	QString resumen (QString("Termina la ejecucción del programa\n==Memoria==\n"));
	for (int i = 0; i < NUMERO_BYTES_MEMORIA_DATOS; ++i) {
		resumen.append(QString("[%1] : %2 ").arg(i).arg(m_memoria_datos[i]));
	}

	emit reportar_estado(resumen);

	//	resumen.append(QString("==Caché==\n"));
	//	for (int i = 0; i < NUMERO_NUCLEOS; ++i) {
	//		resumen.append(QString("Caché %1").arg(i));
	//		for (int j = 0; j < )
	//	}
}

int Procesador::obtener_bloque_candado_RL(int numero_nucleo) const {
	return m_bloques_RL[numero_nucleo];
}

void Procesador::guardar_bloque_candado_RL(int numero_nucleo, int numero_bloque) {
	m_bloques_RL_siguiente_ciclo_reloj[numero_nucleo] = numero_bloque;
}

void Procesador::obtener_bus_cache_datos(int numero_nucleo) {
	bool primera_vez = true;

	if(!m_mutex_bus_cache_datos.tryLock()) {
		while(!m_mutex_bus_cache_datos.tryLock()) {
			if (primera_vez) {
				m_cache_datos[numero_nucleo].mutex.unlock();
				primera_vez = false;
			}
			aumentar_reloj();
		}
		m_cache_datos[numero_nucleo].mutex.lock();
	}
	// Tengo el bus, debo esperar un ciclo de reloj
	aumentar_reloj();
}

void Procesador::liberar_bus_cache_datos() {
	m_mutex_bus_cache_datos.unlock();
}

void Procesador::obtener_cache_foranea(int indice_cache) {
	while (!m_cache_datos[indice_cache].mutex.tryLock()) {
		aumentar_reloj();
	}
	aumentar_reloj();
}

void Procesador::liberar_cache_foranea(int indice_cache) {
	m_cache_datos[indice_cache].mutex.unlock();
}

void Procesador::actualizar_estados_cache_datos() {
	for (int i = 0; i < NUMERO_NUCLEOS; ++i) {
		for (int j = 0; j < NUMERO_BLOQUES_CACHE; ++j) {
			if (m_cache_datos[i].estado_del_bloque_siguiente_ciclo_reloj[j] != ESTADO::SIN_CAMBIO) {
				m_cache_datos[i].estado_del_bloque[j] = m_cache_datos[i].estado_del_bloque_siguiente_ciclo_reloj[j];
				m_cache_datos[i].estado_del_bloque_siguiente_ciclo_reloj[j] = ESTADO::SIN_CAMBIO;
			}
		}
	}
}

int Procesador::obtener_registro_RL(int numero_nucleo) const {
	return m_registros_RL[numero_nucleo];
}

void Procesador::guardar_registro_RL(int numero_nucleo, int valor) {
	m_registros_RL_siguiente_ciclo_reloj[numero_nucleo] = valor;
}

bool Procesador::obtener_bandera(int numero_nucleo) const {
	return m_bandera[numero_nucleo];
}

void Procesador::guardar_bandera(int numero_nucleo, bool valor) {
	m_bandera[numero_nucleo] = valor;
}

