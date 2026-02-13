#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"
#include "miniaudio.h"
#include <stdio.h>
#include <stdlib.h>

static ma_engine engine;
static bool engineInitialized = false;

// Initialize the Miniaudio engine
bool InitAudioSystem(void) {
  if (engineInitialized)
    return true;

  ma_result result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    fprintf(stderr,
            "AUDIO ERROR: Failed to initialize audio engine. Error code: %d\n",
            result);
    return false;
  }

  engineInitialized = true;
  fprintf(stderr, "AUDIO INFO: Miniaudio engine initialized successfully.\n");
  return true;
}

// Cleanup the Miniaudio engine
void CloseAudioSystem(void) {
  if (!engineInitialized)
    return;
  ma_engine_uninit(&engine);
  engineInitialized = false;
  fprintf(stderr, "AUDIO INFO: Miniaudio engine closed.\n");
}

// Load a sound effect (fully decoded in memory for responsiveness)
AudioSound LoadAudioSound(const char *fileName) {
  AudioSound wrapper = {0};

  if (!engineInitialized) {
    fprintf(stderr,
            "AUDIO WARNING: Cannot load sound '%s', audio system not "
            "initialized.\n",
            fileName);
    return wrapper;
  }

  ma_sound *sound = (ma_sound *)malloc(sizeof(ma_sound));
  if (!sound) {
    fprintf(stderr, "AUDIO ERROR: Out of memory loading sound '%s'\n",
            fileName);
    return wrapper;
  }

  // Usar MA_SOUND_FLAG_DECODE para cargar completamente en memoria (baja
  // latencia para SFX) También se podría usar MA_SOUND_FLAG_NO_PITCH |
  // MA_SOUND_FLAG_NO_SPATIALIZATION para optimización extra si es necesario
  ma_result result = ma_sound_init_from_file(
      &engine, fileName, MA_SOUND_FLAG_DECODE, NULL, NULL, sound);

  if (result != MA_SUCCESS) {
    fprintf(stderr, "AUDIO ERROR: Failed to load sound '%s'. Error code: %d\n",
            fileName, result);
    free(sound);
    return wrapper;
  }

  wrapper.internal = sound;
  return wrapper;
}

void UnloadAudioSound(AudioSound sound) {
  if (sound.internal) {
    ma_sound_uninit((ma_sound *)sound.internal);
    free(sound.internal);
  }
}

void PlayAudioSound(AudioSound sound) {
  if (sound.internal) {
    ma_sound_start((ma_sound *)sound.internal);
  }
}

void StopAudioSound(AudioSound sound) {
  if (sound.internal) {
    ma_sound_stop((ma_sound *)sound.internal);
    ma_sound_seek_to_pcm_frame((ma_sound *)sound.internal, 0); // Reset to start
  }
}

void SetAudioSoundVolume(AudioSound sound, float volume) {
  if (sound.internal) {
    ma_sound_set_volume((ma_sound *)sound.internal, volume);
  }
}

void SetAudioSoundPitch(AudioSound sound, float pitch) {
  if (sound.internal) {
    ma_sound_set_pitch((ma_sound *)sound.internal, pitch);
  }
}

bool IsAudioSoundValid(AudioSound sound) { return sound.internal != NULL; }

// --------------------------------------------------------
// Music implementation (Streamed)
// --------------------------------------------------------

AudioMusic LoadAudioMusic(const char *fileName) {
  AudioMusic wrapper = {0};

  if (!engineInitialized) {
    printf("WARNING: Cannot load music '%s', audio system not initialized.\n",
           fileName);
    return wrapper;
  }

  ma_sound *music = (ma_sound *)malloc(sizeof(ma_sound));
  if (!music) {
    printf("ERROR: Out of memory loading music '%s'\n", fileName);
    return wrapper;
  }

  // Force streaming for music
  ma_result result = ma_sound_init_from_file(
      &engine, fileName, MA_SOUND_FLAG_STREAM, NULL, NULL, music);
  if (result != MA_SUCCESS) {
    printf("ERROR: Failed to load music '%s'. Error code: %d\n", fileName,
           result);
    free(music);
    return wrapper;
  }

  // Loop by default for music? Raylib Music loops by default usually if
  // loop=true. We'll set looping manually if needed, or expose it. For Eepers,
  // ambient music loops.
  ma_sound_set_looping(music, true);

  wrapper.internal = music;
  return wrapper;
}

void UnloadAudioMusic(AudioMusic music) {
  if (music.internal) {
    ma_sound_uninit((ma_sound *)music.internal);
    free(music.internal);
  }
}

void PlayAudioMusic(AudioMusic music) {
  if (music.internal) {
    ma_sound_start((ma_sound *)music.internal);
  }
}

void StopAudioMusic(AudioMusic music) {
  if (music.internal) {
    ma_sound_stop((ma_sound *)music.internal);
    ma_sound_seek_to_pcm_frame((ma_sound *)music.internal, 0);
  }
}

void SetAudioMusicVolume(AudioMusic music, float volume) {
  if (music.internal) {
    ma_sound_set_volume((ma_sound *)music.internal, volume);
  }
}

void UpdateAudioMusic(AudioMusic music) {
  // Miniaudio engine handles streaming in background thread.
  // No explicit update needed per frame!
  (void)music;
}

bool IsAudioMusicValid(AudioMusic music) { return music.internal != NULL; }

bool IsAudioMusicPlaying(AudioMusic music) {
  if (music.internal) {
    return ma_sound_is_playing((ma_sound *)music.internal);
  }
  return false;
}
