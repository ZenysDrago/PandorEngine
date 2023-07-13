#include "pch.h"
#include <Resources/Skeleton.h>
#include <Resources/Animation.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/Rigidbody.h>
#include <Components/SphereCollider.h>
#include <Components/Constraint.h>
#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>

#pragma region Bone

Resources::Bone::Bone() : GameObject()
{
}

Resources::Bone::~Bone()
{
	if (m_skeleton && m_skeleton->RootBone == this)
	{
		for (auto& meshes : m_skeleton->m_skeletalMeshes)
			meshes->SetSkeleton(nullptr);
		for (auto& bone : m_skeleton->Bones)
		{
			if (bone != m_skeleton->RootBone)
			{
				bone->m_skeleton = nullptr;
			}
		}
		delete m_skeleton;
		m_skeleton = nullptr;
	}
}

void Resources::Bone::ShowInInspector()
{
	if (WrapperUI::CollapsingHeader((m_name + " " + std::to_string(Id)).c_str()))
	{
		transform->ShowInInspector();
		WrapperUI::TreePush(m_name.c_str());
		for (auto child : m_childrens)
		{
			dynamic_cast<Bone*>(child)->ShowInInspector();
		}
		WrapperUI::TreePop();
	}
}

void Resources::Bone::UpdateBone(Animation* anim, float time)
{
	Math::Vector3 Position = DefaultPosition;
	Math::Quaternion Rotation;
	anim->GetAnimAtFrame(Id, time, Position, Rotation);

	transform->SetLocalPosition(Position);
	transform->SetLocalRotation(DefaultRotation * Rotation);

	for (auto& child : m_childrens)
	{
		if (auto bone = dynamic_cast<Bone*>(child))
			bone->UpdateBone(anim, time);
	}

}

void Resources::Bone::CrossUpdate(float CrossFadeDuration, float Time, class Animation* currentAnimation, class Animation* nextAnimation)
{
	// Define the duration and offset of the transition
	float transitionDuration = CrossFadeDuration; // in seconds
	float transitionOffset = 0.0f; // in seconds

	// Calculate the weight of each animation during the transition
	float t = Time / transitionDuration;
	t = std::fmaxf(0.0f, std::fminf(1.0f, t)); // clamp t to the range [0, 1]


	Math::Vector3 Position = DefaultPosition;
	Math::Quaternion Rotation;
	currentAnimation->GetAnimAtFrame(Id, Time, Position, Rotation);
	Rotation = DefaultRotation * Rotation;

	Math::Vector3 Position2 = DefaultPosition;
	Math::Quaternion Rotation2;
	nextAnimation->GetAnimAtFrame(Id, 0, Position2, Rotation2);
	Rotation2 = DefaultRotation * Rotation2;

	// Interpolate between the keyframes of the two animations
	Vector3 position = Vector3::Lerp(Position, Position2, t);
	Quaternion rotation = Quaternion::SLerp(Rotation, Rotation2, t);

	// Apply the blended animation to your object
	transform->SetLocalPosition(position);
	transform->SetLocalRotation(rotation);

	for (auto child : m_childrens)
		if (auto bone = dynamic_cast<Bone*>(child))
			bone->CrossUpdate(CrossFadeDuration, Time, currentAnimation, nextAnimation);
}

void Resources::Bone::SetDefault()
{
	transform->SetLocalPosition(DefaultPosition);
	transform->SetLocalRotation(DefaultRotation);

	for (auto& child : m_childrens)
	{
		if (auto bone = dynamic_cast<Bone*>(child))
		{
			bone->SetDefault();
		}
	}
}

std::vector<Resources::Bone*> Resources::Bone::GetAllBones()
{
	std::vector<Bone*> out;
	for (auto&& c : m_childrens)
	{
		auto vec = dynamic_cast<Bone*>(c)->GetAllBones();
		out.insert(out.end(), vec.begin(), vec.end());
	}
	out.push_back(this);
	return out;
}

Math::Matrix4 Resources::Bone::GetBoneMatrix()
{
	auto result = DefaultMatrix * transform->GetModelMatrix();
	return result;
}

void Resources::Bone::DrawDebug()
{
	for (auto& child : m_childrens)
	{
		if (auto boneChild = dynamic_cast<Bone*>(child)) {
			auto pos = boneChild->transform->GetWorldPosition();
			auto pos2 = transform->GetWorldPosition();
			WrapperRHI::Line::Get().Color = 1;
			WrapperRHI::Line::Get().Draw(pos, pos2);
			boneChild->DrawDebug();
		}
	}
}

Resources::Bone* Resources::Bone::Clone()
{
	auto bone = GetScene()->CreateBoneObject();
	bone->Id = Id;
	bone->SetName(m_name);
	bone->DefaultMatrix = DefaultMatrix;
	bone->DefaultPosition = DefaultPosition;
	bone->DefaultRotation = DefaultRotation;
	if (bone->transform)
		delete bone->transform;
	bone->transform = new Component::Transform(*transform);
	bone->transform->SetGameObject(bone);

	for (auto& child : m_childrens)
	{
		if (auto childBone = dynamic_cast<Bone*>(child)) {
			Resources::Bone* clone = childBone->Clone();
			bone->AddChildren(clone);
		}
		
	}

	return bone;
}

