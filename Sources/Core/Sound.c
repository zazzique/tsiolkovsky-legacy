#define NOSOUND

#include "Common.h"

#ifndef NOSOUND
#include "fmod.h"
#include "fmod_errors.h"
#endif

#include "Files.h"
#include "FastMath.h"
#include "Vector.h"
#include "Sound.h"


#ifndef NOSOUND
FMOD_SYSTEM *fmod_system;

typedef struct _Sound
{
	char filename[32];
	int valid;
	U32 flags;
	FMOD_SOUND *sound;
	float max_dist;
	
} Sound;

typedef struct _SoundSource
{
	int valid;
	int active;
	FMOD_CHANNEL *channel;
	float frequency;
	
} SoundSource;


Sound sounds[MAX_SOUNDS];
SoundSource sound_sources[MAX_SOUND_SOURCES];

FMOD_CHANNEL *ambient_channel[2];

int abient_sound_index[2];
float ambient_target_volume[2];
float ambient_channel_frequency[2];

int ambient_active_channel_index;
float ambient_channels_cross_factor;

float sound_time_scale;


void Sound_ErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK)
	{
		LogPrint("Error: FMOD error '%s'\n", FMOD_ErrorString(result));
	}
}

#endif

int Sound_GetSoundIndexByName(char *name)
{
#ifndef NOSOUND
	for (int i = 0; i < MAX_SOUNDS; i++)
		if (sounds[i].valid)
			if (sounds[i].filename[0] == name[0])
			{
				int result = strcmp(sounds[i].filename, name);
				if (result == 0)
					return i;
			}
#endif
	return -1;
}

void Sound_Init()
{
#ifndef NOSOUND
	FMOD_RESULT result = FMOD_OK;
	
	unsigned int version = 0;
	
	result = FMOD_System_Create(&fmod_system);
	Sound_ErrorCheck(result);
	
	result = FMOD_System_GetVersion(fmod_system, &version);
	Sound_ErrorCheck(result);
	
	if (version < FMOD_VERSION)
    {
		LogPrint("Error: Old version of FMOD!\n");
        return;
    }
	
	result = FMOD_System_Init(fmod_system, 64, FMOD_INIT_NORMAL, NULL);
	Sound_ErrorCheck(result);
	
	result = FMOD_System_Set3DSettings(fmod_system, 1.0f, 128.0f, 1.0f);
	Sound_ErrorCheck(result);

	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		sounds[i].valid = FALSE;
	}
	
	for (int i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		sound_sources[i].valid = FALSE;
		sound_sources[i].active = FALSE;
	}
	
	abient_sound_index[0] = -1;
	abient_sound_index[1] = -1;
	
	ambient_target_volume[0] = 1.0f;
	ambient_target_volume[1] = 1.0f;
	
	ambient_active_channel_index = 0;
	
	ambient_channels_cross_factor = 0.0f;

	sound_time_scale = 1.0f;
#endif
} 


