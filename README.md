# Programa que simula un procesador MIPS de doble núcleo
Esta es una tarea del curso CI-1323 Arquitectura de Computadoras

## Objetivo
Se debe diseñar y programar la simulación de un procesador MIPS de doble núcleo (un solo hilo cada uno).
El cual debe ser capaz de ejecutar hilos- que por facilidad serán todos del mismo proceso- y que utilizan para su sincronización semáforos también simulados. Memoria compartida centralizada, coherencia de caché con invalidación para escrituras.

## Descripción del sistema que se va a simular
* El procesador es el MIPS de 5 etapas visto en clase, el cual ejecutará solo un subconjunto de las instrucciones para enteros.

* Cada núcleo tiene una caché de datos y una de instrucciones. El sistema, cuando ejecuta hilos de un mismo proceso, trabaja como un multiprocesador de memoria compartida centralizada **(SMP)**. Esto significa que hay una área d ela memoria principal que se comparte “por igual” por los dos núcleos.

* Por simplicidad suponga que hay dos puertos para memoria principal:
Uno para el bus de instrucciones el cual solo puede ser utilizado por una de las dos cachés de instrucciones cada vez)
Otro puerto para el bus de datos, el cual solo puede ser usado por una de las dos cachés de datos cada vez.

* Los bloques de memoria y de cachés son de 4 palabras, cada una de 4 bytes. Tanto las cachés de instrucciones como las cachés de datos tienen capacidad para 8 bloques y son de mapeo directo. Cada instrucción MIPS codificada es de una palabra de longitud.

* Las cachés de datos tienen la estrategia *Write Back* para hit de escritura (escribe solo en caché, y la actualización a memoria se hace o cuando se vaya a reemplazar el bloque, o cuando se debe copiar a otra caché para resolver un fallo de caché para ese bloque) y la estrategia “Write allocate” para fallos de escritura, es decir se sube el bloque a caché (Se trae de otra caché si está modificado ese bloque en esa otra caché, o de memoria principal en cualquier otro caso). Tanto en la caché de instrucciones como en la de datos, y en memoria principal, los bloques son de 4 palabras.

* La memoria principal contiene 128 bloques *(128 * 4 = 512 palabras, o sea 2048 bytes)*. Para simplificar la labor del “sistema operativo, el cual en la simulación no será otro más que parte de su mismo programa” se va a suponer que en esta memoria del bloque 0 al 39 solo se podrán almacenar instrucciones (es decir los hilos que correrán en su simulación) y a partir del bloque 40 (dirección 640) se tendrá el área de datos compartida para ambos núcleos. No se necesita simular la parte de memoria que no será parte de la memoria compartida.

* La lectura o escritura de una palabra en memoria tarda *m* ciclos (latencia de memoria). Para efectos de la simulación, el valor de *m* será dado como un dato inicial del usuario.

* Tanto el bus de datos como el de instrucciones son de una palabra de ancho y tardan *b* ciclos para transferir una palabra de memoria a caché o viceversa. El valor de *b* se le debe solicitar al usuario al inicio.

* Cada núcleo tiene 32 registros de propósito general: *R0, R1, ... R31*. El registro *R0* siempre tendrá asignado el valor cero y no puede utilizarse como registro destino para operación alguna. Y el registro *RL* para enlace para las instrucciónes *LL* y *SC*. El "contexto" entonces para un hilo estará compuesto por *R0 ...R31, RL* y el *PC*.

* La asignación de la CPU la hace el sistema operativo utilizando un algoritmo de planificación por turno rotatorio **(round robin)** con un quantum de *X* ciclos de reloj para cada “hilo”.
Para efectos de la simulación, el valor del *quantum* será dada como un dato inicial del usuario. Desde la cola de listos un hilo se asigna a cualquier núcleo que esté libre (independientemente del núcleo en el que corrió inicialmente).

* La sincronización entre hilos se realiza solo mediante semáforos binarios (mutex o locks) los cuales permiten como máximo a un hilo el ingreso a una sección crítica (exclusión mutua).
Estos mutex se implementarán tan solo usando dos instrucciones que agregaremos al MIPS: *Load linked (LL)* y el *Store Conditional (SC)*, instrucciones que serán incluidas como parte del código de los hilos que correrán en su procesador, para lograr la sincronización.
Muy diferente a como usualmente ocurre en la realidad, ya que cuando en un programa se ha incluido una herramienta de sincronización, cuando ésta debe ejecutarse, lo que sucede es que se da una interrupción y se hace un llamado a una rutina del sistema operativo, la cual es la que realiza la sincronización, utilizando instrucciones como el *LL* y *SC*.


## Uso (Temporal)
Al ejecutar el programa se le envía por parámetro un archivo con la siguiente información (cada uno en una línea y en el orden que se dicta):
* Latencia de memoria *(m)*.
* Ciclos para transferir una palabra de memoria a caché o viceversa. *(b)*
* Quatum (Número de ciclos que tiene un proceso antes de que el procesador cambie de contexto) *(x)*.
* Rutas donde se encuentra los hilos del proceso a correr en el procesador.
