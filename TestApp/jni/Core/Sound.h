
#ifndef _SOUND_H_
#define _SOUND_H_

#define MAX_SOUNDS 256
#define MAX_SOUND_SOURCES 1024


#define SOUND_FLAG_DEFAULT          0x00000000
#define SOUND_FLAG_LOOPED           0x00000001
#define SOUND_FLAG_3D               0x00000002
#define SOUND_FLAG_STREAM           0x00000004


void Sound_Init();

int Sound_GetSoundIndexByName(char *name);
int  Sound_AddSound(char *filename, U32 flags, float max_radius);
float Sound_GetSoundMaxDist(int sound_index);
void Sound_ReleaseSound(int sound_index);

int  Sound_AddSource();
void Sound_UpdateSource(int source_index, Vector3D *pos);
void Sound_ReleaseSource(int source_index);

void Sound_Play(int sound_index, int source_index, Vector3D *pos, Vector3D *velocity, int is_head_relative, float volume);
void Sound_StopSource(int source_index);
void Sound_StopAll();

void Sound_UpdateListener(Vector3D *pos, Vector3D *forward, Vector3D *up, Vector3D *velocity, float delta_time, float time_scale, float volume);

void Sound_Release();

void Sound_SetAmbient(int sound_index, float volume);
void Sound_SetAmbientNow(int sound_index, float volume);
void Sound_StopAmbients();


#endif /* _SOUND_H_ */