int Sound_AddSound(char *filename, U32 flags, float max_radius)
{
	int index = Sound_GetSoundIndexByName(filename);
	if (index >= 0)
		return index;
	
	index = -1;

#ifndef NOSOUND
	
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		if (!sounds[i].valid)
		{
			index = i;
			break;
		}
	}
	
	if (index < 0)
	{
		LogPrint("Error: too many sounds!\n");
		return -1;
	}
		
	char file_path[256];
	
	sounds[index].flags = flags;
	
	FMOD_MODE fmod_flags = FMOD_SOFTWARE | FMOD_3D_LINEARROLLOFF | FMOD_LOWMEM | FMOD_OPENMEMORY;
	
	if (flags & SOUND_FLAG_STREAM)
		fmod_flags |= FMOD_CREATESTREAM;
	else
		fmod_flags |= FMOD_CREATECOMPRESSEDSAMPLE;
	
	if (flags & SOUND_FLAG_3D)
		fmod_flags |= FMOD_3D;
	else
		fmod_flags |= FMOD_2D;
	
	if (flags & SOUND_FLAG_LOOPED)
		fmod_flags |= FMOD_LOOP_NORMAL;
	else
		fmod_flags |= FMOD_LOOP_OFF;
	
	FileHandler sound_file;

	if (!Files_OpenFile(&sound_file, filename))
		return -1;

	U8 *sound_data = NULL;
	I32 sound_size;

	Files_GetData(&sound_file, (void **)&sound_data, &sound_size);

	if (sound_data == NULL)
		return -1;

	FMOD_CREATESOUNDEXINFO sound_info = {0};
	sound_info.cbsize = sizeof(sound_info);
	sound_info.length = sound_size;

	FMOD_RESULT result = FMOD_System_CreateSound(fmod_system, (const char *)sound_data, fmod_flags, &sound_info, &sounds[index].sound);
	Sound_ErrorCheck(result);
		
	if (result == FMOD_OK)
	{
		if (flags & SOUND_FLAG_3D)
		{
			result = FMOD_Sound_Set3DMinMaxDistance(sounds[index].sound, max_radius * 0.1f, max_radius); // TODO: store radiuses
			Sound_ErrorCheck(result);
		}
		
		sounds[index].max_dist = max_radius;
		
		sounds[index].valid = TRUE;

		strncpy(sounds[index].filename, filename, 31);
	}
	else
	{
		index = -1;
	}
	
#endif
	return index;
}


float Sound_GetSoundMaxDist(int sound_index)
{
#ifndef NOSOUND
	if (sound_index < 0 || sound_index >= MAX_SOUNDS)
		return 0.0f;
	
	if (!sounds[sound_index].valid)
		return 0.0f;
	
	return sounds[sound_index].max_dist;
#else
	return 0.0f;
#endif
}


void Sound_ReleaseSound(int sound_index)
{
#ifndef NOSOUND
	if (sound_index < 0 || sound_index >= MAX_SOUNDS)
		return;
	 
	if (!sounds[sound_index].valid)
		return;
	
	// TODO: stop all sources which are using this sound
	
	FMOD_RESULT result = FMOD_OK;
	
	result = FMOD_Sound_Release(sounds[sound_index].sound);
	Sound_ErrorCheck(result);
	
	sounds[sound_index].valid = FALSE;
#endif
}


int Sound_AddSource()
{
	int index = -1;
#ifndef NOSOUND
	for (int i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		if (!sound_sources[i].valid)
		{
			index = i;
			break;
		}
	}
	
	if (index < 0)
	{
		LogPrint("Error: too many sound sources!\n");
		return -1;
	}
	
	sound_sources[index].valid = TRUE;
	sound_sources[index].active = FALSE;

#endif
	return index;
}


void Sound_UpdateSource(int source_index, Vector3D *pos)
{
#ifndef NOSOUND
	if (source_index < 0 || source_index >= MAX_SOUND_SOURCES)
		return;
	
	if (!sound_sources[source_index].valid)
		return;

	if (!sound_sources[source_index].active)
		return;

	FMOD_VECTOR fmod_pos, fmod_vel;
	
	if (pos == NULL)
	{
		fmod_pos.x = 0.0f;
		fmod_pos.y = 0.0f;
		fmod_pos.z = 0.0f;
	}
	else
	{
		fmod_pos.x = pos->x;
		fmod_pos.y = pos->y;
		fmod_pos.z = -pos->z;
	}

	fmod_vel.x = 0.0f;
	fmod_vel.y = 0.0f;
	fmod_vel.z = 0.0f;
		
	FMOD_RESULT result = FMOD_Channel_Set3DAttributes(sound_sources[source_index].channel, &fmod_pos, &fmod_vel);
	Sound_ErrorCheck(result);
#endif
}


void Sound_ReleaseSource(int index)
{
#ifndef NOSOUND
	if (index < 0 || index >= MAX_SOUND_SOURCES)
		return;
	 
	if (!sound_sources[index].valid)
		return;
	 
	Sound_StopSource(index);
	 
	sound_sources[index].valid = FALSE;
#endif
}


