#ifndef QISKIT_H
#define QISKIT_H

#include <stdbool.h>

/* Initialize the Qiskit connection (call once at startup) */
void qiskit_init(void);

/* Shutdown the Qiskit connection (call once at cleanup) */
void qiskit_shutdown(void);

/* Get a quantum random bit (0 or 1) from the Qiskit server.
 * Falls back to local rand() if the server is unreachable. */
int qiskit_random_bit(void);

/* Get a quantum random float [0.0, 1.0) from the Qiskit server.
 * Uses the quantum bit as the source of randomness. */
float qiskit_random_float(void);

/* Returns true if the last call used the real Qiskit server */
bool qiskit_is_connected(void);

#endif
