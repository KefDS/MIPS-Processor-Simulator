#include "procesador.h"

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
	QObject (parent),
	m_latencia_de_memoria (latencia_de_memoria),
	m_quantum (quantum),
	m_trasferencia_memoria_cache(trasferencia),
    m_memoria_instrucciones (new Bloque[NUMERO_BLOQUES_INSTRUCCIONES]),
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
			// Lee las instrucciones y las pone en la memoria
			while ( !in.atEnd() ) {
                QString instruccion = in.readLine();
                QStringList partes_de_instruccion = instruccion.split(' ');

                int i = 0;
                for (const auto& parte_de_instruccion : partes_de_instruccion)
                {
                    int numero_de_bloque = m_indice_memoria_instrucciones / 16;
                    int numero_de_pagina = (m_indice_memoria_instrucciones % 16) / 4;

                    m_memoria_instrucciones[numero_de_bloque].palabra[numero_de_pagina].celda[i] = parte_de_instruccion.toInt();
                    ++m_indice_memoria_instrucciones;
                    ++i;
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

void Procesador::imprimirMemoria() {
    for (int i = 0; i < NUMERO_BLOQUES_INSTRUCCIONES; ++i) {
        m_memoria_instrucciones[i].print();
    }
}
