#include "nucleo.h"

Nucleo::Nucleo (Procesador& procesador, const QString& nombre, QObject* parent) :
	QObject (parent),
	nombre(nombre),
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
	QString estado = this->nombre + " Ejecutándose";
	emit reportar_estado(estado);

	// Mientras hayan procesos en la cola
	while(m_procesador.colaVacia()) {
		Proceso proceso_actual = m_procesador.tomarProceso();
		cargar_contexto(proceso_actual);

		// Mientras el quatum del proceso sea diferente de 0

		//int* instruccion = obtieneInstruccion();
	}

	estado = this->nombre + " Terminó ejecución";
	emit reportar_estado (estado);
}

void Nucleo::cargar_contexto(const Proceso& proceso) {
	for (int i = 0; i < NUMERO_REGISTROS; ++i) {
		m_registros[i] = proceso.registros[i];
	}

	QString estado = this->nombre + " va a ejecutar hilo " + proceso.pid;
	emit reportar_estado(estado);
}

void Nucleo::ejecutar_instruccion() {
}

int* Nucleo::obtieneInstruccion() const {
	int numero_de_bloque = m_registros[PC] / 16;
	int numero_de_palabra = (m_registros[PC] % 16) / 4;

	// Contendrá el índice donde se debería encontrar el bloque en cahé si estuviera
	int indice = numero_de_bloque % NUMERO_BLOQUES_CACHE;

	Palabra p;
	// ¿Está el bloque en caché?
	if (numero_de_bloque == m_cache_instrucciones->bloques[indice].identificador_de_bloque_memoria) {
		p = m_cache_instrucciones->bloques[indice].palabra[numero_de_palabra];
	}
	else {
		// Se pide el bloque a memoria prinicipal
	}

	//return p;
	int* MPLP;
	return MPLP;
}
