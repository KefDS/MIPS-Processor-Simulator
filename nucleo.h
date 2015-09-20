#ifndef NUCLEO_H
#define NUCLEO_H

#define CACHE 128

#include "procesador.h"
#include <QObject>
#include <QString>

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
		QString nombre;

		// Cada núcleo tendrá su propio apuntador a procesador
		Procesador& m_procesador;

		int* const m_cache_instrucciones;

		int const* m_registros;

		int m_quantum_de_proceso_actual;
};
#endif // NUCLEO_H
