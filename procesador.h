#ifndef PROCESADOR_H
#define PROCESADOR_H

#define NUMERO_BLOQUES_INSTRUCCIONES 40
#define NUMERO_REGISTROS 33 // 32 registros + PC
#define PC 32

#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QTextStream>
#include <QMutexLocker>

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

struct Palabra {
        int celda[4];
};

struct Bloque {
        Palabra palabra[4];

        // Solo lo usará la chaché
        int identificador_de_bloque_memoria;

        void print() {
           for (int i = 0; i < 4; ++i) {
               qDebug() << "Palabra[" << i << "] = ";
               for (int j = 0; j < 4; ++j) {
                   qDebug() << "Celda[" << j << "] = " << palabra[i].celda[j];
               }
           }
        }
};

class Procesador : public QObject {
		Q_OBJECT

	public:
		Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent = 0);

		~Procesador();

        bool colaVacia();

        Proceso tomarProceso();

        // Para pruebas
        void imprimirMemoria();

	private:
		const int m_latencia_de_memoria;
		const int m_quantum;
		const int m_trasferencia_memoria_cache;

		// Memoria donde se encontrarán las instrucciones de los hilos
        Bloque* m_memoria_instrucciones;

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
