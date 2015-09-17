#ifndef PROCESADOR_H
#define PROCESADOR_H

#define MEMORIA_INSTRUCCIONES 640
#define MEMORIA_DATOS 1408
#define NUMERO_REGISTROS 34 // 32 registros + PC + IR
#define PC 32
#define IR 33

#include <QFile>
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
	// Constructor
	explicit Procesador (const QStringList& file_names, int latencia_de_memoria, int quantum, QObject* parent = 0);

	// Destructor
	virtual ~Procesador();

  signals:

  public slots:

  private:
	const int m_latencia_de_memoria;
	const int m_quantum;

	// Memoria donde se encontrarán las instrucciones de los hilos
	int* const m_memoria_instrucciones;

	// NOTA: Temporal miemtras se decide cual es la mejor forma
	// NOTA: Reloj debería ir en memoria dinámica?
	int m_reloj;

	// Cola que contrendrá a los hilos
	QQueue<Proceso> m_cola_procesos;


	// Variables auxiliares

	// Esta variable tendrá la posición libre de la memoria de instrucciones
	int m_indice_memoria_instrucciones;
	// Tiene los pid de los procesos
	int m_pid;
};

#endif // PROCESADOR_H
