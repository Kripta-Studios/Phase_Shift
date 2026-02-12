# 🌀 Phase Shift — Quantum Puzzle Game

<p align=center>
  <img src="./assets/icon.png">
</p>

> *"Tu conciencia está entrelazada cuánticamente con un acelerador de partículas..."*

**Phase Shift** es un juego de puzzles 2D basado en mecánicas cuánticas reales. Eres el **Sujeto 44**, un científico atrapado en un experimento cuántico fallido. Tu conciencia existe en múltiples fases de la realidad simultáneamente. Completa **15 niveles** de puzzles cuánticos antes de que tu coherencia cuántica colapse permanentemente.

Desarrollado por **Kripta Studios** · Motor: [Raylib](https://www.raylib.com/) · Lenguaje: C99

---

## 🎮 Controles

| Tecla | Acción |
|-------|--------|
| `W/A/S/D` o `↑←↓→` | Movimiento (basado en turnos) |
| `Z` | Cambiar de fase cuántica |
| `ESPACIO` | Activar Superposición / Crear Eco |
| `X` o `SHIFT IZQ` | Plantar bomba |
| `T` o `.` | Esperar (pasar turno sin moverse) |
| `E` | Entrelazarse con un Colapsor cercano |
| `H` | Enciclopedia cuántica |
| `ENTER` | Avanzar diálogos / Reintentar nivel |
| `ESC` | Menú de pausa |
| `=` / `-` | Zoom in / out |
| `F11` | Pantalla completa |
| `F5` | Saltar nivel (debug) |

---

## 🔬 Mecánicas Cuánticas

### 1. Sistema de Fases (`Z`)

El jugador cambia entre fases de la realidad. Cada fase atraviesa muros del color **opuesto**:

| Fase | Atraviesa | Bloqueado por |
|------|-----------|---------------|
| 🔴 Roja | Muros AZULES | Muros ROJOS |
| 🔵 Azul | Muros ROJOS | Muros AZULES |
| 🟢 Verde | Muros AMARILLOS | Muros VERDES |
| 🟡 Amarilla | Muros VERDES | Muros AMARILLOS |

> Las fases Verde y Amarilla se desbloquean con el item **Desbloqueo de Fase**.

### 2. Superposición Cuántica (`ESPACIO`)

Activa la superposición para crear un **Eco Cuántico**:

1. **Posiciónate** donde quieras que actúe tu Eco (ej. sobre un botón)
2. **Pulsa ESPACIO** para empezar a grabar
3. **Usa [T]** para esperar turnos sin moverte
4. **Muévete** hacia tu objetivo mientras la grabación se registra
5. El **Eco** aparece y repite tus acciones en bucle

### 3. Detectores Cuánticos

Láseres que solo detectan UNA fase:
- **Detector ROJO** te detecta si estás en fase ROJA
- **Detector AZUL** te detecta si estás en fase AZUL
- Detección = pérdida de coherencia + bloqueo de fase

### 4. Portales Cuánticos

Celdas brillantes que te teletransportan:
- Cada portal tiene una **fase** asignada
- Debes estar en la **misma fase** que el portal para usarlo
- Los portales están enlazados en **parejas**

### 5. Túneles Cuánticos

Zonas púrpuras que te permiten atravesar muros:
- **50%** de probabilidad de éxito
- Si fallas: turnos atrapado + pérdida de coherencia
- Solo funciona al activar superposición en la zona

### 6. Entrelazamiento (`E`)

Púlsa `E` cerca de un Colapsor (enemigo) para entrelazarte:
- Tu movimiento se transfiere al enemigo entrelazado
- El enemigo se mueve **igual que tú**
- Útil para posicionar enemigos sobre botones

### 7. Coherencia Cuántica (Barra de Vida)

| Acción | Efecto |
|--------|--------|
| Decaimiento natural | -1% cada 5 turnos |
| Zona de decoherencia | -2% extra por turno |
| Detección | -15% |
| Partícula de coherencia | +5% |
| Checkpoint | Restauración completa |
| Coherencia = 0% | **Muerte** |

---

## 🗺️ Guía de los 15 Niveles

### Nivel 1 — INTERFERENCIA
**Concepto:** Introducción al cambio de fase

Muros rojos y azules forman un patrón de interferencia. Cambia de fase con `Z` para atravesar los muros del color opuesto. Recoge la llave en el centro, abre la puerta y llega a la salida.

**Solución:**
1. Empieza en fase ROJA → atraviesa muros AZULES
2. Cambia a AZUL con `Z` → atraviesa muros ROJOS
3. Recoge la llave en (10, centro)
4. Ve a la puerta (col 17) y sal por (col 18)

---

### Nivel 2 — ZIGZAG DE FASE
**Concepto:** Cambio de fase obligatorio en zigzag

Tres barreras verticales de fase (ROJA → AZUL → ROJA) con aperturas en extremos opuestos fuerzan un recorrido en zigzag.

**Solución:**
1. Fase AZUL → cruza barrera ROJA por apertura inferior (col 5)
2. Fase ROJA → sube y cruza barrera AZUL por apertura superior (col 10)
3. Fase AZUL → cruza barrera ROJA final por abajo (col 15)
4. Recoge la llave en (12, 3), abre la puerta y sal

---

### Nivel 3 — PARADOJA TEMPORAL
**Concepto:** Uso de Ecos Cuánticos para abrir barreras

Tres cámaras separadas por barricadas. Cada barricada se abre con un botón. Necesitas Ecos para mantener botones pulsados.

**Solución:**
1. Ve al botón ROJO (4, centro-3)
2. Pulsa `ESPACIO` para iniciar grabación del Eco
3. Espera con `T` varios turnos
4. Corre hacia el segundo botón. Tu Eco mantiene el primero
5. Repite para el tercer botón si es necesario
6. Alternativa: usa bombas (`X`) para romper barricadas

---

### Nivel 4 — COMPUERTA HADAMARD
**Concepto:** Dos botones simultáneos con Eco

Forma de "H" con muros de fase. Dos botones (ROJO arriba, AZUL abajo) deben pulsarse a la vez para abrir la barricada.

**Solución:**
1. Recoge la llave en (10, 3)
2. Ve al botón de ARRIBA (15, 4)
3. Pulsa `ESPACIO` para crear Eco
4. Espera `T` 3-4 turnos
5. Corre al botón de ABAJO (15, rows-5)
6. Tu Eco mantiene el botón superior → barricada se abre
7. Abre la puerta con la llave y sal

---

### Nivel 5 — GROVER (EXTREMO)
**Concepto:** Búsqueda cuántica en laberinto con guardias

Laberinto denso con muros verticales, detectores y dos guardias. La llave está en el rincón más peligroso.

**Solución:**
1. Cambia de fase para cruzar aperturas de color (ROJA en col 5, AZUL en col 10...)
2. Evita detectores estando en la fase OPUESTA a la del detector
3. Recoge la bomba en (2, rows-2) para emergencias
4. Los guardias persiguen: muévete rápido o usa bombas
5. La llave está en (22, 2). La salida en (22, centro)

---

### Nivel 6 — TELETRANSPORTE (EXTREMO)
**Concepto:** Túneles cuánticos entre zonas aisladas

Tres zonas separadas por muros dobles. Solo los túneles cuánticos (zonas púrpuras) conectan las zonas.

**Solución:**
1. En la zona 1, ve al Túnel (7, 4) → activa superposición
2. **Cuidado**: un guardia espera en la zona 2
3. Recoge la llave en (14, rows-2) de la zona 2
4. Usa el Túnel 2 (16, 6) para llegar a la zona 3
5. El botón rojo en (14, 2) abre barricadas entre zonas 2-3
6. Abre la puerta con la llave y llega a la salida

---

### Nivel 7 — CORRECCIÓN DE ERRORES (EXTREMO)
**Concepto:** Tres corredores con diferentes peligros

Tres corredores paralelos, cada uno con un tipo de "ruido" cuántico diferente.

**Solución:**
1. **Corredor SUPERIOR**: Muros de fase rojos + guardia al final. Cambia a AZUL para cruzar, luego evita al guardia
2. **Corredor MEDIO**: Guardia + detector azul. Usa fase ROJA para evitar detector
3. **Corredor INFERIOR**: Detectores cruzados rojo/azul. Cambia fase rápidamente para esquivar
4. El botón en el corredor superior (cols-6, 3) abre la barrera final
5. Recoge coherencia en (6, 8) y (18, 14) por el camino

---

### Nivel 8 — BOSS RUSH
**Concepto:** Todas las mecánicas combinadas en 3 zonas

Mapa grande con tres zonas: laberinto de fase, kill box con detectores, y escape con túnel.

**Solución:**
1. **ZONA 1**: Cruza el laberinto de fase alternando con `Z`, esquiva al guardia
2. **ZONA 2**: Usa Eco en un botón (16, centro-4), corre al otro (16, centro+4). Los detectores cubren el centro: cambia fase constantemente
3. Ambos botones abren la barricada a la zona 3
4. **ZONA 3**: Usa el túnel (24, 6) para atravesar el muro final
5. Recoge la llave en (28, 7), abre puerta en (col 27), elimina al guardia si es necesario con bombas

---

### Nivel 9 — COMPUERTA TOFFOLI
**Concepto:** Desbloqueo de fase verde + botones duales

La salida está bloqueada por muros VERDES. Dos botones deben pulsarse simultáneamente.

**Solución:**
1. Recoge el **Desbloqueo de Fase Verde** en (12, 8)
2. Ve al primer botón (4, 4)
3. Pulsa `ESPACIO` para crear Eco
4. Espera `T` turnos y corre al segundo botón (4, rows-4)
5. Ambos botones pulsados → muros VERDES en col 18 se abren
6. Cambia a fase VERDE con `Z` si hace falta
7. Cruza y llega a la salida en (22, centro)

---

### Nivel 10 — TELEPORTACIÓN
**Concepto:** Portales cuánticos con fase requerida

🔑 **CLAVE**: Cada portal requiere estar en su **misma fase** para activarse.

Tres islas separadas por muros. Los portales cuánticos son la ÚNICA forma de moverse entre islas.

**Solución:**
1. Empiezas en la Isla 1 (cols 1-7), fase ROJA
2. El portal ROJO está en (4, centro). Pisa el portal → te teletransporta a la Isla 2
3. En la Isla 2 (cols 9-17), recoge la **LLAVE** en (13, rows-4)
4. Cambia a fase **AZUL** con `Z`
5. El portal AZUL está en (13, 4). Pisa → te teletransporta a la Isla 3
6. En la Isla 3 (cols 19-26), abre la **PUERTA** en col 24 con la llave
7. Llega a la **SALIDA** en (25, centro)

> Si necesitas volver, el portal VERDE en (22, centro) te devuelve a la Isla 1 (cambia a fase VERDE primero).

---

### Nivel 11 — DEUTSCH-JOZSA
**Concepto:** Oráculo y detector con espejos

Un oráculo esconde una propiedad. Usa el detector azul y los espejos para interrogarlo.

**Solución:**
1. Activa el botón ROJO para eliminar obstrucciones de fase
2. Usa los espejos para redirigir el haz del detector hacia el oráculo
3. El oráculo revela su estado al ser "medido"
4. Navega hasta la salida usando la información obtenida

---

### Nivel 12 — SHOR: BÚSQUEDA DE PERÍODO
**Concepto:** Patrones repetitivos con guardias entrelazables

Tres guardias se mueven en ciclo. Usa el entrelazamiento para romper sus patrones.

**Solución:**
1. Observa el patrón de movimiento de los guardias
2. Acércate a un guardia y pulsa `E` para entrelazarte
3. Tu movimiento se transfiere al guardia entrelazado
4. Posiciona guardias fuera de tu camino
5. Llega a la salida en el borde derecho

---

### Nivel 13 — ENTRELAZAMIENTO MASIVO
**Concepto:** Tres botones con múltiples entidades

Tres interruptores deben activarse. Entrelaza múltiples entidades para coordinar.

**Solución:**
1. Entrelázate con los colapsores usando `E`
2. Muévete para posicionar a los colapsores sobre botones
3. Los tres botones pulsados simultáneamente abren la salida
4. El movimiento de uno afecta a todos los entrelazados

---

### Nivel 14 — INTERFEROMETRÍA CUÁNTICA
**Concepto:** Dos caminos, dos activadores con superposición

La dualidad es tu herramienta. Dos botones en caminos separados.

**Solución:**
1. Usa superposición `ESPACIO` para crear un Eco
2. El Eco recorre un camino mientras tú recorres el otro
3. Ambos activadores pulsados a la vez abren la salida
4. Colápsa la función de onda en el objetivo

---

### Nivel 15 — SUPREMACÍA CUÁNTICA
**Concepto:** Prueba final — todas las mecánicas

Combina entrelazamiento, superposición, fases, zonas de decoherencia y oráculos.

**Solución:**
1. Despeja el camino usando bombas y cambio de fase
2. Evita las zonas de decoherencia (pérdida rápida de coherencia)
3. Usa entrelazamiento y ecos para resolver puzzles de botones
4. Consulta al oráculo final para abrir la salida

---

## 🔨 Compilación

### Dependencias

- **GCC** (compilador C99)
- **Raylib** 5.0+ (incluido como .dll en Windows)
- **GNU Make** (opcional)

### Compilar

```bash
# Con Make (recomendado)
make clean
make
./eepers_v2.exe

# Manualmente (Windows MinGW)
gcc -O3 -Wall -Wno-missing-braces -std=c99 -I. -Isrc -L. -o eepers_v2.exe \
  src/main.c src/utils.c src/logic.c src/render.c src/levels.c \
  src/menus.c src/persistence.c src/atmosphere.c src/quantum.c \
  -lraylib -lopengl32 -lgdi32 -lwinmm
```

---

## 🧬 Arquitectura del Código

| Archivo | Propósito |
|---------|-----------|
| `src/main.c` | Bucle principal, gestión de estados |
| `src/common.h` | Structs, enums, constantes globales |
| `src/utils.c/h` | Mapa, colisiones, paleta, pathfinding |
| `src/logic.c/h` | Turnos, IA, física cuántica, ecos |
| `src/render.c/h` | Renderizado visual, HUD, efectos |
| `src/levels.c/h` | Definición y carga de 15 niveles |
| `src/menus.c/h` | Menú principal y pausa |
| `src/persistence.c/h` | Guardado/cargado de progreso |
| `src/atmosphere.c/h` | Estrellas, átomos decorativos |
| `src/quantum.c/h` | Qubits, puertas cuánticas, portales |

### Estructuras de Datos

| Struct | Propósito |
|--------|-----------|
| `GameState` | Estado global: mapa, jugador, colapsores, items |
| `PlayerState` | Posición, fase, coherencia, eco, qubits |
| `ColapsarState` | Enemigos con IA, pathfinding BFS |
| `QuantumPortal` | Portales con fase y enlace |
| `QuantumDetector` | Detectores con dirección y fase |
| `PressureButton` | Botones de presión por fase |
| `DialogSystem` | Diálogos con páginas |

---

## 🎵 Créditos

- **Motor**: [Raylib](https://www.raylib.com/) by Ramon Santamaria
- **Música**: Type 42 — [SoundCloud](https://soundcloud.com/type42) · [YouTube](https://www.youtube.com/@Type42) · [Bandcamp](https://type42.bandcamp.com/)
- **Concepto original**: Basado en [eepers](https://github.com/tsoding/eepers) por Tsoding
- **Port a C + Phase Shift**: Kripta Studios

---

## 📄 Licencia

Ver [LICENSE.txt](./LICENSE.txt)