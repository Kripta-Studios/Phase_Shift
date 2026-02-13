#ifndef QISKIT_H
#define QISKIT_H

#include <stdbool.h>

/* Inicializar conexión Qiskit (llamar una vez al inicio) */
void qiskit_init(void);

/* Cerrar conexión Qiskit (llamar una vez al limpiar) */
void qiskit_shutdown(void);

/* Obtener bit aleatorio cuántico (0 o 1) del servidor Qiskit.
 * Usa rand() local si servidor no disponible. */
int qiskit_random_bit(void);

/* Obtener float aleatorio cuántico [0.0, 1.0) del servidor Qiskit.
 * Usa bit cuántico como fuente de aleatoriedad. */
float qiskit_random_float(void);

/* Devuelve verdadero si última llamada usó servidor Qiskit real */
bool qiskit_is_connected(void);

#endif
