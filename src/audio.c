/*
libgo2 - Support library for the ODROID-GO Advance
Copyright (C) 2020 OtherCrashOverride

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
// #include <threads.h>
#include <pthread.h>
#include "audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <alsa/asoundlib.h>
#include <alsa/mixer.h>


#define SOUND_SAMPLES_SIZE  (2048)
#define SOUND_CHANNEL_COUNT 2


typedef struct go2_audio
{
    ALsizei frequency;
    ALCdevice *device;
    ALCcontext *context;
    ALuint source;
    bool isAudioInitialized;
} go2_audio_t;


go2_audio_t* go2_audio_create(int frequency)
{

go2_audio_t* result = malloc(sizeof(*result));
    if (!result)
    {
        printf("malloc failed.\n");
        goto out;
    }

    memset(result, 0, sizeof(*result));


    result->frequency = frequency;

	result->device = alcOpenDevice(NULL);
	if (!result->device)
	{
		printf("alcOpenDevice failed.\n");
		goto err_00;
	}

	result->context = alcCreateContext(result->device, NULL);
	if (!alcMakeContextCurrent(result->context))
	{
		printf("alcMakeContextCurrent failed.\n");
		goto err_01;
	}

	alGenSources((ALuint)1, &result->source);

	alSourcef(result->source, AL_PITCH, 1);
	alSourcef(result->source, AL_GAIN, 1);
	alSource3f(result->source, AL_POSITION, 0, 0, 0);
	alSource3f(result->source, AL_VELOCITY, 0, 0, 0);
	alSourcei(result->source, AL_LOOPING, AL_FALSE);

	//memset(audioBuffer, 0, AUDIOBUFFER_LENGTH * sizeof(short));

	const int BUFFER_COUNT = 4;
	for (int i = 0; i < BUFFER_COUNT; ++i)
	{
		ALuint buffer;
		alGenBuffers((ALuint)1, &buffer);
		alBufferData(buffer, AL_FORMAT_STEREO16, NULL, 0, frequency);
		alSourceQueueBuffers(result->source, 1, &buffer);
	}

	alSourcePlay(result->source);

    result->isAudioInitialized = true;

    // testing
    //uint32_t vol = go2_audio_volume_get(result);
    //printf("audio: vol=%d\n", vol);
    //go2_audio_path_get(result);


    return result;


err_01:
    alcCloseDevice(result->device);

err_00:
    free(result);

out:
    return NULL;
}

void go2_audio_destroy(go2_audio_t* audio)
{
    alDeleteSources(1, &audio->source);
    alcDestroyContext(audio->context);
    alcCloseDevice(audio->device);

    free(audio);
}







void go2_audio_submit(go2_audio_t* audio, const short* data, int frames)
{
      
      
      printf("go2_audio_submit.\n");
      if (!audio || !audio->isAudioInitialized) {
        printf("audio not initialized.\n");
        return;
    }


    /*if (!alcMakeContextCurrent(audio->context))
    {
        printf("alcMakeContextCurrent failed.\n");
        return;
    }*/

    ALint processed = 0;
    ALuint openALBufferID;
    ALint state;
    
    //while(!processed)
    int i=0;
    while( !processed)
    {
        alGetSourceiv(audio->source, AL_BUFFERS_PROCESSED, &processed);
        if (!processed)
        {
            sleep(0);
        }
    }

    alSourceUnqueueBuffers(audio->source, 1, &openALBufferID);

    int dataByteLength = frames * sizeof(short) * SOUND_CHANNEL_COUNT;
    alBufferData(openALBufferID, AL_FORMAT_STEREO16, data, dataByteLength, audio->frequency);
    alSourceQueueBuffers(audio->source, 1, &openALBufferID);
    alGetSourcei(audio->source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING && state != AL_PAUSED )
    {

        alSourcePlay(audio->source);
    }
}



void go2_audio_submit2(go2_audio_t* audio, const short* data, int frames)
{
      
      
      printf("go2_audio_submit2\n");
      if (!audio || !audio->isAudioInitialized) {
        printf("audio not initialized.\n");
        return;
    }


    

    ALint processed = 0;
    ALuint openALBufferID;
    ALint state;
    


    while (!processed) {
        alGetSourceiv(audio->source, AL_BUFFERS_PROCESSED, &processed);
        sched_yield();
    }



    alSourceUnqueueBuffers(audio->source, 1, &openALBufferID);

    int dataByteLength = frames * sizeof(short) * SOUND_CHANNEL_COUNT;
    alBufferData(openALBufferID, AL_FORMAT_STEREO16, data, dataByteLength, audio->frequency);
    alSourceQueueBuffers(audio->source, 1, &openALBufferID);
    alGetSourcei(audio->source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING && state != AL_PAUSED )
    {

        alSourcePlay(audio->source);
    }
}



void go2_audio_submit_max_retry(go2_audio_t* audio, const short* data, int frames, int max_retry)
{
      
      
      printf("go2_audio_submit with max retry.\n");
      if (!audio || !audio->isAudioInitialized) {
        printf("audio not initialized.\n");
        return;
    }


    /*if (!alcMakeContextCurrent(audio->context))
    {
        printf("alcMakeContextCurrent failed.\n");
        return;
    }*/

    ALint processed = 0;
    ALuint openALBufferID;
    ALint state;
    
    //while(!processed)
    int i=0;
    while( !processed && i<max_retry)
    {
        alGetSourceiv(audio->source, AL_BUFFERS_PROCESSED, &processed);
        if (!processed)
        {
            sleep(0);
        }
        i++;        
    }

    if (i>=max_retry){
        return ;
    }

    alSourceUnqueueBuffers(audio->source, 1, &openALBufferID);
    int dataByteLength = frames * sizeof(short) * SOUND_CHANNEL_COUNT;
    alBufferData(openALBufferID, AL_FORMAT_STEREO16, data, dataByteLength, audio->frequency);
    alSourceQueueBuffers(audio->source, 1, &openALBufferID);
    alGetSourcei(audio->source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING && state != AL_PAUSED )
    {
        alSourcePlay(audio->source);
    }
}






uint32_t go2_audio_volume_get(go2_audio_t* audio)
{
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Playback";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    long min;
    long max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
 

    //snd_mixer_selem_set_playback_volume_all(elem, value / 100.0f * max);
    long volume;
    snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &volume);

    snd_mixer_close(handle);

    uint32_t result = volume / (float)max * 100.0f;
    //printf("volume: min=%ld, max=%ld, volume=%ld, result=%d\n", min, max, volume, result);

    return result;
}

void go2_audio_volume_set(go2_audio_t* audio, uint32_t value)
{
    // https://gist.github.com/wolfg1969/3575700

    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Playback";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    long min;
    long max;
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    //printf("volume: min=%ld, max=%ld\n", min, max);

    snd_mixer_selem_set_playback_volume_all(elem, value / 100.0f * max);

    snd_mixer_close(handle);
}

go2_audio_path_t go2_audio_path_get(go2_audio_t* audio)
{
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Playback Path";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    unsigned int value;
    snd_mixer_selem_get_enum_item(elem, SND_MIXER_SCHN_MONO, &value);

    // char name[128];
    // snd_mixer_selem_get_enum_item_name(elem, value, 128, name);
    // printf("audio path: value=%d [%s]\n", value, name);

    snd_mixer_close(handle);

    return (go2_audio_path_t)value;
}

void go2_audio_path_set(go2_audio_t* audio, go2_audio_path_t value)
{
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Playback Path";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_set_enum_item(elem, SND_MIXER_SCHN_MONO, (unsigned int)value);

    snd_mixer_close(handle);
}
