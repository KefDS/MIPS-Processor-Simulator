#ifndef CONTROLADOR_H
#define CONTROLADOR_H

#include "definiciones.h"
#include "nucleo.h"
#include <QObject>
#include <QString>
#include <QThread>
#include "procesador.h"

/**
 * @class Controlador
 *
 * @brief Calse controladora de la simulación.
 *
 *	Esta clase contiene los Qthreads que se encargarán de
 * hacer correr los núcleos. Además tiene procesador, el cual
 * contiene los recursos críticos que los núcleos necesitan
 * para realizar su tarea.
 *
 * @author	Kevin Delgado Sandí	(kefdelgado@gmail.com).
 *			Jean Carlo Zuñiga	(jeanczm@gmail.com).
 */
class Controlador : public QObject {
	Q_OBJECT

public:
	Controlador(const QStringList& rutas_archivos, const Datos_usuario& datos, QObject* parent = 0);
	~Controlador();

	// Métodos

	/**
	* @brief comenzar_simulacion
	* Este método comienza la ejecución de los threads.
	*/
	void comenzar_simulacion();

signals:
	void enviar_estado(const QString& estado);

private:
	QThread m_thread_nucleo_0;		/**< QThread que contendrá el núcleo 0 */
	QThread m_thread_nucleo_1;		/**< QThread que contendrá el núcleo 1 */
	Procesador* const m_procesador;	/**< Esta variable tendrá los recursos críticos que deben compartir los núcleos */
};
#endif // CONTROLADOR_H
