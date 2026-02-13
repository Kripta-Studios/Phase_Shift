#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

// Opaque handle to a Miniaudio sound object
typedef struct AudioSound {
  void *internal;
} AudioSound;

typedef struct AudioMusic {
  void *internal;
} AudioMusic;

// Core Audio System Functions
bool InitAudioSystem(void);
void CloseAudioSystem(void);

// Sound Effects
AudioSound LoadAudioSound(const char *fileName);
void UnloadAudioSound(AudioSound sound);
void PlayAudioSound(AudioSound sound);
void StopAudioSound(AudioSound sound);
void SetAudioSoundVolume(AudioSound sound, float volume);
void SetAudioSoundPitch(AudioSound sound, float pitch);
bool IsAudioSoundValid(AudioSound sound);

// Streaming de Música
AudioMusic LoadAudioMusic(const char *fileName);
void UnloadAudioMusic(AudioMusic music);
void PlayAudioMusic(AudioMusic music);
void StopAudioMusic(AudioMusic music);
void SetAudioMusicVolume(AudioMusic music, float volume);
void UpdateAudioMusic(AudioMusic music); // Por consistencia, aunque el motor
                                         // Miniaudio maneja esto
bool IsAudioMusicValid(AudioMusic music);
bool IsAudioMusicPlaying(AudioMusic music);

#endif // AUDIO_H
