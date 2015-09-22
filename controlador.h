#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include "nucleo.h"
#include <QDebug>
#include <QObject>
#include <QString>
#include <QThread>
#include "procesador.h"

struct Datos_usuario {
	int latencia_de_memoria;
	int trasferencia;
	int quatum;

	Datos_usuario(int latencia_de_memoria, int trasferencia, int quatum) :
		latencia_de_memoria(latencia_de_memoria),
		trasferencia(trasferencia),
		quatum(quatum)
	{

	}
};

class Controlador : public QObject {
	Q_OBJECT

	public:
		Controlador(const QStringList& rutas_archivos, const Datos_usuario& datos, QObject* parent = 0);

		~Controlador();

		void comenzar_simulacion();

	signals:
        void enviar_estado(const QString& estado);

	private:
		QThread m_thread_nucleo_1;
		QThread m_thread_nucleo_2;
        Procesador* m_procesador;

		// Temporal
		QThread m_thread_reloj;
};
#endif // CONTROLADOR_H
