#ifndef PROCESADOR_H
#define PROCESADOR_H

#include <QDebug>

#include "definiciones.h"
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QQueue>
#include <QReadWriteLock>
#include <QTextStream>
#include <QThread>
#include <QWaitCondition>

/**
 * @brief Esta clase es una abstracón de los recursos a los cuáles los núcleos deben
 * acceder para completar su labor.
 *
 * Los miembros de esta clase son recursos críticos, ya que ambos núcleos deben acceder a él.
 * Los recursos críticos de esta clase son controlados por medio de mutex.
 *
 * @author	Kevin Delgado Sandí	(kefdelgado@gmail.com).
 * @author	Jean Carlo Zuñiga	(jeanczm@gmail.com).
 */
class Procesador : public QObject {
    Q_OBJECT

public:
    Procesador (const QStringList& nombre_archivos, int latencia_de_memoria, int trasferencia, int quantum, QObject* parent = 0);
    ~Procesador();


    // Métodos

    /**
    * @brief Verifica si la cola de procesos está vacía.
    * @return true si la cola esta vacía. false en caso contrario.
    */
    bool cola_vacia();

    /**
    * @brief Toma el contexto de un proceso de la cola de procesos.
    *
    * Cuando toma el contexto, desaparece de la cola de procesos.
    *
    * @return Primer proceso en la cola.
    */
    Proceso tomar_proceso();

    /**
     * @brief Toma el contexto de un proceso que le envía el núcleo
     * y lo encola en la cola de procesos.
     * @param proceso_a_encolar contexto de proceso a encolar.
     */
    void encolar_proceso(const Proceso& proceso_a_encolar);

    /**
     * @brief Devuleve el quatum que digitó el usuario.
     * @return quatum que digitó el usuario.
     */
    int obtener_quatum() const;

    /**
     * @brief Obtiene la duración de trasferencia en tomar un bloque de
     * memoria y copiarla en memoria cache y viceversa.
     * @return Obtiene el tiempo que dura la máquina en traer y/o guardar
     * un (datos) o varios (instrucciones) bloques entre la caché y memoria.
     */
    int obtener_duracion_transferencia_memoria_a_cache() const;

    /**
     * @brief Se obtiene una copia del bloque deseado de memoria principal de instrucciones.
     * @param numero_bloque que se desea obtener de la memoria.
     * @return Copia del bloque pedido.
     */
    BloqueInstruccion obtener_bloque_instrucciones(int numero_bloque) const;

    /**
     * @brief Se obtiene una copia del bloque deseado de memoria principal de datos.
     * @param numero_bloque que se desea obtener de la memoria.
     * @returnCopia del bloque pedido.
     */
    BloqueDato obtener_bloque_datos(int numero_bloque) const;

    /**
     * @brief Intenta tomar el bus de datos si este se encuentra libre.
     * Si ya esta haciendo ocupado por otra cache, devuelve un resultado negativo.
     * @return @c true si pudo obtener acceso al bus, @c falso en caso contrario.
     */
    bool bus_de_memoria_instrucciones_libre();

    /**
     * @brief Libera el bus que va a la memoria de instrucciones.
     */
    void liberar_bus_de_memoria_instrucciones();

    /**
     * @brief Barrera para sincronizar la ejecucción de los núcleos.
     *
     * Espera a que todos lo núcleos llamen a esta método para poder
     * aumentar el reloj. Con esta barrera se garantiza la ejecucción
     * sincronizada de los núcleos.
     */
    void aumentar_reloj();

    /**
     * @brief Modifica las variables que son utilizadas en la barrera
     * para evitar el bloque permanente de los demás núcleos.
     */
    void fin_nucleo();


    /**
     * @brief Este método se encarga de realizar todas las operaciones
     *        correspondientes a la coherencia de todas las cachés.
     *
     * Este método reliza las operaciones de LOAD y STORE .
     *
     * @param direccion_fisica numero de bloque que se desea traer.
     * @param numero_nucleo numero de núcleo que hace la petición.
     * @param store bandera que indica si la instrucción es un *store*.
     * @param dato dato que se bedde guardar en case de que sea un *store*.
     * @return Si fue un LOAD devuelve el valor solicitado, un STORE devuelve un -1.
     */
    int realiza_operacion_cache_datos(int direccion_fisica, int numero_nucleo, bool store = false, int dato = 0);

    void guardar_direccion_en_bloque_con_candado_RL(int numero_nucleo, int direccion_fisica);

    int obtener_bloque_con_candado_RL(int numero_nucleo);

    int obtener_direccion_en_bloque_con_candado_RL(int numero_nucleo);

    void imprimir();

private:
    const int m_quantum;
    const int m_duracion_transferencia_memoria_a_cache;

    int* const m_memoria_instrucciones;		/**< Memoria donde se encontrarán las instrucciones de los hilos */
    int* const m_memoria_datos;				/**< Memoria donde se encontrarán los datos que necesitarán los hilos */

    QQueue<Proceso> m_cola_procesos;		/** Cola que contrendrá a los hilos */

    // Mutex para los recursos compartidos
    QMutex m_mutex_memoria_instrucciones;	/**< Mutex que se encarga de sincronizar la lectura y escritura de la memoria de instrucciones */
    QMutex m_mutex_cola_procesos;			/**< Mutex que se encarga de sincronizar la lectura y escritura de la cola de procesos */
    QMutex m_mutex_bus_cache_datos;			/**< Mutex que se encarga de simular el bus que tiene las caches para hacer snooping */

    // Para el aumentar el reloj
    int m_numero_de_nucleos;				/**< Tiene un contador de los números de núcleos que acceden a sus recursos */
    unsigned long m_reloj;					/**< Reloj del la máquina. Con él se simula la sincronización de hilos */
    int m_cuenta;
    QMutex m_mutex_numero_de_nucleos;
    QMutex m_mutex_barrera;					/**< Mutex que se encarga de sincronizar el aumento del reloj del sistema */
    QWaitCondition m_condicion;				/**< Los núcleos se quedan bloqueados con esta condición hasta que uno los deesbloquee */

    Cache* const m_cache_datos;             /**< Contendrá las cachés de datos de cada núcleo */


    // Variables auxiliares

    // Esta variable tendrá la posición libre de la memoria de instrucciones
    int m_indice_memoria_instrucciones;

    // Contador para asignar los PID a los hilos
    int m_pid;

    int* const m_bloques_RL;


    // Métodos auxiliares
    void tomar_bus_cache_datos(int numero_nucleo);

    void liberar_bus_cache_datos();

    void tomar_cache_foranea(int indice_cache);

    void liberar_cache_foranea(int indice_cache);

    void guardar_bloque_en_memoria_datos(int numero_bloque, const BloqueDato& bloque_a_guardar);

    void actualizar_estados_cache_datos();

};
#endif // PROCESADOR_H
