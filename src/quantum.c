#include "quantum.h"
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
  // Swaps alpha and beta
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

  // This is a simplified simulation since we don't store the full state vector
  // of the system In a real simulation we'd need a tensor product of the
  // states. For this game jam, we'll approximate: If control is likely |1>,
  // flip target. If control is superposition, we entitle them (simplified).

  // For now, let's just use probability to decide if we flip, but without
  // collapsing? Actually, accurate CNOT requires a multi-qubit system state.
  // Let's implement a "Probabilistic CNOT" which is physically wrong but
  // functionally OK for the game logic: "If measure(control) would be 1, apply
  // X to target". AND it collapses the control qubit. Wait, that's not CNOT.

  // Better approximation for independent qubit structs:
  // Calculate P(control=1).
  // Mix target state based on that probability?
  // No, let's keep it simple: CNOT only works if control is fully |0> or |1>
  // (Classical control) OR allow it to Entagle them (special state).

  // For now: Classical Control CNOT
  float p1 = get_qubit_probability(control, 1);
  if (p1 > 0.99f) {
    apply_pauli_x_gate(target);
  } else if (p1 > 0.01f) {
    // Control is in superposition.
    // We can't properly simulate entanglement with independent structs easily
    // without a system state vector. Let's just say "Entanglement required" and
    // do nothing or make a sound. Or we force collapse control?
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
  float r = (float)rand() / (float)RAND_MAX;

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

  // Check entanglement requirement
  if (p->requires_entanglement) {
    // TODO: Check if player is entangled
    // For now, assume false if not implemented
    return false;
  }

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
    // Feedback: Wrong phase?
    return;
  }

  int dest_idx = portal->linked_portal_index;
  if (dest_idx >= 0 && dest_idx < MAX_PORTALS &&
      game->portals[dest_idx].active) {
    // Teleport!
    game->player.position = game->portals[dest_idx].position;

    // Optional: consume 1 turn or just move instant?
    // Let's make it instant but play sound/effect
    if (IsAudioSoundValid(teleport_sound)) {
      PlayAudioSound(teleport_sound);
    }
  }
}
