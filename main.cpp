#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QStringList>
#include "controller.h"

int main (int argc, char* argv[]) {
	QCoreApplication app (argc, argv);

	if (argc > 0) {
		QFile file (argv[1]);

		if (file.open (QIODevice::ReadOnly | QIODevice::Text) ) {
			QStringList files_name;
			QTextStream in (&file);

			while (!in.atEnd() ) {
				files_name << in.readLine();
			}

			Controller controlador (files_name);
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
