#include "procesador.h"

Procesador::Procesador (const QStringList& file_names, int latencia_de_memoria, int quantum, QObject* parent) :
	QObject (parent),
	m_latencia_de_memoria (latencia_de_memoria),
	m_quantum (quantum),
	m_memoria_instrucciones (new int[MEMORIA_INSTRUCCIONES]),
	m_cola_procesos(QQueue<Proceso>()),
	m_indice_memoria_instrucciones(0),
	m_pid(0)
{
	// Cargar las instrucciones de los archivos a memoria
	for (const auto& file_name : file_names) {
		QFile file (file_name);

		if (file.open (QIODevice::ReadOnly | QIODevice::Text) ) {
			QTextStream in (&file);
			int inicio_hilo = m_indice_memoria_instrucciones;
			// Lee las instrucciones y las pone en la memoria
			while ( !in.atEnd() ) {
				QString instruccion = in.readLine();
				// Toma cada dos números, los convierte a enteros y los coloca en la memoria ram
				// (las instrucciones son de 4 bytes, osea el ciclo se hace 4 veces)
				for (int i = 0; i < 6; i += 2) {
					m_memoria_instrucciones[m_indice_memoria_instrucciones] = instruccion.mid(i, 2).toInt();
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

Procesador::~Procesador()
{
	delete[] m_memoria_instrucciones;
}