Core::GameObject* Resources::Bone::Instantiate(GameObject* parent)
{
	Bone* newInstance = Core::SceneManager::Get()->GetCurrentScene()->CreateBoneObject();
	newInstance->SetName(this->m_name + "(Clone)");

	for (auto& component : m_components) {
		Component::BaseComponent* newComponent = component->Clone();
		newInstance->AddComponent(newComponent);
	}
	for (auto& component : newInstance->m_components)
	{
		if (Core::App::Get().GetGameState() == Core::GameState::Play)
		{
			component->Start();
		}
	}

	// Recursively instantiate child GameObjects
	for (auto& child : m_childrens) {
		GameObject* newChild = child->Instantiate();
		newInstance->AddChildren(newChild);
	}

	if (!parent)
		Core::SceneManager::Get()->GetCurrentNode()->AddChildren(newInstance);
	else
		parent->AddChildren(newInstance);
	return newInstance;
}

void Resources::Bone::RemoveFromParent()
{
	if (m_skeleton && m_skeleton->RootBone != this)
	{
		m_skeleton->RootBone->RemoveFromParent();
	}
	else
	{
		GameObject::RemoveFromParent();
	}
}

void Resources::Bone::ReadGameObject(std::fstream& sceneFile, Core::Scene* scene)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		Id = std::stoi(line);
	if (getline(sceneFile, line) && line != "end")
		DefaultPosition = ParseVector3(line);
	if (getline(sceneFile, line) && line != "end")
		DefaultRotation = ParseQuaternion(line);
	if (getline(sceneFile, line) && line != "end")
		DefaultMatrix = ParseMatrix4(line);

	std::string skelPath;
	if (getline(sceneFile, line) && line != "end")
		skelPath = line;

	getline(sceneFile, line);
	getline(sceneFile, line);

	GameObject::ReadGameObject(sceneFile, scene);

	if (m_name.find("Hips"))
	{
		m_skeleton = new Skeleton(skelPath, ResourcesType::Skeleton);
		m_skeleton->Bones = GetAllBones();
		for (auto& bone : m_skeleton->Bones)
		{
			bone->m_skeleton = m_skeleton;
		}
		m_skeleton->RootBone = this;
		m_skeleton->SortBones();
	}
}

std::ostream& Resources::Bone::operator<<(std::ostream& os)
{
	os << "===" << "\n";
	os << "Bone" << "\n";
	os << Id << '\n';
	os << DefaultPosition << '\n';
	os << DefaultRotation << '\n';
	os << DefaultMatrix << '\n';
	os << m_skeleton->GetPath() << '\n';

	GameObject::operator<<(os);
	return os;
}

#pragma endregion

Resources::Skeleton::~Skeleton()
{
	if (!RootBone->GetParent())
	{
		delete RootBone;
		RootBone = nullptr;
	}
}

std::vector<Math::Matrix4> Resources::Skeleton::GetBonesMatrices()
{
	std::vector<Math::Matrix4> Matrix;
	//Matrix.resize(52);
	for (size_t i = 0; i < Bones.size(); i++)
	{
		//Matrix.push_back(Math::Matrix4());
		Matrix.push_back(Bones[i]->GetBoneMatrix());
	}
	return Matrix;
}

bool compareById(Bone* a, Bone* b) {
	return a->Id < b->Id;
}

void Resources::Skeleton::SortBones()
{
	std::sort(Bones.begin(), Bones.end(), compareById);
}

Resources::Skeleton* Resources::Skeleton::Clone() const
{
	// Clone The Skeleton.
	auto skel = new Skeleton(p_path, ResourcesType::Skeleton);
	// Clone All Bones.
	Bone* node = RootBone->Clone();
	skel->RootBone = node;
	skel->Bones = skel->RootBone->GetAllBones();
	skel->SortBones();
	for (auto& bone : skel->Bones)
	{
		bone->m_skeleton = skel;
	}
	return skel;
}

void Resources::Skeleton::RemoveMesh(Component::SkeletalMeshComponent* skelMesh)
{
	auto it = std::find(m_skeletalMeshes.begin(), m_skeletalMeshes.end(), skelMesh);
	if (it != m_skeletalMeshes.end())
	{
		m_skeletalMeshes.erase(it);
	}
}

void Resources::Skeleton::AddMesh(Component::SkeletalMeshComponent* skelMesh)
{
	auto it = std::find(m_skeletalMeshes.begin(), m_skeletalMeshes.end(), skelMesh);
	if (it == m_skeletalMeshes.end())
	{
		m_skeletalMeshes.push_back(skelMesh);
	}
}
