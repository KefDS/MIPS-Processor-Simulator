#ifndef NUCLEO_H
#define NUCLEO_H

#include "procesador.h"
#include <QObject>

class Nucleo : public QObject {
	Q_OBJECT

  public:
	// Constructor
	explicit Nucleo (Procesador& procesador, QObject* parent = 0);

	// Destructor
	virtual ~Nucleo();

  signals:

  public slots:

private:
	/* Cada núcleo tendrá su propio apuntador a procesador, para así poder realizar operaciones
	 * como: cambiar de contexto, aumentar el reloj, etc */
	Procesador& m_procesador; // Variable compartida
	int const* m_registros;
};

#endif // NUCLEO_H
