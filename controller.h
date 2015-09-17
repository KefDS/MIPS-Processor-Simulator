#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QThread>
#include "nucleo.h"
#include "procesador.h"

class Controller : public QObject {
	Q_OBJECT

  public:
	explicit Controller (const QStringList& information, QObject* parent = 0);

  signals:

  public slots:

  private:
	Procesador* m_procesador;
	Nucleo* m_nucleo_1;
	Nucleo* m_nucleo_2;

	QThread t_nucleo_1;
	QThread t_nucleo_2;
	QThread t_procesador;
};

#endif // CONTROLLER_H
