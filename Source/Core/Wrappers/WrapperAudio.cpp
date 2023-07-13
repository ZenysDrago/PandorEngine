#include "pch.h"
#include <Core/Wrappers/WrapperAudio.h>
#include <Debug/Log.h>

#define MINIAUDIO_IMPLEMENTATION
#include <MiniAudio/miniaudio.h>

#include "Core/App.h"

using namespace Core::Wrapper;

WrapperAudio::AudioManager::AudioManager()
{
	ma_result result;
	ma_engine* engine = new ma_engine;

	m_engine = engine;
	result = ma_engine_init(NULL, m_engine);
	if (result != MA_SUCCESS)
		PrintError("Failed to initialize audio engine");

}

WrapperAudio::AudioManager::~AudioManager()
{
	for (auto listener : listenerList)
	{
		if (listener)
			delete listener;
		listener = nullptr;
	}
	listenerList.clear();

	soundsList.clear();

	ma_engine_uninit(m_engine);
	delete m_engine;
	m_engine = nullptr;
}

bool WrapperAudio::AudioManager::PlaySoundFromName(std::string soundName)
{
	std::string ext = soundName.substr(soundName.find_last_of(".")+1);
	if (ext != "wav" && ext != "MP3" && ext != "FLAC" && ext != "mp3" && ext != "OGG")
	{
		PrintError("Not Supported format for audio file : %s", soundName.c_str());
		return false;
	}

	ma_result result;
	result = ma_engine_play_sound(m_engine, soundName.c_str(), NULL);
	if (result != MA_SUCCESS)
	{
		PrintError("Failed play sound");
		return true;
	}

	return false;
}

void Core::Wrapper::WrapperAudio::AudioManager::SetEngineVolume(float soundStrength)
{
	if (soundStrength > 1)
		PrintWarning("The volume should be between 0 and 1 this sound is too high");
	if (soundStrength < 0)
		soundStrength = 0;

	ma_result result;
	result = ma_engine_set_volume(m_engine, soundStrength);
	if (result != MA_SUCCESS)
	{
		PrintError("Error when updating volume of engine");
		return;
	}

	m_currentVolume = soundStrength;
}

void WrapperAudio::AudioManager::AddListener(Listener* listener)
{
	if(listener->GetIsMainListener())
	{
		SetMainListener(listener);
	}
	listenerList.push_back(listener);
}

WrapperAudio::Listener* WrapperAudio::AudioManager::CreateListener(bool isMainListener)
{
	Listener* listener = new Listener();
	listener->SetAsMainListener(isMainListener);
	return listener;
}

void WrapperAudio::AudioManager::SetMainListener(Listener* listener)
{
	ma_engine_listener_set_position(m_engine, 0, listener->m_position.x, listener->m_position.y, listener->m_position.z);
	ma_engine_listener_set_direction(m_engine, 0, listener->m_orientation.x, listener->m_orientation.y, listener->m_orientation.z);
	ma_engine_listener_set_cone(m_engine, 0, listener->m_cone.x, listener->m_cone.y, listener->m_cone.z);
	ma_engine_listener_set_velocity(m_engine, 0, listener->m_velocity.x, listener->m_velocity.y, listener->m_velocity.z);
	for (Listener* listen : listenerList)
	{
		if(listen != listener)
			listen->m_mainListener = false;
	}
}

/* ================================================================================== */
/*								     Audio class									  */
/* ================================================================================== */

WrapperAudio::Audio::Audio(AudioManager* audioManager, std::string soundName)
{
	if (audioManager)
	{
		m_audioManagerRef = audioManager;
		m_audioManagerRef->soundsList.push_back(this);
	}
	
	m_sound = new ma_sound;
	ma_result result;
	result = ma_sound_init_from_file(m_audioManagerRef->m_engine, soundName.c_str(), 0, NULL, NULL, m_sound);
	if (result != MA_SUCCESS)
	{
		PrintError("Couldn't load init sound of name : %s", soundName.c_str());
		return;
	}
	SetVolume(1);
	m_isSoundLoaded = true;
}

WrapperAudio::Audio::Audio()
{
	m_sound = new ma_sound;
}
               
WrapperAudio::Audio::~Audio()
{
	ma_sound_uninit(m_sound);
	delete m_sound;
	m_sound = nullptr;
}

