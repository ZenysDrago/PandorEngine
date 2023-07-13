#include "pch.h"
#include <Components/SoundListener.h>

#include "Components/Rigidbody.h"
#include "Core/GameObject.h"

using namespace Component;
using namespace Core::Wrapper;

Component::SoundListener::SoundListener() 
{
    m_listener = new WrapperAudio::Listener();
    if (gameObject)
    {
        m_listener->SetPosition(gameObject->transform->GetWorldPosition());
        m_listener->SetOrientation(gameObject->transform->GetWorldRotation().ToEuler());
    }
    m_listener->SetCone({ 360 * DEG2RAD , 360 * DEG2RAD, 0 });
}

void SoundListener::Start()
{
    BaseComponent::Start();
    std::vector<Rigidbody*> rbodies = gameObject->GetComponentsInChildren<Rigidbody>();
    if(!rbodies.empty())
        m_rbody = rbodies[0];
}

void SoundListener::Update()
{
    BaseComponent::Update();
    m_listener->SetPosition(gameObject->transform->GetWorldPosition());
    m_listener->SetOrientation(gameObject->transform->GetWorldRotation().ToEuler());
    if (m_rbody)
        m_listener->SetVelocity(m_rbody->GetVelocity());
}

void SoundListener::EditorUpdate()
{
    m_listener->SetPosition(gameObject->transform->GetWorldPosition());
    m_listener->SetOrientation(gameObject->transform->GetWorldRotation().ToEuler());
    if (m_rbody)
        m_listener->SetVelocity(m_rbody->GetVelocity());
}

void SoundListener::ShowInInspector()
{
    BaseComponent::ShowInInspector();

    bool isMain = m_listener->GetIsMainListener();
    if (WrapperUI::Checkbox("Set as main listener", &isMain))
        m_listener->SetAsMainListener(isMain);

    Math::Vector3 cone = m_listener->GetCone();
    if(WrapperUI::SliderFloat3("Cone", &cone.x, -50,50))
        m_listener->SetCone(cone);
   
}

BaseComponent* Component::SoundListener::Clone() const
{
    return new SoundListener();
}

void SoundListener::ReadComponent(std::fstream& sceneFile)
{
    std::string line;
    
    m_listener = new WrapperAudio::Listener();
    if (getline(sceneFile, line) && line != "end")
        m_listener->SetAsMainListener(std::stoi(line));
	if (getline(sceneFile, line) && line != "end")
        m_listener->SetCone(ParseVector3(line));

	while (getline(sceneFile, line) && line != "end") {}
}

std::ostream& Component::SoundListener::operator<<(std::ostream& os)
{
	os << m_listener->GetIsMainListener() << '\n';
	os << m_listener->GetCone() << '\n';
    return os;
}