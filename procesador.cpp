#include "procesador.h"
#include <QDebug>
#include <QThread>

Procesador::Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent) :
	QObject (parent),
	m_latencia_de_memoria (latencia_de_memoria),
	m_quantum (quantum),
	m_trasferencia_memoria_cache(trasferencia),
	m_memoria_instrucciones (new int[MEMORIA_INSTRUCCIONES]),
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
                QString instruccion =in.readLine();
                QStringList instrucciones;
                instrucciones = instruccion.split(' ');

                for (const auto& i : instrucciones)
                {
                    m_memoria_instrucciones[m_indice_memoria_instrucciones] = i.toInt();
                    ++m_indice_memoria_instrucciones;
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

void Procesador::run() {
    qDebug() << "corriendo procesador" << QThread::currentThreadId();
    for (int i = 0; i < 20; i++) {
        qDebug() << "instrucciones " << m_memoria_instrucciones[i];
    }
}
