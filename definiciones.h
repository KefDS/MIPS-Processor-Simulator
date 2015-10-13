#ifndef DEFINICIONES
#define DEFINICIONES

// Definiciones
#define NUMERO_BYTES_PALABRA 4
#define NUMERO_PALABRAS_BLOQUE 4
#define NUMERO_BLOQUES_INSTRUCCIONES 40
#define NUMERO_BLOQUES_CACHE 8
#define NUMERO_BYTES_MEMORIA_INSTRUCCIONES	NUMERO_BYTES_PALABRA * NUMERO_PALABRAS_BLOQUE * NUMERO_BLOQUES_INSTRUCCIONES
#define NUMERO_NUCLEOS 2

#define NUMERO_REGISTROS 33 // 32 registros + PC
#define PC 32

// Codigos de operacion
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
struct Bloque {
	Palabra palabra[NUMERO_PALABRAS_BLOQUE];
};


/**
 * @brief Representa una caché de instrucciones y le pertenece a un núcleo.
 */
struct Cache {
	Bloque bloques[NUMERO_BLOQUES_CACHE];
	int identificador_de_bloque_memoria[NUMERO_BLOQUES_CACHE];

	Cache() {
		for (int i = 0; i < NUMERO_BLOQUES_CACHE; ++i) {
			// Inválido
			identificador_de_bloque_memoria[i] = -1;
		}
	}
};
#endif // DEFINICIONES