void Sound_Play(int sound_index, int source_index, Vector3D *pos, Vector3D *velocity, int is_head_relative, float volume)
{
#ifndef NOSOUND
	if (source_index < 0 || source_index >= MAX_SOUND_SOURCES)
		return;
	
	if (sound_index < 0 || sound_index >= MAX_SOUNDS)
		return;
	
	if (!sounds[sound_index].valid)
		return;
	
	if (!sound_sources[source_index].valid)
		return;
	
	FMOD_RESULT result = FMOD_OK;
	FMOD_VECTOR fmod_pos, fmod_vel;

	//if (sound_sources[source_index].active) // TODO: test
	//	Sound_StopSource(source_index);
	
	result = FMOD_System_PlaySound(fmod_system, FMOD_CHANNEL_FREE, sounds[sound_index].sound, TRUE, &sound_sources[source_index].channel);
	Sound_ErrorCheck(result);

	result = FMOD_Channel_GetFrequency(sound_sources[source_index].channel, &sound_sources[source_index].frequency);
	Sound_ErrorCheck(result);
	
	if (pos == NULL || is_head_relative)
	{
		fmod_pos.x = 0.0f;
		fmod_pos.y = 0.0f;
		fmod_pos.z = 0.0f;
	}
	else
	{
		fmod_pos.x = pos->x;
		fmod_pos.y = pos->y;
		fmod_pos.z = -pos->z;
	}

	if (velocity == NULL || is_head_relative)
	{
		fmod_vel.x = 0.0f;
		fmod_vel.y = 0.0f;
		fmod_vel.z = 0.0f;
	}
	else
	{
		fmod_vel.x = velocity->x;
		fmod_vel.y = velocity->y;
		fmod_vel.z = -velocity->z;
	}
		
	if (sounds[sound_index].flags & SOUND_FLAG_3D)
	{
		if (is_head_relative)
		{
			result = FMOD_Channel_SetMode(sound_sources[source_index].channel, FMOD_3D_HEADRELATIVE);
			Sound_ErrorCheck(result);
		}
		
		result = FMOD_Channel_Set3DAttributes(sound_sources[source_index].channel, &fmod_pos, &fmod_vel);
		Sound_ErrorCheck(result);
	}
	
	result = FMOD_Channel_SetVolume(sound_sources[source_index].channel, volume);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetPaused(sound_sources[source_index].channel, FALSE);
	Sound_ErrorCheck(result);

	sound_sources[source_index].active = TRUE;
#endif
}


void Sound_StopSource(int source_index)
{
#ifndef NOSOUND
	if (source_index < 0 || source_index >= MAX_SOUND_SOURCES)
		return;
	
	if (!sound_sources[source_index].valid)
		return;

	if (!sound_sources[source_index].active)
		return;
	
	FMOD_RESULT result = FMOD_OK;
	
	result = FMOD_Channel_Stop(sound_sources[source_index].channel);
	Sound_ErrorCheck(result);

	sound_sources[source_index].active = FALSE;
#endif
}


void Sound_StopAll()
{
#ifndef NOSOUND
	for (int i = 0; i < MAX_SOUND_SOURCES; i++)
		Sound_StopSource(i);
#endif
}


void Sound_SetAmbientNow(int sound_index, float volume)
{
#ifndef NOSOUND
	if (sound_index < 0 || sound_index >= MAX_SOUNDS)
		return;
	
	if (!sounds[sound_index].valid)
		return;
	
	if ((sound_index == abient_sound_index[0]) && (ambient_active_channel_index == 0))
		return;
	
	if ((sound_index == abient_sound_index[1]) && (ambient_active_channel_index == 1))
		return;
	
	FMOD_RESULT result = FMOD_OK;
		
	Sound_StopAmbients();
	
	result = FMOD_System_PlaySound(fmod_system, FMOD_CHANNEL_FREE, sounds[sound_index].sound, TRUE, &ambient_channel[0]);
	Sound_ErrorCheck(result);

	result = FMOD_Channel_GetFrequency(ambient_channel[0], &ambient_channel_frequency[0]);
	Sound_ErrorCheck(result);

	result = FMOD_Channel_SetFrequency(ambient_channel[0], ambient_channel_frequency[0] * sound_time_scale);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetPriority(ambient_channel[0], 0);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetVolume(ambient_channel[0], volume);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetPaused(ambient_channel[0], FALSE);
	Sound_ErrorCheck(result);
	
	abient_sound_index[0] = sound_index;
	ambient_active_channel_index = 0;
	ambient_channels_cross_factor = 0.0f;
	ambient_target_volume[0] = volume;
#endif
}


