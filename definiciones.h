#include <QMutex>

#ifndef DEFINICIONES
#define DEFINICIONES

// Definiciones
#define NUMERO_BYTES_PALABRA 4
#define NUMERO_PALABRAS_BLOQUE 4
#define NUMERO_BLOQUES_INSTRUCCIONES 40
#define NUMERO_BLOQUES_DATOS 88
#define NUMERO_BLOQUES_CACHE 8
#define NUMERO_BYTES_MEMORIA_INSTRUCCIONES		NUMERO_BYTES_PALABRA * NUMERO_PALABRAS_BLOQUE * NUMERO_BLOQUES_INSTRUCCIONES
#define NUMERO_BYTES_MEMORIA_DATOS				NUMERO_BYTES_PALABRA * NUMERO_PALABRAS_BLOQUE * NUMERO_BLOQUES_DATOS
#define NUMERO_NUCLEOS 2

#define NUMERO_REGISTROS 34 // 32 registros + PC + RL
#define PC 32
#define RL 33

// Representa los estados en la que puede estar la caché
enum class ESTADO : char { COMPARTIDO, MODIFICADO, INVALIDO, SIN_CAMBIO };

// Códigos de operación
#define DADDI	8
#define DADD	32
#define DSUB	34
#define DMUL	12
#define DDIV	14
#define LW		35
#define SW		43
#define BEQZ	4
#define BNEZ	5
#define JAL		3
#define JR		2
#define LL		50
#define SC		51
#define FIN		63


// Estructuras

/**
 * @brief Estructura que contendrá las variable que el usuario digite
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
 * @brief Esta estructura define un proceso que debe esperar en la cola de procesos
 * hasta ser ubicado en un núcleo para su ejecucción.
 */
struct Proceso {
	int pid;
	int registros[NUMERO_REGISTROS];

	Proceso(int pid, int pc) : pid(pid) {
		for (int i = 0; i < NUMERO_REGISTROS; ++i) {
			registros[i] = 0;
		}
		registros[PC] = pc;
	}
};


/**
 * @brief Representa la unidad lógica de palabra.
 */
struct Palabra {
	int celda[NUMERO_BYTES_PALABRA];
};

/**< Para evitar confusiones se utilizará la frase Instruccion cuando lo que se quiere es una instruccion y no una palabra */
typedef Palabra Instruccion;

/**
 * @brief Representa la unidad lógica de bloque.
 */
struct BloqueInstruccion {
	Palabra palabra[NUMERO_PALABRAS_BLOQUE];
};

struct BloqueDato {
    int palabra[NUMERO_PALABRAS_BLOQUE];
};

/**
 * @brief Representa una caché de instrucciones y le pertenece a un núcleo.
 */
struct Cache {

    // Atrbutos para caché de datos
    BloqueInstruccion bloques[NUMERO_BLOQUES_CACHE];

    // Compartido
	int identificador_de_bloque_memoria[NUMERO_BLOQUES_CACHE];

	// Atributos para la caché de datos
    BloqueDato bloque_dato[NUMERO_BLOQUES_CACHE];
	ESTADO estado_del_bloque[NUMERO_BLOQUES_CACHE];
	ESTADO estado_del_bloque_siguiente_ciclo_reloj[NUMERO_BLOQUES_CACHE];
    QMutex mutex;

	Cache() {
		for (int i = 0; i < NUMERO_BLOQUES_CACHE; ++i) {
			identificador_de_bloque_memoria[i] = -1;
			estado_del_bloque[i] = ESTADO::INVALIDO;
			estado_del_bloque_siguiente_ciclo_reloj[i] = ESTADO::SIN_CAMBIO;
		}
	}
};
#endif // DEFINICIONES
