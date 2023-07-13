#include "pch.h"
#include <Components\SkeletalMeshComponent.h>
#include <Components\Animator.h>

#include <Resources\ResourcesManager.h>
#include <Resources\SkeletalMesh.h>
#include <Resources\Skeleton.h>

#include <Render/Camera.h>

#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>

Component::SkeletalMeshComponent::SkeletalMeshComponent()
{
}

Component::SkeletalMeshComponent::~SkeletalMeshComponent()
{
	if (gameObject)
		if (auto anim = gameObject->GetComponent<Animator>())
			anim->SetSkeletalMesh(nullptr);
	if (m_skeleton)
		m_skeleton->RemoveMesh(this);
}

void Component::SkeletalMeshComponent::ShowInInspector()
{
	// SkeletalMesh
	if (WrapperUI::Button("Change Mesh"))
	{
		WrapperUI::OpenPopup("SkeletalMeshPopup");
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(m_skeletalMesh ? m_skeletalMesh->GetPath().c_str() : "None");
	WrapperUI::SameLine();
	if (WrapperUI::Button("Reset"))
		m_skeletalMesh = nullptr;
	if (auto skeletalMesh = Resources::ResourcesManager::Get()->ResourcePopup<Resources::SkeletalMesh>("SkeletalMeshPopup"))
	{
		SetMesh(skeletalMesh);
	}

	// Skeleton
	if (WrapperUI::Button("Change Skeleton"))
	{
		WrapperUI::OpenPopup("SkeletonPopup");
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(m_skeleton ? m_skeleton->GetPath().c_str() : "None");
	if (auto skeleton = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Skeleton>("SkeletonPopup"))
	{
		SetSkeleton(skeleton);
	}

	// Root Bone
	if (WrapperUI::Button("Set Root Bone"))
	{
		WrapperUI::OpenPopup("BonePopup");
	}
	if (WrapperUI::BeginDragDropTarget()) {
		if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
			int PayloadIndex = *(int*)payload->Data;
			Core::GameObject* gameObject = Core::App::Get().sceneManager->GetCurrentNode()->GetWithIndex(PayloadIndex);
			if (auto root = dynamic_cast<Resources::Bone*>(gameObject))
			{
				if (root->GetSkeleton()->RootBone == root)
				{
					SetSkeleton(root->GetSkeleton(), false);
				}
			}
		}
		WrapperUI::EndDragDropTarget();
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(m_skeleton ? m_skeleton->RootBone->GetName().c_str() : "None");
	WrapperUI::SameLine();
	if (WrapperUI::Button("Clear"))
	{
		SetSkeleton(nullptr, false);
	}
	if (auto root = Core::SceneManager::Get()->GetCurrentScene()->BonePopup("BonePopup"))
	{
		SetSkeleton(root->GetSkeleton(), false);
	}
	
	// Other
	if (m_skeleton)
	{
		m_skeleton->RootBone->ShowInInspector();
	}
	if (!m_skeletalMesh) return;
	// Material List.
	static int SelectedRow = 0;
	if (WrapperUI::BeginTable("Materials", 3, TableFlags::Borders))
	{
		for (int row = 0; row < m_materials.size(); row++)
		{
			WrapperUI::TableNextRow();
			for (int column = 0; column < 3; column++)
			{
				WrapperUI::TableSetColumnIndex(column);
				switch (column)
				{
				case 0:
					WrapperUI::TextUnformatted(std::to_string(row).c_str());
					break;
				case 1:
					if (m_materials[row])
						WrapperUI::TextUnformatted(m_materials[row]->GetPath().c_str());
					else
						WrapperUI::TextUnformatted("Missing Material");
					break;
				case 2:
					WrapperUI::PushID(row);
					if (WrapperUI::Button("Change Material"))
					{
						SelectedRow = row;
						WrapperUI::OpenPopup("MaterialPopup");
					}
					WrapperUI::PopID();
					break;
				}
			}
		}
		WrapperUI::PushID(SelectedRow);
		if (auto mat = Core::App::Get().resourcesManager->ResourcePopup<Resources::Material>("MaterialPopup")) {
			m_materials[SelectedRow] = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(mat->GetPath());
		}
		WrapperUI::PopID();
		WrapperUI::EndTable();
	}

	// Material Components List.
	int index = 0;
	for (auto&& mat : m_materials)
	{
		if (!mat)
			continue;
		WrapperUI::PushID(index++);
		WrapperUI::Separator();
		if (WrapperUI::CollapsingHeader(mat->GetName().c_str())) {

			mat->ShowInInspector();
		}
		WrapperUI::PopID();
	}
}

void Component::SkeletalMeshComponent::EditorDraw()
{
	if (m_skeleton)
	{
		m_skeleton->RootBone->DrawDebug();
	}
}

void Component::SkeletalMeshComponent::Draw()
{
	if (m_skeletalMesh && m_skeleton && m_materials.size() != 0)
	{
		bool onFrustum = m_skeletalMesh->IsVisible(Core::SceneManager::Get()->GetCurrentScene()->currentCamera, gameObject->transform);
		if (onFrustum)
			m_skeletalMesh->Render(gameObject->transform->GetModelMatrix(), m_materials, m_skeleton, gameObject->IsSelected());
	}
}

void Component::SkeletalMeshComponent::DrawPicking(int ID)
{
	if (m_skeletalMesh && m_skeleton)
	{
		m_skeletalMesh->PickingResource(gameObject->transform->GetModelMatrix(), m_materials, m_skeleton, ID);
	}
}

void Component::SkeletalMeshComponent::Update()
{
	if (!m_skeletalMesh && !m_meshToLoad.empty())
	{
		if (!m_skeletalMesh)
		{
			SetMesh(Resources::ResourcesManager::Get()->Find<Resources::SkeletalMesh>(m_meshToLoad));
		}
		if (m_skeletalMesh)
		{
			m_meshToLoad.clear();
		}
	}
	if (!m_skeleton && m_boneIndex != -1)
	{
		if (auto rootBone = Core::SceneManager::Get()->GetCurrentScene()->GetSceneNode()->GetWithIndex(m_boneIndex))
			if (auto root = dynamic_cast<Resources::Bone*>(rootBone))
				SetSkeleton(root->GetSkeleton(), false);
		if (m_skeleton)
		{
			m_boneIndex = -1;
		}
	}
}

void Component::SkeletalMeshComponent::SetMesh(Resources::SkeletalMesh* mesh)
{
	if (!mesh || !mesh->HasBeenSent())
		return;
	m_skeletalMesh = mesh;

	if (m_skeletalMesh->GetSubMeshes().size() != this->m_materials.size())
	{
		m_materials.resize(m_skeletalMesh->GetSubMeshes().size());
		for (auto&& mat : m_materials)
		{
			if (!mat)
				mat = Core::App::Get().resourcesManager->Find<Resources::Material>("DefaultMaterial");;
		}
	}

}

void Component::SkeletalMeshComponent::SetSkeleton(Resources::Skeleton* _skeleton, bool addSkeletonToScene /*= true*/)
{
	if (_skeleton)
	{
		if (addSkeletonToScene)
		{
			m_skeleton = dynamic_cast<Resources::Skeleton*>(_skeleton->Clone());
			gameObject->GetParent()->AddChildren(m_skeleton->RootBone);
		}
		else
		{
			m_skeleton = _skeleton;
		}
		m_skeleton->AddMesh(this);
	}
	else
	{
		if (m_skeleton)
		{
			m_skeleton->RemoveMesh(this);
		}
		m_skeleton = nullptr;
	}
}

std::ostream& Component::SkeletalMeshComponent::operator<<(std::ostream& os)
{
	if (m_skeletalMesh) {
		os << m_skeletalMesh->GetModel()->GetPath() << '\n';
		os << m_skeletalMesh->GetPath() << '\n';
	}
	else {
		os << "nullptr" << '\n';
		os << "nullptr" << '\n';
	}
	if (m_skeleton)
		os << m_skeleton->RootBone->uuid << '\n';
	else
		os << "nullptr" << '\n';
	for (auto& mat : m_materials)
	{
		mat->operator<<(os);
	}
	return os;
}

void Component::SkeletalMeshComponent::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_meshToLoad = line;

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_boneIndex = std::stoi(line);

	while (getline(sceneFile, line) && line != "end")
	{
		if (getline(sceneFile, line) && line != "end" && line == "Material") {

			getline(sceneFile, line);
			this->AddMaterial(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(line));
		}
	}

}
