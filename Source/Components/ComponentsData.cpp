#include "pch.h"

#include <Components/ComponentsData.h>
#include <Components/MeshComponent.h>
#include <Components/LightComponent.h>
#include <Components/BoxCollider.h>
#include <Components/Rigidbody.h>
#include <Components/ShaderComponent.h>
#include <Components/UI/Canvas.h>
#include <Components/UI/UIImage.h>
#include <Components/UI/Button.h>
#include <Components/UI/Text.h>
#include <Components/UI/RectTransform.h>
#include <Components/ScriptComponent.h>
#include <Components/CameraComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/Animator.h>
#include <Components/TextMesh.h>
#include <Components/SphereCollider.h>
#include <Components/CapsuleCollider.h>
#include <Components/MeshCollider.h>
#include <Components/SoundListener.h>
#include <Components/Constraint.h>
#include <Components/VehicleComponent.h>

#include "Components/SoundEmitter.h"
#include <Components/ParticleSystem.h>


Component::ComponentsData* Component::ComponentsData::m_componentsDatas = nullptr;

Component::ComponentsData::ComponentsData()
{
}

Component::ComponentsData::~ComponentsData()
{

}


void Component::ComponentsData::Destroy()
{
	for (auto&& component : Components)
	{
		delete component;
		component = nullptr;
	}
}

void Component::ComponentsData::NewComponent(BaseComponent* comp)
{
	Components.push_back(comp);
}

void Component::ComponentsData::Initialize()
{
	NewComponent(new Component::MeshComponent());
	NewComponent(new Component::DirectionalLight());
	NewComponent(new Component::PointLight());
	NewComponent(new Component::SpotLight());
	NewComponent(new Component::BoxCollider());
	NewComponent(new Component::SphereCollider());
	NewComponent(new Component::CapsuleCollider());
	NewComponent(new Component::MeshCollider());
	NewComponent(new Component::Rigidbody());
	NewComponent(new Component::ShaderComponent());
	NewComponent(new Component::UI::Canvas());
	NewComponent(new Component::UI::RectTransform());
	NewComponent(new Component::UI::UIImage());
	NewComponent(new Component::UI::Button());
	NewComponent(new Component::UI::Text());
	NewComponent(new Component::CameraComponent());
	NewComponent(new Component::ScriptComponent());
	NewComponent(new Component::SkeletalMeshComponent());
	NewComponent(new Component::Animator());
	NewComponent(new Component::TextMesh());
	NewComponent(new Component::ParticleSystem());
	NewComponent(new Component::SoundListener());
	NewComponent(new Component::SoundEmitter());
	NewComponent(new Component::Constraint());
	NewComponent(new Component::VehicleComponent());
}

void Component::ComponentsData::Delete()
{
	m_componentsDatas->Destroy();
	delete m_componentsDatas;
	m_componentsDatas = nullptr;
}