void Sound_SetAmbient(int sound_index, float volume)
{
#ifndef NOSOUND
	if (sound_index < 0 || sound_index >= MAX_SOUNDS)
		return;
	
	if (!sounds[sound_index].valid)
		return;
	
	if (sound_index == abient_sound_index[0])
	{
		ambient_active_channel_index = 0;
		return;
	}
	
	if (sound_index == abient_sound_index[1])
	{
		ambient_active_channel_index = 1;
		return;
	}
	
	FMOD_RESULT result = FMOD_OK;
	
	if (ambient_active_channel_index == 0)
		ambient_active_channel_index = 1;
	else
		ambient_active_channel_index = 0;
	
	if (abient_sound_index[0] < 0)
		ambient_active_channel_index = 0;
	
	if (abient_sound_index[1] < 0)
		ambient_active_channel_index = 1;
	
	
	if (abient_sound_index[ambient_active_channel_index] >= 0 && abient_sound_index[ambient_active_channel_index] < MAX_SOUNDS)
	{
		if (sounds[abient_sound_index[ambient_active_channel_index]].valid)
		{
			result = FMOD_Channel_Stop(ambient_channel[ambient_active_channel_index]);
			Sound_ErrorCheck(result);
		}
		
		abient_sound_index[ambient_active_channel_index] = -1;
	}
	
	
	result = FMOD_System_PlaySound(fmod_system, FMOD_CHANNEL_FREE, sounds[sound_index].sound, TRUE, &ambient_channel[ambient_active_channel_index]);
	Sound_ErrorCheck(result);

	result = FMOD_Channel_GetFrequency(ambient_channel[ambient_active_channel_index], &ambient_channel_frequency[ambient_active_channel_index]);
	Sound_ErrorCheck(result);

	result = FMOD_Channel_SetFrequency(ambient_channel[ambient_active_channel_index], ambient_channel_frequency[ambient_active_channel_index] * sound_time_scale);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetPriority(ambient_channel[ambient_active_channel_index], 0);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetVolume(ambient_channel[ambient_active_channel_index], volume);
	Sound_ErrorCheck(result);
	
	result = FMOD_Channel_SetPaused(ambient_channel[ambient_active_channel_index], FALSE);
	Sound_ErrorCheck(result);
	
	abient_sound_index[ambient_active_channel_index] = sound_index;
	ambient_target_volume[ambient_active_channel_index] = volume;
#endif
}


void Sound_StopAmbients()
{
#ifndef NOSOUND
	FMOD_RESULT result = FMOD_OK;
	
	for (int i = 0; i < 2; i++)
	{
		if (abient_sound_index[i] >= 0 && abient_sound_index[i] < MAX_SOUNDS)
		{
			if (sounds[abient_sound_index[i]].valid)
			{
				result = FMOD_Channel_Stop(ambient_channel[i]);
				Sound_ErrorCheck(result);
			}
		
			abient_sound_index[i] = -1;
		}
	}
#endif
}


