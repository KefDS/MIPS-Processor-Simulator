#ifndef NUCLEO_H
#define NUCLEO_H

#include "definiciones.h"
#include "procesador.h"
#include <QObject>
#include <QString>

/**
 * @brief Esta clase representa un núcleo de un microporcesador MIPS.
 *
 * @author	Kevin Delgado Sandí	(kefdelgado@gmail.com).
 * @author	Jean Carlo Zuñiga	(jeanczm@gmail.com).
 */
class Nucleo : public QObject {
	Q_OBJECT

public:
	Nucleo (Procesador& procesador, int numero_nucleo, QObject* parent = 0);
	~Nucleo();

signals:
	void reportar_estado(const QString& estado);

public slots:
	void run();

private:

	// Métodos privados

	/**
	* @brief cargar_contexto
	* Toma el contexto de un proceso de la cola y coloca sus valores en los registros del núcleo.
	* @param contexto de un proceso al que se quiere ejecutar en el núcleo.
	*/
	void cargar_contexto(const Proceso& proceso);

	/**
	 * @brief guardar_contexto
	 * Toma los registros del núcleo y lo guarda en un proceso
	 * para que sea puesto en la cola nuevamente.
	 * @param proceso que guardará el contexto del hilo
	 */
	void guardar_contexto(Proceso& proceso) const;

	/**
	 * @brief ejecutar_instruccion
	 * Toma la instruccion y la ejecuta. Modifica PC para que apunte a la nueva dirección.
	 * @param instruccion a ser ejecutada.
	 * @return @c true si es la instruccion @c FIN, @c false en caso contrario.
	 */
	bool ejecutar_instruccion(const Instruccion& instruccion);

	/**
	 * @brief obtiene_instruccion
	 * Con el PC se pide la instrucción que se necesita.
	 * Si esta en caché, le devuelve el dato al núcleo
	 * Sino debe ir a memoria, copiar el bloque deseado en caché
	 * y devolverle el dato al núcleo.
	 * @return instruccion pedida por el PC
	 */
	Instruccion obtiene_instruccion();


    // LOAD
    int obtener_dato(int direccion_fisica, int numero_nucleo);

    // STORE
    void guardar_dato_a_memoria(int direccion_fisica, int dato);


	// Miembros de la clase

	int m_numero_nucleo;
	Procesador& m_procesador; // Cada núcleo tendrá su propio apuntador a procesador
	Cache* const m_cache_instrucciones; /**< Representa la cache de instrucciones que posee el núcleo */
	int* const m_registros;
	int m_quantum_de_proceso_actual;
};
#endif // NUCLEO_H
