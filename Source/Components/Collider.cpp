#include "pch.h"

#include <Components\Collider.h>
#include <Components\Rigidbody.h>
#include <Core/GameObject.h>
#include <Core/App.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Material.h>
#include <Resources/PhysicMaterial.h>


void Component::Collider::Initialize()
{
	p_material = Resources::ResourcesManager::Get()->Find<Resources::Material>("ColliderMaterial");
	if (!p_material)
	{
		p_material = new Resources::Material("ColliderMaterial", ResourcesType::Material);
		p_material->SetShader(Resources::ResourcesManager::Get()->GetUnlitShader());
		p_material->SetDiffuse({ 0, 1, 0, 1 });
		Resources::ResourcesManager::Get()->Add("ColliderMaterial", p_material);
	}
}

void Component::Collider::Start()
{
	p_rigidbody = gameObject->GetComponentShared<Component::Rigidbody>();
	if (!p_rigidbody.lock().get())
	{
		p_rigidbody = gameObject->GetComponentSharedInParent<Component::Rigidbody>();
	}
}

void Component::Collider::Update()
{
}

void Component::Collider::SetOffset(const Vector3& offset)
{
	if (!p_rigidbody.expired() && p_rigidbody.lock().get()->gameObject != gameObject) {
		Component::Rigidbody* rb = p_rigidbody.lock().get();
		Vector3 worldPos = gameObject->GetParent()->transform->GetModelMatrix().GetTransposed() * Vector4(gameObject->transform->GetLocalPosition() + offset, 1.0f);
		auto mat = rb->gameObject->transform->GetModelMatrix().CreateInverse(true);
		auto offset = mat * Vector4(worldPos, 1.f);
		Math::Quaternion rotOffset = rb->gameObject->transform->GetWorldRotation().GetInverse() * gameObject->transform->GetWorldRotation();
		SetLocal(offset, rotOffset);
	}
	else if (!p_rigidbody.expired() && p_rigidbody.lock().get()->gameObject == gameObject)
	{
		SetLocal(offset, Quaternion::Identity());
	}
}

void Component::Collider::GameUpdate()
{
}

void Component::Collider::OnDestroy()
{
	Core::App::Get().physic->RemoveCollider(this);
	Destroy(this);
}

void Component::Collider::Draw()
{

}

void Component::Collider::ShowInInspector()
{
	if (WrapperUI::Checkbox("Is Trigger", &p_isTrigger))
		SetTrigger(p_isTrigger);
	if (WrapperUI::Button("Physic Material"))
	{
		WrapperUI::OpenPopup("PhysicMaterialPopup");
	}
	if (auto phm = Resources::ResourcesManager::Get()->ResourcePopup<Resources::PhysicMaterial>("PhysicMaterialPopup"))
	{
		this->p_physicalMaterial = Resources::ResourcesManager::Get()->GetOrLoad<Resources::PhysicMaterial>(phm->GetPath());
	}
	WrapperUI::SameLine();
	if (this->p_physicalMaterial)
		WrapperUI::TextUnformatted(p_physicalMaterial->GetName().c_str());
	else
		WrapperUI::TextUnformatted("Default");
}

std::ostream& Component::Collider::operator<<(std::ostream& os)
{
	if (p_physicalMaterial)
		os << p_physicalMaterial->GetPath() << "\n";
	else
		os << "nullptr" << "\n";
	os << p_isTrigger << '\n';
	return os;
}

void Component::Collider::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
	{
		if (line != "nullptr")
			p_physicalMaterial = ResourcesManager::Get()->GetOrLoad<Resources::PhysicMaterial>(line);
	}

	if (getline(sceneFile, line) && line != "end")
		p_isTrigger = std::stoi(line);
}

