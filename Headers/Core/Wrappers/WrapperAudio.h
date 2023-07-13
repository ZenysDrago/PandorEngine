#pragma once
#include <vector>

struct ma_engine;
struct ma_sound;
struct ma_device;

namespace Core::Wrapper
{
	namespace WrapperAudio
	{
		enum class AttenuationMode
		{
			INVERSE,
			LINEAR,
			EXPONENTIAL,
		};
		
		class PANDOR_API AudioManager
		{
		private : 
			friend class Audio;
			friend class Listener;

			std::vector<class Listener*> listenerList;
			std::deque<class Audio*> soundsList;

			ma_engine* m_engine;
			float m_currentVolume = 1;

			std::mutex mutexAudio;
		public : 

		private : 

		public :
			AudioManager();
			~AudioManager();

			bool PlaySoundFromName(std::string soundName);
			/* Set the volume of the engine (Should be between 0 and 1)*/
			void SetEngineVolume(float soundStrength);
			float GetCurrentVolume() { return m_currentVolume; }

			void AddListener(class Listener* listener);
			class Listener* CreateListener(bool isMainListener = false);
			void SetMainListener(class Listener* listener);
		};
	
		class PANDOR_API Audio
		{
		private:
			ma_sound* m_sound;
			bool m_isSoundLoaded = false;
			bool m_isSpatialized = true;
			bool m_isLooping = false;

			friend AudioManager;
			AudioManager* m_audioManagerRef;
			
			Math::Vector3 m_position;
			Math::Vector3 m_velocity;
			Math::Vector3 m_direction;
			Math::Vector3 m_cone;
			float m_rolloff;
			float m_minGain;
			float m_maxGain;
			float m_dopplerFactor;
			float m_minDistance;
			float m_maxDistance;
			float m_volume;

		private :
			void SoundFadeOut(int millisecondsFading);

		protected: 
			bool LoadSound(AudioManager* audioManager, std::string soundName);
			
		public:
			Audio();
			Audio(AudioManager* audioManager, std::string soundName);
			~Audio();
			
			void SoundFadeIn(int millisecondsFading);

			/* delay in seconds */ 
			void StartSoundDelayed(int delay);
			
			/* delay in seconds */ 
			void StopSoundDelayed(int delay);
			
			void StartSound();
			void StopSound();
			void PauseSound();
#pragma region Setters
			void SetSpatialization(bool shouldBeSpatialized);

			/* Set the position as relative to a listener */
			void SetPositionRelative();

			/* Set the absolute position of the sound */
			void  SetPosition(Math::Vector3 position);

			/* Set sound direction */
			void SetDirection(Math::Vector3 direction);

			/* Set the inner and outerAngle of sound and OuterGain*/
			void SetCone(float innerAngle, float outerAngle, float outerGain);
			void SetCone(Math::Vector3 cone);

			/* Set the velocity of the sound for Doppler effect or other */
			void SetVelocity(Math::Vector3 velocity);

			void SetRolloff(float rollOff);

			void SetMinGain(float minGain);
			void SetMaxGain(float maxGain);
			void SetGain(float minGain, float maxGain);

			void SetMinDistance(float minDistance);
			void SetMaxDistance(float maxDistance);
			void SetDistance(float minDistance, float maxDistance);

			void SetDopplerFactor(float dopplerFactor);

			void SetLooping(const bool loop = false);
			
			void SetAttenuationMode(AttenuationMode mode);

			void SetVolume(float volume);

			void SetAudioManager(AudioManager* audioManagerRef);
#pragma endregion

#pragma region  Getters
			bool GetIsSoundLoaded() const{	return m_isSoundLoaded;}
			bool GetIsSpatialized() const{	return m_isSpatialized;}
			bool GetIsLooping() const{	return m_isLooping;}
			
			Math::Vector3 GetPosition() const{	return m_position;}
			Math::Vector3 GetVelocity() const{	return m_velocity;}
			Math::Vector3 GetDirection() const{	return m_direction;}
			Math::Vector3 GetCone() const{	return m_cone;}
			float GetRolloff() const{	return m_rolloff;}
			float GetMinGain() const{	return m_minGain;}
			float GetMaxGain() const{	return m_maxGain;}
			float GetDopplerFactor() const{	return m_dopplerFactor;}
			float GetMinDistance() const{	return m_minDistance;}
			float GetMaxDistance() const{	return m_maxDistance;}
			float GetVolume() const { return m_volume; }
			bool GetIsPlaying() const;

#pragma endregion 
		};
	
		class Listener
		{
		private :
			friend AudioManager;
			AudioManager* m_audioManagerRef;
			
			bool m_mainListener = false;

			Math::Vector3 m_position;
			Math::Vector3 m_orientation;
			Math::Vector3 m_cone;
			Math::Vector3 m_velocity;
		public :
			Listener();
			~Listener();

			void SetPosition(Math::Vector3 position);
			void SetOrientation(Math::Vector3 orientation);
			void SetVelocity(Math::Vector3 velocity);
			void SetCone(Math::Vector3 cone);
			void SetAsMainListener(bool isMainListener);

			Math::Vector3 GetPosition()		const {return m_position;}
			Math::Vector3 GetOrientation()  const {return m_orientation;}
			Math::Vector3 GetVelocity()		const {return m_velocity;}
			Math::Vector3 GetCone()			const {return m_cone;}
			bool GetIsMainListener()		const {return m_mainListener;}
		};
	}

	
}