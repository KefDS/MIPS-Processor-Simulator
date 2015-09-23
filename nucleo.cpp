#include "nucleo.h"

Nucleo::Nucleo (Procesador& procesador, const QString& nombre, QObject* parent) :
	QObject (parent),
	m_nombre(nombre),
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
	QString estado = m_nombre + " Ejecutándose";
	emit reportar_estado(estado);

	// Mientras hayan procesos en la cola
	while(!m_procesador.colaVacia()) {
		Proceso proceso_actual = m_procesador.tomarProceso();
		cargar_contexto(proceso_actual);

		estado = QString(m_nombre + " va a ejecutar el hilo con PID %1").arg(proceso_actual.pid);
		emit reportar_estado(estado);

		m_quantum_de_proceso_actual = m_procesador.obtenerQuatum();

		while (m_quantum_de_proceso_actual > 0) {
			Instruccion instruccion = obtiene_instruccion();
			ejecutar_instruccion (instruccion);

			m_quantum_de_proceso_actual--;
		}

        m_procesador.encolarProceso(proceso_actual);

		break;
	}

	estado = m_nombre + " Terminó ejecución";
	emit reportar_estado (estado);
}

void Nucleo::cargar_contexto(const Proceso& proceso) {
	for (int i = 0; i < NUMERO_REGISTROS; ++i) {
		m_registros[i] = proceso.registros[i];
	}
}

void Nucleo::ejecutar_instruccion(Instruccion& instruccion) {
	// Código de operación
	switch (instruccion.celda[0]) {
		case DADDI:
            this->m_registros[instruccion.celda[2]]=this->m_registros[instruccion.celda[1]]+instruccion.celda[3];
			break;
		case DADD:
            this->m_registros[instruccion.celda[3]]=this->m_registros[instruccion.celda[1]]+this->m_registros[instruccion.celda[2]];
			break;
		case DSUB:
            this->m_registros[instruccion.celda[3]]=this->m_registros[instruccion.celda[1]]-this->m_registros[instruccion.celda[2]];
			break;
		case DMUL:
            this->m_registros[instruccion.celda[3]]=this->m_registros[instruccion.celda[1]]*this->m_registros[instruccion.celda[2]];
			break;
		case DDIV:
            this->m_registros[instruccion.celda[3]]=this->m_registros[instruccion.celda[1]]/this->m_registros[instruccion.celda[2]];
			break;
		case BEQZ:
            if(this->m_registros[instruccion.celda[1]]==0)
            {
                this->m_registros[PC] = instruccion.celda[3];
            }
			break;
		case BNEZ:
            if(this->m_registros[instruccion.celda[1]]!=0)
            {
                this->m_registros[PC] = instruccion.celda[3];
            }
			break;
		case JAL:
            this->m_registros[31] = this->m_registros[PC];
            this->m_registros[PC] = this->m_registros[PC] + instruccion.celda[3];
			break;
		case JR:
            this->m_registros[PC] = this->m_registros[instruccion.celda[1]];
			break;
		case FIN:
            m_quantum_de_proceso_actual=0;
			break;
		default:
			emit reportar_estado (QString("La instrucción %1 no es válida para esta simulación").arg(instruccion.celda[0]));
	};
}

Instruccion Nucleo::obtiene_instruccion() {
	int numero_de_bloque = m_registros[PC] / 16;
	int numero_de_palabra = (m_registros[PC] % 16) / NUMERO_PALABRAS_BLOQUE;

	// Contendrá el índice donde se debería encontrar el bloque en cahé si estuviera
	// NOTA: Preguntar si esto es mapeo directo
	int indice = numero_de_bloque % NUMERO_BLOQUES_CACHE;

	Instruccion instruccion;

	// ¿Está el bloque en caché?
	if (numero_de_bloque == m_cache_instrucciones->identificador_de_bloque_memoria[indice]) {
		instruccion = m_cache_instrucciones->bloques[indice].palabra[numero_de_palabra];
	}
	else {
		// Se pide el bloque a memoria prinicipal
	}

	return instruccion;
}
