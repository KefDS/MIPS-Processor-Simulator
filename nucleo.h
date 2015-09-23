#ifndef NUCLEO_H
#define NUCLEO_H

#include "definiciones.h"
#include "procesador.h"
#include <QObject>
#include <QString>

/**
 * @brief Nucleo class
 * Esta clase representa un núcleo de un microporcesador MIPS.
 *
 * @author	Kevin Delgado Sandí	(kefdelgado@gmail.com).
 *			Jean Carlo Zuñiga	().
 */
class Nucleo : public QObject {
	Q_OBJECT

public:
	Nucleo (Procesador& procesador, const QString& nombre, QObject* parent = 0);
	~Nucleo();

signals:
	void reportar_estado(const QString& estado);

public slots:
	void run();

private:

	// Métodos privados

	/**
		 * @brief cargar_contexto
		 * Toma un procesos de la cola y la coloca en los registros del núcleo.
		 * @param proceso al que se quiere ejecutar en el núcleo.
		 */
	void cargar_contexto(const Proceso& proceso);
	void ejecutar_instruccion();
	int* obtieneInstruccion() const;


	// Miembros de la clase

	QString nombre;
	Procesador& m_procesador; // Cada núcleo tendrá su propio apuntador a procesador
	Cache* m_cache_instrucciones;
	int* const m_registros;
	int m_quantum_de_proceso_actual;
};
#endif // NUCLEO_H
