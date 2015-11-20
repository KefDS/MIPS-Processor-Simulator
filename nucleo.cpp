#include "nucleo.h"

Nucleo::Nucleo (Procesador& procesador, int numero_nucleo, QObject* parent) :
    QObject (parent),
    m_numero_nucleo(numero_nucleo),
    m_procesador (procesador),
    m_cache_instrucciones(new Cache()),
	m_registros(new int[NUMERO_REGISTROS-1])
{

}

Nucleo::~Nucleo() {
    delete[] m_registros;
    delete m_cache_instrucciones;
}

void Nucleo::run() {
    emit reportar_estado (QString("Núcleo %1 inició su ejecución.").arg(m_numero_nucleo));

    // Mientras hayan procesos en la cola
    while(!m_procesador.cola_vacia()) {
        Proceso proceso_actual = m_procesador.tomar_proceso();
        cargar_contexto(proceso_actual);
        emit reportar_estado(QString("El núcleo %1 va a ejecutar el hilo con PID %2.").arg(m_numero_nucleo).arg(proceso_actual.pid));

        m_quantum_de_proceso_actual = m_procesador.obtener_quatum();

        bool termino_hilo = false;

        while (m_quantum_de_proceso_actual > 0) {
            Instruccion instruccion = obtiene_instruccion();
            termino_hilo = ejecutar_instruccion(instruccion);
            --m_quantum_de_proceso_actual;
            m_procesador.aumentar_reloj();
        }

        // Cuando se acaba el Quatum, se baja la bandera de LL.
        m_procesador.guardar_registro_RL(m_numero_nucleo, -1);
        m_procesador.guardar_bandera(m_numero_nucleo, false);

        // Si el hilo no ha terminado, se envía a la cola de procesos de nuevo
        if (!termino_hilo) {
            guardar_contexto(proceso_actual);
            m_procesador.encolar_proceso(proceso_actual);
        }
        else {
            emit reportar_estado(obtener_resumen_proceso());
        }
    }

    m_procesador.fin_nucleo();

    emit reportar_estado (QString("Núcleo %1 terminó su ejecución.").arg(m_numero_nucleo));
}

void Nucleo::cargar_contexto(const Proceso& proceso) {
    for (int i = 0; i < NUMERO_REGISTROS - 1; ++i) {
        m_registros[i] = proceso.registros[i];
    }
    m_procesador.guardar_registro_RL(m_numero_nucleo, proceso.registros[RL]);
}

void Nucleo::guardar_contexto(Proceso& proceso) const {
    for (int i = 0; i < NUMERO_REGISTROS - 1; ++i) {
        proceso.registros[i] = m_registros[i];
    }
    proceso.registros[RL] = m_procesador.obtener_registro_RL(m_numero_nucleo);
}

