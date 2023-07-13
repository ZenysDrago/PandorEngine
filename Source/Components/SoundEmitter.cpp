#include "pch.h"
#include <Components/SoundEmitter.h>
#include <Resources/Sound.h>
#include <Core/App.h>

#include "Core/GameObject.h"

using namespace Component;


Component::SoundEmitter::~SoundEmitter()
{
    if (m_sound)
        m_sound->StopSound();
}

Component::SoundEmitter::SoundEmitter()
{
    Resources::ResourcesManager* resourcesManager = Core::App::Get().resourcesManager;
    for (Resources::Sound* resource : resourcesManager->GetAllResourcesOfType<Sound>())
    {
        Core::App::Get().resourcesManager->GetOrLoad<Resources::Sound>(resource->GetPath());
    }
}

void Component::SoundEmitter::OnDestroy()
{
    if(m_sound)
        m_sound->StopSound();
}

void SoundEmitter::Start()
{
    IComponent<SoundEmitter>::Start();
    if (m_sound)
        m_sound->StopSound();
}

void SoundEmitter::Update()
{
    IComponent<SoundEmitter>::Update();

    if(m_sound && m_sound->HasBeenSent())
    {
        m_sound->SetPosition(this->gameObject->transform->GetWorldPosition());
    }
}

void Component::SoundEmitter::EditorUpdate()
{
    if (m_sound && m_sound->HasBeenSent())
    {
        m_sound->SetPosition(this->gameObject->transform->GetWorldPosition());
    }
}

void SoundEmitter::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
	{
        if (line == "nullptr")
        {
            while (getline(sceneFile, line) && line != "end") {}
            return;
        }

		m_sound = Core::App().Get().resourcesManager->GetOrLoad<Resources::Sound>(line);
		m_soundPath = line;
		name = m_sound->GetName();
	}

	saved.isSet = true;

	if (getline(sceneFile, line) && line != "end")
	{
		saved.fadeIn = std::atoi(line.c_str());
		m_fadeInTime = saved.fadeIn;
	}

	if (getline(sceneFile, line) && line != "end")
	{
		saved.isLoop = (bool)std::atoi(line.c_str());
		isLooping = saved.isLoop;
	}

	if (getline(sceneFile, line) && line != "end")
	{
		saved.attenuation = (WrapperAudio::AttenuationMode)std::atoi(line.c_str());
		m_attenuation = saved.attenuation;
	}

	if (getline(sceneFile, line) && line != "end")
	{
		saved.cone = Math::ParseVector3(line);
	}

	if (getline(sceneFile, line) && line != "end")
	{
		saved.volume = std::stof(line.c_str());
	}

	if (getline(sceneFile, line) && line != "end")
	{
		saved.minDist = std::atoi(line.c_str());
	}

	if (getline(sceneFile, line) && line != "end")
	{
		saved.maxDist = std::atoi(line.c_str());
	}

	while (getline(sceneFile, line) && line != "end") {}

}

bool SoundPopup(std::string& name , std::string& path)
{
    bool out = false;
    if (WrapperUI::BeginPopup("Sounds", WindowFlags::AlwaysVerticalScrollbar))
    {
        static TextFilter filter;
        filter.Draw("##");
        for (Resources::Sound* sound : Core::App::Get().resourcesManager->GetAllResourcesOfType<Resources::Sound>())
        {
            if (!filter.PassFilter( sound->GetName().c_str()))
                continue;
            if (WrapperUI::Selectable( sound->GetName().c_str(), false, SelectableFlags::SpanAllColumns)) {
                name = sound->GetName();
                path = sound->GetPath();
                out = true;
                WrapperUI::CloseCurrentPopup();
            }
        }
        WrapperUI::EndPopup();
    }
    return out;
}

void SoundEmitter::ShowInInspector()
{
    IComponent<SoundEmitter>::ShowInInspector();

    m_soundExists = Core::App::Get().resourcesManager->Find<Resources::Sound>(m_soundPath);

    if (saved.isSet)

        SendSavedValues();

    if (WrapperUI::Button(name.c_str(), Math::Vector2(200, 0)))
        WrapperUI::OpenPopup("Sounds");

    if (SoundPopup(name, m_soundPath))
    {
        m_sound = Core::App::Get().resourcesManager->GetOrLoad<Resources::Sound>(m_soundPath);
        return;
    }

    if (!m_soundExists)
        return;
    
    WrapperUI::NewLine();

    if (WrapperUI::Button("Play"))
        Play();

    WrapperUI::SameLine();
    if (WrapperUI::Button("Stop"))
        Stop();

    if (WrapperUI::Button("FadeIn"))
        FadeIn(m_fadeInTime);

    WrapperUI::Text("Attenuation : %d", m_attenuation);

    float vol = m_sound->GetVolume();
    float minDistance = m_sound->GetMinDistance();
    float maxDistance = m_sound->GetMaxDistance();
    Vector3 cone = m_sound->GetCone();
    
    WrapperUI::Text("Is playing : %s", m_sound->GetIsPlaying() ? "Yes" : "No");
    
    if (WrapperUI::SliderFloat("volume", &vol, 0, 1))
        m_sound->SetVolume(vol);
        
    WrapperUI::SliderInt("FadeInTime(ms)" , &m_fadeInTime, 500,10000);

    if (WrapperUI::Checkbox("Sound looping", &isLooping))
        SetLooping(isLooping);

    if(WrapperUI::SliderFloat("min dist sound", &minDistance, 0,50))
        m_sound->SetMinDistance(minDistance);

    if(WrapperUI::SliderFloat("max dist sound", &maxDistance, -1,100))
        m_sound->SetMaxDistance(maxDistance);

    if(WrapperUI::SliderFloat3("Cone" , &cone.x, -50,50))
        m_sound->SetCone(cone);
}

void Component::SoundEmitter::Play()
{
    m_sound->StartSound();
    m_sound->SetAttenuationMode(m_attenuation);
}

void Component::SoundEmitter::Stop()
{
    m_sound->StopSound();
}

void Component::SoundEmitter::Pause()
{
    m_sound->PauseSound();
}

void Component::SoundEmitter::FadeIn(int millisecondsFading)
{
    m_sound->SetAttenuationMode(m_attenuation);
    m_sound->SoundFadeIn(millisecondsFading);
}

void Component::SoundEmitter::SetLooping(bool value)
{
    m_sound->SetLooping(value);
}

std::ostream& SoundEmitter::operator<<(std::ostream& os)
{
    if (!m_sound)
    {
        os << "nullptr" << "\n";
        return os;
    }

    os << m_sound->GetPath() << "\n";
    os << m_fadeInTime << "\n";
    os << isLooping << "\n";
    os << (int)m_attenuation << "\n";
    os << m_sound->GetCone() << "\n";
    os << m_sound->GetVolume() << "\n";
    os << m_sound->GetMinDistance() << "\n";
    os << m_sound->GetMaxDistance() << "\n";
    return os;
}

 void Component::SoundEmitter::SendSavedValues()
{

    m_sound->SetAttenuationMode(saved.attenuation);
    m_sound->SetMaxDistance((float)saved.maxDist);
    m_sound->SetMinDistance((float)saved.minDist);
    m_sound->SetVolume(saved.volume);
    if (saved.cone != Math::Vector3::Zero())
        m_sound->SetCone(saved.cone);
    m_sound->SetLooping(saved.isLoop);

    saved = SavedValues();
}