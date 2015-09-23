#ifndef DEFINICIONES
#define DEFINICIONES

#include <QDebug>

// Definiciones
#define NUMERO_BYTES_PALABRA 4
#define NUMERO_PALABRAS_BLOQUE 4
#define NUMERO_BLOQUES_INSTRUCCIONES 40
#define NUMERO_BLOQUES_CACHE 8

#define NUMERO_REGISTROS 33 // 32 registros + PC
#define PC 32

// Estructuras

/**
 * @brief Datos_usuario struct
 * Estructura que contendrá las variable que el usuario digite
 * con respecto a tiempo de duración al realizar una actividad.
 */
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


/**
 * @brief Proceso struct
 * Esta estructura define un proceso que debe esperar en la cola de procesos
 * hasta ser ubicado en un núcleo para su ejecucción.
 */
struct Proceso {
	int pid;
	int registros[NUMERO_REGISTROS];

	Proceso() {
		for (int i = 0; i < NUMERO_REGISTROS; ++i) {
			registros[i] = 0;
		}
	}
};


/**
 * @brief Palabra struct
 * Representa la unidad lógica de palabra
 */
struct Palabra {
	int celda[NUMERO_BYTES_PALABRA];
};


/**
 * @brief Bloque struct
 * Representa la unidad lógica de bloque.
 */
struct Bloque {
	Palabra palabra[NUMERO_PALABRAS_BLOQUE];

	// Solo lo usará la chaché
	int identificador_de_bloque_memoria;

	void print() {
		for (int i = 0; i < 4; ++i) {
			qDebug() << "Palabra[" << i << "] = ";
			for (int j = 0; j < 4; ++j) {
				qDebug() << "Celda[" << j << "] = " << palabra[i].celda[j];
			}
		}
	}
};


/**
 * @brief Cache struct
 * Representa una caché de instrucciones y le pertenece a un núcleo.
 */
struct Cache {
	Bloque bloques[NUMERO_BLOQUES_CACHE];
};

#endif // DEFINICIONES
