#include "quantum.h"
#include "qiskit.h"
#include "utils.h"
#include <math.h>

/* ===== QUBIT LOGIC ===== */

void init_qubit(Qubit *q) {
    q->alpha_real = 1.0f;
    q->alpha_imag = 0.0f;
    q->beta_real = 0.0f;
    q->beta_imag = 0.0f;
    q->is_measured = false;
    q->measured_value = 0;
    q->active = true;
}

void apply_hadamard_gate(Qubit *q) {
    if (q->is_measured)
        return;

    // H = 1/sqrt(2) * [[1, 1], [1, -1]]
    float s = 1.0f / sqrtf(2.0f);

    float new_alpha_r = s * (q->alpha_real + q->beta_real);
    float new_alpha_i = s * (q->alpha_imag + q->beta_imag);
    float new_beta_r = s * (q->alpha_real - q->beta_real);
    float new_beta_i = s * (q->alpha_imag - q->beta_imag);

    q->alpha_real = new_alpha_r;
    q->alpha_imag = new_alpha_i;
    q->beta_real = new_beta_r;
    q->beta_imag = new_beta_i;

    if (IsAudioSoundValid(qubit_rotate_sound)) {
        PlayAudioSound(qubit_rotate_sound);
    }
}

void apply_pauli_x_gate(Qubit *q) {
    if (q->is_measured)
        return;

    // X = [[0, 1], [1, 0]]
    float temp_r = q->alpha_real;
    float temp_i = q->alpha_imag;
    q->alpha_real = q->beta_real;
    q->alpha_imag = q->beta_imag;
    q->beta_real = temp_r;
    q->beta_imag = temp_i;

    if (IsAudioSoundValid(qubit_rotate_sound)) {
        PlayAudioSound(qubit_rotate_sound);
    }
}

void apply_cnot_gate(Qubit *control, Qubit *target) {
    if (control->is_measured || target->is_measured)
        return;

    float p1 = get_qubit_probability(control, 1);
    if (p1 > 0.99f) {
        apply_pauli_x_gate(target);
    } else if (p1 > 0.01f) {

        measure_qubit(control);
        if (control->measured_value == 1) {
            apply_pauli_x_gate(target);
        }
    }
}

void measure_qubit(Qubit *q) {
    if (q->is_measured)
        return;

    float p0 = get_qubit_probability(q, 0);
    float r = qiskit_random_float();

    if (r < p0) {
        q->measured_value = 0;
        q->alpha_real = 1.0f;
        q->alpha_imag = 0.0f;
        q->beta_real = 0.0f;
        q->beta_imag = 0.0f;
    } else {
        q->measured_value = 1;
        q->alpha_real = 0.0f;
        q->alpha_imag = 0.0f;
        q->beta_real = 1.0f;
        q->beta_imag = 0.0f;
    }

    q->is_measured = true;

    if (IsAudioSoundValid(measurement_sound)) {
        PlayAudioSound(measurement_sound);
    }
}

float get_qubit_probability(Qubit *q, int outcome) {
    if (outcome == 0) {
        return q->alpha_real * q->alpha_real + q->alpha_imag * q->alpha_imag;
    } else {
        return q->beta_real * q->beta_real + q->beta_imag * q->beta_imag;
    }
}

/* ===== PORTAL LOGIC ===== */

void init_portals(GameState *game) {
    for (int i = 0; i < MAX_PORTALS; i++) {
        game->portals[i].active = false;
        game->portals[i].linked_portal_index = -1;
    }
}

bool can_use_portal(GameState *game, int portal_idx) {
    if (portal_idx < 0 || portal_idx >= MAX_PORTALS)
        return false;
    QuantumPortal *p = &game->portals[portal_idx];
    if (!p->active)
        return false;

    // Check phase match
    // Player must be in same phase, OR in superposition
    bool phase_match = (game->player.phase_system.current_phase == p->phase);
    if (game->player.phase_system.state == PHASE_STATE_SUPERPOSITION)
        phase_match = true;

    if (!phase_match)
        return false;

    return true;
}

void handle_portal_teleport(GameState *game) {
    // Check if player is on a portal
    int p_idx = -1;
    for (int i = 0; i < MAX_PORTALS; i++) {
        if (game->portals[i].active &&
            ivec2_eq(game->player.position, game->portals[i].position)) {
            p_idx = i;
            break;
        }
    }

    if (p_idx == -1)
        return;

    QuantumPortal *portal = &game->portals[p_idx];

    if (!can_use_portal(game, p_idx)) {
        return;
    }

    int dest_idx = portal->linked_portal_index;
    if (dest_idx >= 0 && dest_idx < MAX_PORTALS &&
        game->portals[dest_idx].active) {
        // Teleport
        game->player.position = game->portals[dest_idx].position;

        if (IsAudioSoundValid(teleport_sound)) {
            PlayAudioSound(teleport_sound);
        }
    }
}
