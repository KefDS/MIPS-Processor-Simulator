#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include "controlador.h"

int main (int argc, char* argv[]) {
	QCoreApplication app (argc, argv);

	if (argc > 0) {
		QFile archivo_informacion (argv[1]);

		if (archivo_informacion.open (QIODevice::ReadOnly | QIODevice::Text) ) {
			QStringList informacion;
			QTextStream in (&archivo_informacion);

			while (!in.atEnd() ) {
				informacion << in.readLine();
			}

			Controlador controlador (informacion);
		}
		else {
			qDebug() << "Lo siento, el archivo no pudo ser abierto";
		}
	}
	else {
		qDebug() << "No se envió el archivo con las direcciones de archivos de los hilos";
	}

	return 	app.exec();
}
