#pragma once

#include <Components/BaseComponent.h>
#include <Core/Wrappers/WrapperAudio.h>
#include <Resources/Sound.h>

namespace Component
{
    struct SavedValues
    {
        bool isSet = false;
        int fadeIn;
        bool isLoop;
        Vector3 cone;
        float volume;
        int minDist;
        int maxDist;
        WrapperAudio::AttenuationMode attenuation;
    };

    class SoundEmitter : public IComponent<SoundEmitter> 
    { 
    private :
        Resources::Sound* m_sound = nullptr;
        bool m_soundExists = false;
        std::string m_soundPath = "";
        WrapperAudio::AttenuationMode m_attenuation = WrapperAudio::AttenuationMode::LINEAR;
        int m_fadeInTime = 1000; /* Time in milliseconds */
        SavedValues saved = {};

        void SendSavedValues();
    public :
        std::string name = "None";
        bool isLooping = false;
    private:
    public :
        ~SoundEmitter();
        SoundEmitter();
        void OnDestroy() override;
        void Start() override;
        void Update() override;
        void EditorUpdate() override;

        void ShowInInspector() override;

        void Play();
        void Stop();
        void Pause();
        void FadeIn(int millisecondsFading);
        void SetLooping(bool value);

        std::ostream& operator<<(std::ostream& os) override;
        std::string GetComponentName() override { return "SoundEmitter"; }

        void ReadComponent(std::fstream& sceneFile) override;
    };
};