bool WrapperAudio::Audio::LoadSound(AudioManager* audioManager, std::string soundName)
{
	ma_result result;
	result = ma_sound_init_from_file(audioManager->m_engine, soundName.c_str(), 0, NULL, NULL, m_sound);
	if (result != MA_SUCCESS)
	{
		PrintError("Couldn't load init sound of name : %s", soundName.c_str());
		return false;
	}
	m_audioManagerRef = Core::App::Get().audioManager;
	while (!m_audioManagerRef->mutexAudio.try_lock())
		m_audioManagerRef->soundsList.push_back(this);
	m_audioManagerRef->mutexAudio.unlock();
	SetSpatialization(true);
	SetVolume(1);
	m_isSoundLoaded = true;
	return m_isSoundLoaded;
}

void Core::Wrapper::WrapperAudio::Audio::SetSpatialization(bool shouldBeSpatialized)
{
	ma_sound_set_spatialization_enabled(m_sound, shouldBeSpatialized);
}

void WrapperAudio::Audio::SetPositionRelative()
{
	ma_sound_set_positioning(m_sound, ma_positioning_relative);
}

void WrapperAudio::Audio::SetPosition(Math::Vector3 position)
{
	m_position = position;
	ma_sound_set_position(m_sound,m_position.x , m_position.y ,m_position.z);
}

void WrapperAudio::Audio::SetDirection(Math::Vector3 direction)
{
	m_direction = direction;
	ma_sound_set_direction(m_sound,m_direction.x , m_direction.y ,m_direction.z);
}

void WrapperAudio::Audio::SetCone(float innerAngle, float outerAngle, float outerGain)
{
	m_cone = {innerAngle , outerAngle, outerGain};
	ma_sound_set_cone(m_sound,innerAngle , outerAngle ,outerGain);
}

void WrapperAudio::Audio::SetCone(Math::Vector3 cone)
{
	SetCone(cone.x, cone.y, cone.z);
}

void WrapperAudio::Audio::SetVelocity(Math::Vector3 velocity)
{
	m_velocity = velocity;
	ma_sound_set_velocity(m_sound,m_velocity.x , m_velocity.y ,m_velocity.z);
}

void WrapperAudio::Audio::SetRolloff(float rollOff)
{
	m_rolloff = rollOff;
	ma_sound_set_rolloff(m_sound, m_rolloff);
}

void WrapperAudio::Audio::SetMinGain(float minGain)
{
	m_minGain = minGain;
	ma_sound_set_min_gain(m_sound, m_minGain);
}

void WrapperAudio::Audio::SetMaxGain(float maxGain)
{
	m_maxGain = maxGain;
	ma_sound_set_max_gain(m_sound, m_maxGain);
}

void WrapperAudio::Audio::SetGain(float minGain, float maxGain)
{
	SetMinGain(minGain);
	SetMaxGain(maxGain);
}
 
void WrapperAudio::Audio::SetMinDistance(float minDistance)
{
	m_minDistance = minDistance;
	ma_sound_set_min_distance(m_sound, m_minDistance);
}

void WrapperAudio::Audio::SetMaxDistance(float maxDistance)
{
	m_maxDistance = maxDistance;
	ma_sound_set_max_distance(m_sound, m_maxDistance);
}

void WrapperAudio::Audio::SetDistance(float minDistance, float maxDistance)
{
	SetMinDistance(minDistance);
	SetMaxDistance(maxDistance);
}

void WrapperAudio::Audio::SetDopplerFactor(float dopplerFactor)
{
	m_dopplerFactor = dopplerFactor;
	ma_sound_set_doppler_factor(m_sound, dopplerFactor);
}

void WrapperAudio::Audio::SoundFadeIn(int millisecondsFading)
{
	if (!ma_sound_is_playing(m_sound))
	{
		ma_sound_set_fade_in_milliseconds(m_sound, 0, 1, millisecondsFading);
		StartSound();
	}
}

void WrapperAudio::Audio::SoundFadeOut(int millisecondsFading)
{
	ma_sound_set_fade_in_milliseconds(m_sound, -1, 0, millisecondsFading);
	StopSoundDelayed((int)(millisecondsFading * 0.001f));
}

void WrapperAudio::Audio::StartSoundDelayed(int delay)
{
	ma_sound_set_start_time_in_pcm_frames(m_sound, ma_engine_get_time(m_audioManagerRef->m_engine) + (ma_engine_get_sample_rate(m_audioManagerRef->m_engine) * delay));
	StartSound();
}