void Sound_UpdateListener(Vector3D *pos, Vector3D *forward, Vector3D *up, Vector3D *velocity, float delta_time, float time_scale, float volume)
{
#ifndef NOSOUND
	FMOD_RESULT result = FMOD_OK;

	sound_time_scale = time_scale;

	for (int i = 0; i < MAX_SOUND_SOURCES; i ++)
	{
		if (!sound_sources[i].valid)
			continue;

		if (!sound_sources[i].active)
			continue;

		FMOD_BOOL is_playing;

		result = FMOD_Channel_IsPlaying(sound_sources[i].channel, &is_playing);
		if ((result == FMOD_OK) && is_playing)
		{
			result = FMOD_Channel_SetFrequency(sound_sources[i].channel,  sound_sources[i].frequency * sound_time_scale);
			Sound_ErrorCheck(result);
		}
		else
		{
			sound_sources[i].active = FALSE;
		}
	}

	static float prev_sound_volume = 1.0f;
	
	if (prev_sound_volume != volume)
	{
		FMOD_CHANNELGROUP *channelgroup;
		result = FMOD_System_GetMasterChannelGroup(fmod_system, &channelgroup);
		
		result = FMOD_ChannelGroup_SetVolume(channelgroup, volume);
		
		prev_sound_volume = volume;
	}

	if (ambient_active_channel_index == 0)
	{
		DeltaFunc(&ambient_channels_cross_factor, 0.0f, delta_time * 0.5f);
	}
	else
	{
		DeltaFunc(&ambient_channels_cross_factor, 1.0f, delta_time * 0.5f);
	}
	
	float channel_volume[2] = { (1.0f - ambient_channels_cross_factor) * ambient_target_volume[0], ambient_channels_cross_factor * ambient_target_volume[1] };
	
	for (int i = 0; i < 2; i++)
	{
		if (abient_sound_index[i] >= 0 && abient_sound_index[i] < MAX_SOUNDS)
		{
			if (channel_volume[i] == 0.0f)
			{
				if (ambient_active_channel_index != i)
				{
					result = FMOD_Channel_Stop(ambient_channel[i]);
					Sound_ErrorCheck(result);
				
					abient_sound_index[i] = -1;
				}
			}
			else
			{
				result = FMOD_Channel_SetFrequency(ambient_channel[i], ambient_channel_frequency[i] * sound_time_scale);
				Sound_ErrorCheck(result);
				result = FMOD_Channel_SetVolume(ambient_channel[i], channel_volume[i]);
				Sound_ErrorCheck(result);
			}
		}
	}
	
	FMOD_VECTOR listener_pos, listener_vel, listener_forward, listener_up;
	
	if (pos == NULL)
	{
		listener_pos.x = 0.0f;
		listener_pos.y = 0.0f;
		listener_pos.z = 0.0f;
	}
	else
	{
		listener_pos.x = pos->x;
		listener_pos.y = pos->y;
		listener_pos.z = -pos->z;
	}

	if (velocity == NULL)
	{
		listener_vel.x = 0.0f;
		listener_vel.y = 0.0f;
		listener_vel.z = 0.0f;
	}
	else
	{
		listener_vel.x = velocity->x;
		listener_vel.y = velocity->y;
		listener_vel.z = -velocity->z;
	}
	
	if (forward == NULL || up == NULL)
	{
		listener_forward.x = 0.0f;
		listener_forward.y = 0.0f;
		listener_forward.z = 1.0f;
		
		listener_up.x = 0.0f;
		listener_up.y = 1.0f;
		listener_up.z = 0.0f;
	}
	else
	{
		listener_forward.x = forward->x;
		listener_forward.y = forward->y;
		listener_forward.z = -forward->z;
		
		listener_up.x = up->x;
		listener_up.y = up->y;
		listener_up.z = -up->z;
	}

	result = FMOD_System_Set3DListenerAttributes(fmod_system, 0, &listener_pos, &listener_vel, &listener_forward, &listener_up);
	Sound_ErrorCheck(result);
	
	result = FMOD_System_Update(fmod_system);
	Sound_ErrorCheck(result);
#endif
}


void Sound_Release()
{
#ifndef NOSOUND
	Sound_StopAll();
	
	Sound_StopAmbients();
	
	for (int i = 0; i < MAX_SOUND_SOURCES; i ++)
		Sound_ReleaseSource(i);
	
	for (int i = 0; i < MAX_SOUNDS; i ++)
		Sound_ReleaseSound(i);
	
	FMOD_RESULT result = FMOD_OK;
	
	result = FMOD_System_Release(fmod_system);
	Sound_ErrorCheck(result);
#endif
}

