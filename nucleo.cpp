#include "nucleo.h"

Nucleo::Nucleo (Procesador& procesador, int numero_nucleo, QObject* parent) :
	QObject (parent),
	m_numero_nucleo(numero_nucleo),
	m_procesador (procesador),
	m_cache_instrucciones(new Cache()),
	m_registros(new int[NUMERO_REGISTROS])
{

}

Nucleo::~Nucleo() {
	delete[] m_registros;
	delete m_cache_instrucciones;
}

void Nucleo::run() {
	emit reportar_estado (QString("Núcleo %1 inició su ejecución.").arg(m_numero_nucleo));

	// Mientras hayan procesos en la cola
	while(!m_procesador.cola_vacia()) {
		Proceso proceso_actual = m_procesador.tomar_proceso();
		cargar_contexto(proceso_actual);
		emit reportar_estado(QString("El núcleo %1 va a ejecutar el hilo con PID %2.").arg(m_numero_nucleo).arg(proceso_actual.pid));

		m_quantum_de_proceso_actual = m_procesador.obtener_quatum();

		bool termino_hilo = false;

		while (m_quantum_de_proceso_actual > 0) {
			Instruccion instruccion = obtiene_instruccion();
			termino_hilo = ejecutar_instruccion(instruccion);
			--m_quantum_de_proceso_actual;
			m_procesador.aumentar_reloj();

            // Imprimir estatus después del ciclo de reloj
            QString resumen (QString("Núcleo %1. Registros: ").arg(m_numero_nucleo));
            for (int i = 0; i < NUMERO_REGISTROS; ++i) {
                resumen.append(QString("[%1] : %2, ").arg(i).arg(m_registros[i]));
            }
            emit reportar_estado(resumen);
		}

		// Si el hilo no ha terminado, se envía a la cola de procesos de nuevo
		if (!termino_hilo) {
			guardar_contexto(proceso_actual);
			m_procesador.encolar_proceso(proceso_actual);
		}
	}

    m_procesador.fin_nucleo(m_numero_nucleo);

	emit reportar_estado (QString("Núcleo %1 terminó su ejecución.").arg(m_numero_nucleo));
}

void Nucleo::cargar_contexto(const Proceso& proceso) {
	for (int i = 0; i < NUMERO_REGISTROS; ++i) {
		m_registros[i] = proceso.registros[i];
	}
}

void Nucleo::guardar_contexto(Proceso& proceso) const {
	for (int i = 0; i < NUMERO_REGISTROS; ++i) {
		proceso.registros[i] = m_registros[i];
	}
}

bool Nucleo::ejecutar_instruccion(const Instruccion& instruccion) {
	bool es_instruccion_fin = false;

	// Mueve el PC hacia la siguiente instrucción
	m_registros[PC] += NUMERO_BYTES_PALABRA;

	// Código de operación
	switch (instruccion.celda[0]) {
		case DADDI:
			m_registros[instruccion.celda[2]] = m_registros[instruccion.celda[1]] + instruccion.celda[3];
			break;

		case DADD:
			m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] + m_registros[instruccion.celda[2]];
			break;

		case DSUB:
			m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] - m_registros[instruccion.celda[2]];
			break;

		case DMUL:
			m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] * m_registros[instruccion.celda[2]];
			break;

		case DDIV:
			m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] / m_registros[instruccion.celda[2]];
			break;

		case LW:
			// @todo LW
			break;

		case SW:
			// @todo SW
			break;

		case BEQZ:
			if(m_registros[instruccion.celda[1]] == 0) {
				m_registros[PC] += instruccion.celda[3] * NUMERO_BYTES_PALABRA;
			}
			break;

		case BNEZ:
			if(m_registros[instruccion.celda[1]] != 0) {
				m_registros[PC] += instruccion.celda[3] * NUMERO_BYTES_PALABRA;
			}
			break;

		case JAL:
			m_registros[31] = m_registros[PC];
			m_registros[PC] += instruccion.celda[3];
			break;

		case JR:
			m_registros[PC] = m_registros[instruccion.celda[1]];
			break;

		case LL:
			// @todo LL
			break;

		case SC:
			// @todo SC
			break;

		case FIN:
			es_instruccion_fin = true;
			m_quantum_de_proceso_actual = 0;
			break;

		default:
			emit reportar_estado (QString("La instrucción %1 no es válida para esta simulación, por favor presione terminar simulación.").arg(instruccion.celda[0]));
	};

	return es_instruccion_fin;
}

Instruccion Nucleo::obtiene_instruccion() {
	int numero_de_bloque = m_registros[PC] / 16;
	int numero_de_palabra = (m_registros[PC] % 16) / NUMERO_PALABRAS_BLOQUE;

	// índice donde se debería encontrar el bloque en caché si estuviera
	int indice = numero_de_bloque % NUMERO_BLOQUES_CACHE;

	// El bloque no está en caché
	if (numero_de_bloque != m_cache_instrucciones->identificador_de_bloque_memoria[indice]) {

		// Debe esperar mientras el bus no esté disponible
		while(!m_procesador.bus_de_memoria_instrucciones_libre()) {
			m_procesador.aumentar_reloj();
           emit reportar_estado(QString("Núcleo %1 está esperando a que se desocupe el bus de datos").arg(m_numero_nucleo));
		}

		// Se pide el bloque a memoria prinicipal
		m_cache_instrucciones->bloques[indice] = m_procesador.obtener_bloque(numero_de_bloque);
		m_cache_instrucciones->identificador_de_bloque_memoria[indice] = numero_de_bloque;

		// Aquí se da el retraso de tiempo en el cual se debe ir a memoria a traer un bloque.
		int tiempo_de_espera = m_procesador.obtener_duracion_transferencia_memoria_a_cache_instrucciones();
		for(int i = 0; i < tiempo_de_espera; ++i) {
			m_procesador.aumentar_reloj();
            emit reportar_estado(QString("Núcleo %1 está ocupando el bus de datos").arg(m_numero_nucleo));
		}

		m_procesador.liberar_bus_de_memoria_instrucciones();
	}

    return m_cache_instrucciones->bloques[indice].palabra[numero_de_palabra];
}

int Nucleo::obtener_dato(int direccion_fisica, int numero_nucleo)
{
    return 0;
}

void Nucleo::guardar_dato_a_memoria(int direccion_fisica, int dato)
{

}

