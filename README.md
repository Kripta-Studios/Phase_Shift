# 🌀 Phase Shift — Quantum Puzzle Game

<p align=center>
  <img src="./assets/icon.png">
</p>

> *"Tu conciencia está entrelazada cuánticamente con un acelerador de partículas..."*

**Phase Shift** es un juego de puzzles 2D basado en mecánicas cuánticas reales. Eres el **Sujeto 44**, un científico atrapado en un experimento cuántico fallido. Tu conciencia existe en dos fases de la realidad simultáneamente. Completa 8 niveles de puzzles cuánticos antes de que tu coherencia cuántica colapse permanentemente.

Desarrollado por **Kripta Studios** · Motor: [Raylib](https://www.raylib.com/) · Lenguaje: C99

---

## 🎮 Cómo se Juega

### Controles

| Tecla | Acción |
|-------|--------|
| `W/A/S/D` o `↑←↓→` | Movimiento (basado en turnos) |
| `E` | Cambiar de fase cuántica (Roja ↔ Azul) |
| `Space` | Plantar bomba |
| `Enter` | Avanzar diálogos |
| `F11` | Pantalla completa |
| `F5` | Saltar nivel (debug) |

### Objetivo

Llega a la **Cámara de Estabilización** (puerta de salida 🟢) en cada nivel antes de que tu **coherencia cuántica** llegue a 0%. Completa los 8 niveles para ganar.

### Cómo se Gana

- Resuelve puzzles cuánticos usando el cambio de fase, superposición y ecos cuánticos
- Mantén tu coherencia por encima de 0%
- Alcanza la salida de cada nivel (marcada en verde brillante)

### Cómo se Pierde

- Coherencia llega a **0%** → Tu función de onda colapsa
- Un eeper (enemigo) te toca → Muerte instantánea
- Reinicio desde el último checkpoint o inicio del nivel

---

## 🔬 Mecánicas Cuánticas

### 1. Sistema de Fases (Tecla `E`)

El jugador puede cambiar entre dos fases de la realidad:

| Aspecto | 🔴 Fase Roja (Materia) | 🔵 Fase Azul (Energía) |
|---------|------------------------|------------------------|
| Ambiente | Naranjas/rojos cálidos | Cianes/púrpuras fríos |
| Plataformas rojas | Sólidas ✅ | Fantasma (atraviesas) 👻 |
| Plataformas azules | Fantasma 👻 | Sólidas ✅ |
| Plataformas neutras | Siempre sólidas | Siempre sólidas |

### 2. Superposición Cuántica (0.8s)

Durante el cambio de fase hay una ventana de **3 turnos** donde existes en **AMBAS FASES** simultáneamente:
- Interactúas con objetos de ambas fases
- Puedes activar botones rojos Y azules a la vez
- Eres vulnerable a peligros de ambas fases

### 3. Quantum Echo (Eco Cuántico)

Al pulsar **ESPACIO**, entras en Superposición. Un "Eco" grabará tus acciones durante 12 turnos y luego las repetirá en bucle.

**Guía de uso para Puzzles:**
1. **Posiciónate**: Colócate donde quieras que actúe tu Eco (ej. sobre un botón).
2. **Graba**: Pulsa ESPACIO. 
3. **Espera**: Usa la tecla **[T]** o **[.]** para pasar turnos sin moverte.
4. **Ejecuta**: Al terminar la grabación, el Eco aparecerá y repetirá lo que hiciste (ej. quedarse pisando el botón).
5. **Coopera**: Mientras tu Eco mantiene el botón pulsado, tú eres libre para cruzar la puerta o pulsar un segundo botón.

### 4. Detectores Cuánticos

Cámaras que "observan" solo UNA fase. Si te detectan:
- Fuerzan colapso a su fase de detección
- Bloquean cambio de fase durante 5 turnos
- Pierdes 15% de coherencia

### 5. Túnel Cuántico

Muros especiales que puedes intentar atravesar durante la superposición:
- Probabilidad base: **50%** (inspirado en Qiskit)
- Con estabilizador: **75%**
- Si fallas: 2 turnos atrapado + 20% coherencia perdida

### 6. Objetos Entrelazados

Pares de cajas conectadas cuánticamente:
- Mover una → la otra se mueve en dirección **OPUESTA**
- Solo interactúas con la caja de tu fase actual

### 7. Coherencia Cuántica (Barra de Vida)

| Acción | Efecto |
|--------|--------|
| Decaimiento natural | -1% cada 5 turnos |
| Fallo de túnel | -20% |
| Detección | -15% |
| Partícula de coherencia | +5% |
| Checkpoint | +100% (restauración completa) |

---

## 🗺️ Niveles (8 niveles, 4 mundos)

### Mundo 1: Laboratorio de Superficie
- **Nivel 1 — Tutorial**: Movimiento, cambio de fase básico. Atraviesa muros rojos en Fase Azul.
- **Nivel 2 — Superposición**: Presiona botones rojo+azul simultáneamente durante la superposición.

### Mundo 2: Cámaras de Experimentación
- **Nivel 3 — Quantum Echo**: Graba pararte en un botón, el eco lo mantiene mientras avanzas.
- **Nivel 4 — Detectores y Entrelazamiento**: Evita detectores y posiciona cajas entrelazadas.

### Mundo 3: Reactor de Partículas
- **Nivel 5 — Túnel Cuántico**: Elige: túnel arriesgado (50%) vs camino largo con desgaste de coherencia.
- **Nivel 6 — Combo Avanzado**: Puzzle combinando Echo + Superposición.

### Mundo 4: Núcleo Cuántico
- **Nivel 7 — Gauntlet**: Serie de salas, cada una requiere una mecánica diferente.
- **Nivel 8 — Puzzle Final**: Puzzle multi-fase con temporizador de coherencia ajustado.

**Tiempo total**: 30-45 minutos (playthrough casual)

---

## 🔨 Compilación

### Dependencias

- **GCC** (compilador C99)
- **Raylib** 5.0+ (incluido como .dll en Windows)

### Compilar con Make

```bash
# Linux / MacOS / MSYS2
make clean
make
./eepers_c

# Solo Linux
./build-linux.sh

# Solo MacOS
./build-macos.sh

# Windows (MinGW)
./build-mingw32-w64.sh

# Windows (MSYS2)
./build-msys2.sh
```

### Makefile explicado

```makefile
CC = gcc                    # Compilador
CFLAGS = -Wall -Wextra -std=c99 -g  # Flags de compilación con warnings
LDFLAGS = -lraylib -lm -lpthread    # Librerías: Raylib, math, threads

# En Windows se añaden: -lglfw3 -lopengl32 -lgdi32 -lwinmm
# En MacOS se añaden frameworks: CoreVideo, IOKit, Cocoa, OpenGL
```

---

## 🧬 Arquitectura del Código

El código del juego ha sido refactorizado en múltiples módulos para mejorar la mantenibilidad:

- **src/main.c**: Punto de entrada, bucle principal y gestión de estados.
- **src/common.h**: Definiciones compartidas (structs, enums, constantes).
- **src/utils.c/h**: Funciones de utilidad (matemáticas, mapa, colisiones).
- **src/logic.c/h**: Lógica del juego, IA, actualizaciones de física cuántica.
- **src/render.c/h**: Sistema de renderizado visual.
- **src/levels.c/h**: Definición y carga de niveles.

### Estructuras de Datos Principales

| Struct | Propósito |
|--------|-----------|
| `GameState` | Estado global: mapa, jugador, enemigos, items, nivel actual |
| `PlayerState` | Posición, fase cuántica, coherencia, grabación de eco |
| `QuantumPhaseSystem` | Fase actual (roja/azul), estado de superposición |
| `CoherenceSystem` | Barra de vida cuántica con decaimiento |
| `QuantumEcho` | Grabación/reproducción de movimientos pasados |
| `QuantumDetector` | Cámaras con raycast de detección |
| `QuantumTunnel` | Muros con probabilidad de atravesar |
| `EntangledObject` | Pares de objetos con movimiento inverso |
| `PressureButton` | Botones de presión vinculados a puertas |
| `DialogSystem` | Sistema de diálogos con páginas y navegación |

### Flujo del Juego

```
main()
  ├── InitWindow / InitAudioDevice
  ├── init_game_state()
  ├── init_intro_dialogs()        ← Diálogos de introducción
  │
  ├── Game Loop:
  │   ├── GAME_STATE_DIALOG       ← Mostrar diálogos (Enter avanza)
  │   ├── GAME_STATE_PLAYING      ← Gameplay principal
  │   │   ├── Input → execute_turn()
  │   │   │   ├── game_player_turn()     ← Movimiento + items
  │   │   │   ├── game_eepers_turn()     ← IA enemigos
  │   │   │   ├── update_phase_system()  ← Superposición/eco
  │   │   │   ├── update_coherence()     ← Decaimiento
  │   │   │   ├── update_quantum_detectors()
  │   │   │   └── update_pressure_buttons()
  │   │   └── Render:
  │   │       ├── render_game_cells()    ← Mapa con fase-coloring
  │   │       ├── render_items()         ← Items con glow pulsante
  │   │       ├── render_quantum_effects() ← Ecos, detectores, partículas
  │   │       ├── render_player()        ← Jugador con outline
  │   │       ├── render_eepers()        ← Enemigos
  │   │       ├── render_dark_effects()  ← Viñeta, scanlines, glow
  │   │       └── render_hud()           ← Coherencia, fase, nivel
  │   ├── GAME_STATE_LEVEL_TRANSITION ← Fade + nombre del nivel
  │   └── GAME_STATE_WIN             ← Pantalla de victoria
  │
  └── Cleanup
```

### Pipeline de Renderizado

1. `ClearBackground` — Fondo oscuro profundo
2. `BeginMode2D` — Cámara 2D siguiendo al jugador
3. Celdas del mapa con colores de fase
4. Items con animación pulsante
5. Efectos cuánticos (ecos, detectores, partículas)
6. Jugador y enemigos
7. `EndMode2D`
8. Overlay oscuro: viñeta + scanlines + tinte de fase
9. HUD: coherencia, fase, nivel, diálogos

---

## 💻 Integración y Compilación

Para compilar el juego refactorizado en Windows (con MSYS2/MinGW), utiliza el siguiente comando:

```bash
gcc -O3 -Wall -Wno-missing-braces -std=c99 -I. -Isrc -L. -o eepers.exe src/main.c src/utils.c src/logic.c src/render.c src/levels.c -lraylib -lopengl32 -lgdi32 -lwinmm
```

Alternativamente, si tienes `make` instalado, simplemente ejecuta:
```bash
make
```

Esto generará el ejecutable `eepers_refactored.exe` (o el nombre definido en Makefile).

---

## 🎵 Créditos

- **Motor**: [Raylib](https://www.raylib.com/) by Ramon Santamaria
- **Música**: Type 42 — [SoundCloud](https://soundcloud.com/type42) · [YouTube](https://www.youtube.com/@Type42) · [Bandcamp](https://type42.bandcamp.com/)
- **Concepto original**: Basado en [eepers](https://github.com/tsoding/eepers) por Tsoding
- **Port a C + Phase Shift**: Kripta Studios

---

## 📄 Licencia

Ver [LICENSE.txt](./LICENSE.txt)
