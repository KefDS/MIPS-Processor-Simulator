#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include "nucleo.h"
#include <QDebug>
#include <QObject>
#include <QString>
#include <QThread>
#include "procesador.h"

class Controlador : public QObject {
	Q_OBJECT

	public:
		Controlador (const QStringList& informacion, QObject* parent = 0);

		~Controlador();

		void comenzar_simulacion();

	public slots:
		void imprimir_estado(QString estado);

	signals:
		void empezar_simulacion();

	private:
		QThread m_thread_procesador;
		QThread m_thread_nucleo_1;
		QThread m_thread_nucleo_2;

		// Temporal
		QThread m_thread_reloj;
};
#endif // CONTROLADOR_H
