#ifndef PROCESADOR_H
#define PROCESADOR_H

#include <QDebug>

#include "definiciones.h"
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include <QTextStream>

/**
 * @brief Procesador class
 * Esta clase es una abstracón de los recursos a los cuáles los núcleos deben
 * acceder para completar su labor. Los miembros de esta clase son
 * recursos críticos, ya que ambos núcleos deben acceder a él.
 *
 * @author	Kevin Delgado Sandí	(kefdelgado@gmail.com).
 *			Jean Carlo Zuñiga	().
 */
class Procesador : public QObject {
	Q_OBJECT

public:
	Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent = 0);
	~Procesador();


	// Métodos

	/**
	* @brief colaVacia.
	* Verifica sí la cola de procesos está vacía.
	* @return true si la cola esta vacía. false en caso contrario.
	*/
	bool colaVacia();

	/**
	* @brief tomarProceso
	* Esta clase toma un proceso de la cola de procesos y quita el procesos de la cola.
	* @return Primer proceso en la cola.
	*/
	Proceso tomarProceso();

	int obtenerQuatum() const;

	// Para pruebas
	void imprimirMemoria();

    //Encola un proceso
    void encolarProceso(Proceso);
private:
	const int m_latencia_de_memoria;
	const int m_quantum;
	const int m_trasferencia_memoria_cache;

	int* const m_memoria_instrucciones; /**< Memoria donde se encontrarán las instrucciones de los hilos */

	QQueue<Proceso> m_cola_procesos; /** Cola que contrendrá a los hilos */

	// Mutex para los recursos compartidos
	QMutex mutex_memoria_instrucciones; /**< Mutex que se encarga de sincronizar la lectura y escritura de la memoria de instrucciones */
	QMutex mutex_cola_procesos;			/**< Mutex que se encarga de sincronizar la lectura y escritura de la cola de procesos */


	// Variables auxiliares

	// Esta variable tendrá la posición libre de la memoria de instrucciones
	int m_indice_memoria_instrucciones;
	// Tiene los pid de los procesos
	int m_pid;
};
#endif // PROCESADOR_H