void WrapperAudio::Audio::StopSoundDelayed(int delay)
{
	ma_sound_set_stop_time_in_pcm_frames(m_sound, ma_engine_get_time(m_audioManagerRef->m_engine) + (ma_engine_get_sample_rate(m_audioManagerRef->m_engine) * delay));
}

void WrapperAudio::Audio::SetAttenuationMode(AttenuationMode mode)
{
	switch(mode)
	{
	case AttenuationMode::LINEAR:
		ma_sound_set_attenuation_model(m_sound,ma_attenuation_model_linear);
		break;
	case AttenuationMode::EXPONENTIAL:
		ma_sound_set_attenuation_model(m_sound,ma_attenuation_model_exponential);
		break;
	case AttenuationMode::INVERSE:
		ma_sound_set_attenuation_model(m_sound,ma_attenuation_model_inverse);
		break;
	default:
		ma_sound_set_attenuation_model(m_sound,ma_attenuation_model_none);
		break;
	}
}

void Core::Wrapper::WrapperAudio::Audio::SetVolume(float volume)
{
	m_volume = volume;
	ma_sound_set_volume(m_sound, m_volume);
}

void Core::Wrapper::WrapperAudio::Audio::SetAudioManager(AudioManager* audioManagerRef)
{
	/* If audioManager already set remove this sound from the list */
	if (m_audioManagerRef)
	{
		auto it = std::find(m_audioManagerRef->soundsList.begin(), m_audioManagerRef->soundsList.end(), this);
		if (it != m_audioManagerRef->soundsList.end())
			m_audioManagerRef->soundsList.erase(it);
	}

	m_audioManagerRef = audioManagerRef;
	m_audioManagerRef->soundsList.push_back(this);
}

bool Core::Wrapper::WrapperAudio::Audio::GetIsPlaying() const
{ 
	if (m_sound)
		return ma_sound_is_playing(m_sound);
	else
		return false;
}

void WrapperAudio::Audio::StartSound()
{
	ma_sound_set_volume(m_sound, m_volume);
	ma_result result = ma_sound_start(m_sound);
	if (result != MA_SUCCESS)
		PrintError("Couldn't start sound");	
}

void WrapperAudio::Audio::StopSound()
{
	if (m_sound) {
		ma_sound_stop(m_sound);
		ma_sound_seek_to_pcm_frame(m_sound, 0);
	}
}

void Core::Wrapper::WrapperAudio::Audio::PauseSound()
{
	ma_sound_stop(m_sound);
}

void WrapperAudio::Audio::SetLooping(const bool loop)
{
	m_isLooping = loop;
	ma_data_source_set_looping(m_sound->pDataSource,loop);
}

/* ================================================================================== */
/*								     Listener class									  */
/* ================================================================================== */

WrapperAudio::Listener::Listener()
{
	m_audioManagerRef = Core::App::Get().audioManager;
	m_audioManagerRef->AddListener(this);
}

WrapperAudio::Listener::~Listener()
{
	
}

void WrapperAudio::Listener::SetPosition(Math::Vector3 position)
{
	m_position = position;
	if(m_mainListener)
		ma_engine_listener_set_position(m_audioManagerRef->m_engine, 0, m_position.x, m_position.y, m_position.z);
}

void WrapperAudio::Listener::SetOrientation(Math::Vector3 orientation)
{
	m_orientation = orientation;
	if (m_mainListener)
		ma_engine_listener_set_direction(m_audioManagerRef->m_engine, 0, m_orientation.x, m_orientation.y, m_orientation.z);
}

void WrapperAudio::Listener::SetVelocity(Math::Vector3 velocity)
{
	m_velocity = velocity;
	if (m_mainListener)
		ma_engine_listener_set_velocity(m_audioManagerRef->m_engine, 0, m_velocity.x, m_velocity.y, m_velocity.z);
}

void WrapperAudio::Listener::SetCone(Math::Vector3 cone)
{
	m_cone = cone;
	if (m_mainListener)
		ma_engine_listener_set_cone(m_audioManagerRef->m_engine, 0, m_cone.x, m_cone.y, m_cone.z);
}

void WrapperAudio::Listener::SetAsMainListener(bool isMainListener)
{
	if (isMainListener)
	{
		m_mainListener = isMainListener;
		m_audioManagerRef->SetMainListener(this);
		return;
	}
	else if (m_mainListener)
		m_audioManagerRef->SetMainListener(m_audioManagerRef->listenerList[0]);

	m_mainListener = isMainListener;
}