bool Nucleo::ejecutar_instruccion(const Instruccion& instruccion) {
    bool es_instruccion_fin = false;
    int direccion_dato;

    // Mueve el PC hacia la siguiente instrucción
    m_registros[PC] += NUMERO_BYTES_PALABRA;

    // Código de operación
    switch (instruccion.celda[0]) {
    case DADDI:
        m_registros[instruccion.celda[2]] = m_registros[instruccion.celda[1]] + instruccion.celda[3];
        break;

    case DADD:
        m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] + m_registros[instruccion.celda[2]];
        break;

    case DSUB:
        m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] - m_registros[instruccion.celda[2]];
        break;

    case DMUL:
        m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] * m_registros[instruccion.celda[2]];
        break;

    case DDIV:
        m_registros[instruccion.celda[3]] = m_registros[instruccion.celda[1]] / m_registros[instruccion.celda[2]];
        break;

    case LW:
        direccion_dato = instruccion.celda[3] + m_registros[instruccion.celda[1]];
		m_registros[instruccion.celda[2]] = m_procesador.realiza_operacion_cache_datos(direccion_dato, m_numero_nucleo);
        break;

    case SW:
        direccion_dato = instruccion.celda[3] + m_registros[instruccion.celda[1]];
		m_procesador.realiza_operacion_cache_datos(direccion_dato, m_numero_nucleo , true, m_registros[instruccion.celda[2]]);
        break;

    case BEQZ:
        if(m_registros[instruccion.celda[1]] == 0) {
            m_registros[PC] += instruccion.celda[3] * NUMERO_BYTES_PALABRA;
        }
        break;

    case BNEZ:
        if(m_registros[instruccion.celda[1]] != 0) {
            m_registros[PC] += instruccion.celda[3] * NUMERO_BYTES_PALABRA;
        }
        break;

    case JAL:
        m_registros[31] = m_registros[PC];
        m_registros[PC] += instruccion.celda[3];
        break;

    case JR:
        m_registros[PC] = m_registros[instruccion.celda[1]];
        break;

    case LL:
        direccion_dato = instruccion.celda[3] + m_registros[instruccion.celda[1]];
		m_procesador.guardar_registro_RL (m_numero_nucleo, direccion_dato);
		m_procesador.guardar_bloque_candado_RL(m_numero_nucleo, (direccion_dato / 16));
        m_registros[instruccion.celda[2]] = m_procesador.realiza_operacion_cache_datos(direccion_dato, m_numero_nucleo);
        m_procesador.guardar_bandera(m_numero_nucleo, true);
		qDebug() << "Núcleo: " << m_numero_nucleo << " entró a Load Link";
        m_procesador.imprimir();
        break;

    case SC:
		qDebug() << "Núcleo: " << m_numero_nucleo << " entro a Store Conditional";
        m_procesador.imprimir();
        direccion_dato = instruccion.celda[3] + m_registros[instruccion.celda[1]];

		qDebug() << "Núcleo: " << m_numero_nucleo <<" RL: " << m_procesador.obtener_registro_RL(m_numero_nucleo) << ", direccion store: " << direccion_dato;
        // Si la dirección de memoria en la que debe hacerce un store, coincide con el valor de RL
		if(m_procesador.obtener_registro_RL(m_numero_nucleo) == direccion_dato) {
			m_procesador.realiza_operacion_cache_datos(direccion_dato, m_numero_nucleo , true,  m_registros[instruccion.celda[2]]);
        }
        else {
            m_registros[instruccion.celda[2]] = 0;
        }
        m_procesador.guardar_bandera(m_numero_nucleo, false);
        break;

    case FIN:
        es_instruccion_fin = true;
        m_quantum_de_proceso_actual = 0;
        break;

    default:
        emit reportar_estado (QString("La instrucción %1 no es válida para esta simulación, por favor presione terminar simulación.").arg(instruccion.celda[0]));
    };

    return es_instruccion_fin;
}

Instruccion Nucleo::obtiene_instruccion() {
    int numero_de_bloque = m_registros[PC] / 16;
    int numero_de_palabra = (m_registros[PC] % 16) / NUMERO_PALABRAS_BLOQUE;

    // índice donde se debería encontrar el bloque en caché si estuviera
    int indice = numero_de_bloque % NUMERO_BLOQUES_CACHE;

    // El bloque no está en caché
    if (numero_de_bloque != m_cache_instrucciones->identificador_de_bloque_memoria[indice]) {

        // Debe esperar mientras el bus no esté disponible
        while(!m_procesador.bus_de_memoria_instrucciones_libre()) {
            m_procesador.aumentar_reloj();
            emit reportar_estado(QString("Núcleo %1 está esperando a que se desocupe el bus de datos").arg(m_numero_nucleo));
        }

        // Se pide el bloque a memoria prinicipal
        m_cache_instrucciones->bloques[indice] = m_procesador.obtener_bloque_instrucciones(numero_de_bloque);
        m_cache_instrucciones->identificador_de_bloque_memoria[indice] = numero_de_bloque;

        // Aquí se da el retraso de tiempo en el cual se debe ir a memoria a traer un bloque.
        int tiempo_de_espera = m_procesador.obtener_duracion_transferencia_memoria_a_cache();
        for(int i = 0; i < tiempo_de_espera; ++i) {
            m_procesador.aumentar_reloj();
            emit reportar_estado(QString("Núcleo %1 está ocupando el bus de datos").arg(m_numero_nucleo));
        }

        m_procesador.liberar_bus_de_memoria_instrucciones();
    }

    return m_cache_instrucciones->bloques[indice].palabra[numero_de_palabra];
}

QString Nucleo::obtener_resumen_proceso() {
    // Imprimir estatus después del ciclo de reloj
    QString resumen (QString("Núcleo %1. Registros: ").arg(m_numero_nucleo));
    for (int i = 0; i < NUMERO_REGISTROS; ++i) {
        resumen.append(QString("[%1] : %2, ").arg(i).arg(m_registros[i]));
    }
    return resumen;
}


