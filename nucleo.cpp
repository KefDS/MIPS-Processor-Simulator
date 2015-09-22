#include "nucleo.h"
#include <QThread>
#include <QDebug>

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
}

void Nucleo::run() {
    QString estado = this->nombre + " Ejecutándose";
    emit reportar_estado(estado);

    // Mientras hayan procesos en la cola
    while(this->m_procesador.colaVacia()) {
        Proceso proceso_actual = m_procesador.tomarProceso();
        cargar_contexto(&proceso);

        int* instruccion = obtieneInstruccion();
    }


}

void Nucleo::cargar_contexto(const Proceso& proceso) {
    for (int i = 0; i < NUMERO_REGISTROS; ++i) {
        m_registros[i] = proceso.registros[i];
    }
}

void Nucleo::ejecutar_instruccion() {
}

int* Nucleo::obtieneInstruccion(int) {
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

    return p;
}
