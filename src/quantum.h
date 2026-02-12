#ifndef QUANTUM_H
#define QUANTUM_H

#include "common.h"

// Qubit Management
void init_qubit(Qubit *q);
void apply_hadamard_gate(Qubit *q);
void apply_pauli_x_gate(Qubit *q);
void apply_cnot_gate(Qubit *control, Qubit *target); // Simplified CNOT
void measure_qubit(Qubit *q);
float get_qubit_probability(Qubit *q, int outcome);

// Portal Management
void init_portals(GameState *game);
void handle_portal_teleport(GameState *game);
bool can_use_portal(GameState *game, int portal_idx);

#endif
