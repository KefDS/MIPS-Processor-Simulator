#ifndef NUCLEO_H
#define NUCLEO_H

#define NUMERO_BLOQUES_CACHE 8

#include "procesador.h"
#include <QObject>
#include <QString>

struct Cache {
    Bloque bloques[NUMERO_BLOQUES_CACHE];
};

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
        void cargar_contexto(const Proceso &proceso);
        void ejecutar_instruccion();

        int *obtieneInstruccion(int);
		QString nombre;

		// Cada núcleo tendrá su propio apuntador a procesador
		Procesador& m_procesador;

        Cache* m_cache_instrucciones;

		int const* m_registros;

		int m_quantum_de_proceso_actual;
};
#endif // NUCLEO_H
