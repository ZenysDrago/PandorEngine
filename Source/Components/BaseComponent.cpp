#include "pch.h"

#include <Components/BaseComponent.h>
#include <Components/ComponentsData.h>
#include <Components/Transform.h>
#include <Components/MeshComponent.h>
#include <Components/LightComponent.h>
#include <Components/CameraComponent.h>
#include <Components/ScriptComponent.h>
#include <Components/Rigidbody.h>
#include <Components/BoxCollider.h>
#include <Components/ShaderComponent.h>
#include <Components/ComponentsData.h>
#include <Core/App.h>
#include <Components/SoundListener.h>
#include "Components/SoundEmitter.h"

#include <Core/GameObject.h>
#include <algorithm>


Component::BaseComponent::BaseComponent()
{
}

void Component::BaseComponent::SetGameObject(Core::GameObject* go)
{
	this->gameObject = go;
}

void Component::BaseComponent::RemoveFromGameObject()
{
	if (!gameObject)
		return;
	gameObject->RemoveComponent(this);
}

void Component::BaseComponent::ShouldStart()
{
	if (Core::App::Get().GetGameState() == Core::GameState::Play)
	{
		Start();
	}
}

void Component::BaseComponent::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	while (getline(sceneFile, line))
	{
		if (line == "end")
		{
			break;
		}
	}
}

std::ostream& operator<<(std::ostream& os, Component::BaseComponent& bc) 
{
	bc.operator<<(os);
	return os;
}

std::ostream& Component::BaseComponent::operator<<(std::ostream& os)
{
	return os;
}

Component::BaseComponent* Component::CreateComponent(std::string componentName)
{
	ComponentsData ComponentsDatas = ComponentsData::Get();
	
	auto foundClass = std::find_if(ComponentsDatas.Components.begin(), ComponentsDatas.Components.end(),
		[componentName](Component::BaseComponent* comp) { return comp->GetComponentName() == componentName; });

	if (foundClass != ComponentsDatas.Components.end()) 
	{
		Component::BaseComponent* newInstance = (*foundClass)->Clone();
		return newInstance;
	}
	
	PrintWarning("Doesn't found class %s return nullptr", componentName.c_str());
	return nullptr;	

}
