#ifndef PROCESADOR_H
#define PROCESADOR_H

#define MEMORIA_INSTRUCCIONES 640
#define MEMORIA_DATOS 1408
#define NUMERO_REGISTROS 33 // 32 registros + PC
#define PC 32

#include <QFile>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QTextStream>

// Estructura que reprensetará la cola de los hilos
struct Proceso {
		int pid;
		int registros[NUMERO_REGISTROS];

		Proceso() {
			for (int i = 0; i < NUMERO_REGISTROS; ++i) {
				registros[i] = 0;
			}
		}
};

class Procesador : public QObject {
		Q_OBJECT

	public:
		Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int quantum, QObject* parent = 0);

		~Procesador();

	public slots:
		void run();

	private:
		const int m_latencia_de_memoria;
		const int m_quantum;

		// Memoria donde se encontrarán las instrucciones de los hilos
		int* const m_memoria_instrucciones;

		// Cola que contrendrá a los hilos
		QQueue<Proceso> m_cola_procesos;

		// Mutex para los recursos compartidos
		QMutex mutex_memoria_instrucciones;
		QMutex mutex_cola_procesos;

		// Variables auxiliares

		// Esta variable tendrá la posición libre de la memoria de instrucciones
		int m_indice_memoria_instrucciones;
		// Tiene los pid de los procesos
		int m_pid;
};
#endif // PROCESADOR_H
