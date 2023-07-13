#include "pch.h"

#include <Resources/Model.h>
#include <Resources/SkeletalMesh.h>
#include <Resources/Skeleton.h>
#include <Resources/ResourcesManager.h>
#include <Core/GameObject.h>
#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Components/MeshComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Resources/Mesh.h>
#include <Utils/Loader.h>
#include <regex>

Resources::Model::~Model()
{

}

Model& Resources::Model::operator=(const Model&)
{
	return *this;
}

void Resources::Model::Load()
{
	if (isLoaded || p_shouldBeLoaded)
		return;
	p_shouldBeLoaded = true;
	if (p_path.substr(p_path.find_last_of('.')) == ".obj")
		Utils::Loader::OBJ::Load(this, p_fullPath);
	else if (p_path.substr(p_path.find_last_of('.')) == ".fbx")
		Utils::Loader::FBX::Load(this, p_fullPath);

#ifndef PANDOR_GAME
	std::string filePath = std::regex_replace(p_path, std::regex("/"), "~");
	filePath = std::regex_replace(filePath, std::regex("\\\\"), "~");
	// Chek the last update of the thumbnails and the model file, if the models was update after the thumbnail creation then Create a new

	std::string thumbnailPath = ENGINEPATH"Textures/Thumbnails/" + filePath + ".tmb";
	std::string assetPath = "assets~";
	if (filePath.substr(0, assetPath.length()) == assetPath)
	{
		std::string folder = ENGINEPATH"Textures/Thumbnails/" + Resources::ResourcesManager::ProjectName();
		if (!std::filesystem::exists(folder))
		{
			std::filesystem::create_directory(folder);
		}
		thumbnailPath = folder + '/' + filePath + ".tmb";
	}

	try {
		if (std::filesystem::exists(thumbnailPath)) {
			// Get the last update time of the file
			const auto last_update_thumbnail = std::filesystem::last_write_time(thumbnailPath);

			// Get the time of the last update of the model.
			const auto last_update_model = std::filesystem::last_write_time(p_fullPath);

			// Print the time difference in seconds
			if (last_update_model > last_update_thumbnail)
			{
				Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Model);
			}
			// Else check if a material inside the model has been updated.
			else
			{
				for (auto&& material : m_materials) {
					if (material && std::filesystem::exists(material->GetPath())) {
						// Get the time of the last update of the model.
						const auto last_update_material = std::filesystem::last_write_time(material->GetPath());

						if (last_update_material > last_update_thumbnail)
						{
							Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Model);
						}
					}

				}
			}
		}
		else {
			Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Model);
		}
	}
	catch (const std::exception& ex) {
		PrintError("An error occurred: %s", ex.what());
	}
#endif
	isLoaded.store(true);
}

void Resources::Model::SendResource()
{
}

Core::GameObject* Resources::Model::ToGameObject(bool isRuntimeObject /*= true*/)
{
	auto gameObject = (isRuntimeObject) ? Core::App::Get().sceneManager->GetCurrentScene()->CreateObject(GetName()) : new Core::GameObject(GetName());
	int materialIndex = 0;
	Resources::Skeleton* skeleton = nullptr;
	for (auto&& mesh : m_meshes)
	{
		Core::GameObject* child = nullptr;
		auto skeletalMesh = dynamic_cast<Resources::SkeletalMesh*>(mesh);
		if (m_meshes.size() == 1 && !skeletalMesh) {
			child = gameObject;
		}
		else {
			child = (isRuntimeObject) ? Core::App::Get().sceneManager->GetCurrentScene()->CreateObject(mesh->GetName()) : new Core::GameObject(mesh->GetName());
			gameObject->AddChildren(child);
		}
		if (skeletalMesh)
		{
			auto skeltalMeshComponent = new Component::SkeletalMeshComponent();
			child->AddComponent(skeltalMeshComponent);
			bool addSkeleton = false;
			if (!skeleton)
			{
				skeleton = Resources::ResourcesManager::Get()->Find<Resources::Skeleton>(GetPath() + "::" + "Skel");
				addSkeleton = true;
			}
			if (skeleton)
			{
				skeltalMeshComponent->SetSkeleton(skeleton, addSkeleton);
				// Because of Clone
				skeleton = skeltalMeshComponent->GetSkeleton();
			}
			for (int i = 0; i < mesh->m_subMeshes.size(); i++)
			{
				if (m_materials.size() > materialIndex)
					skeltalMeshComponent->AddMaterial(m_materials[materialIndex]);
				else
					break;
				materialIndex++;
			}
			skeltalMeshComponent->SetMesh(skeletalMesh);
		}
		else
		{
			auto meshComponent = new Component::MeshComponent();
			child->AddComponent(meshComponent);
			for (int i = 0; i < mesh->m_subMeshes.size(); i++)
			{
				if (m_materials.size() > materialIndex)
					meshComponent->AddMaterial(m_materials[materialIndex]);
				else
					break;
				materialIndex++;
			}
			meshComponent->SetMesh(mesh);
			meshComponent->gameObject->transform->SetLocalPosition(mesh->m_translation);
		}
	}
	return gameObject;
}

void Resources::Model::AddMesh(Resources::Mesh* mesh)
{
	this->m_meshes.push_back(mesh);
	mesh->m_fromModel = this;
}

bool Resources::Model::HasBeenSent()
{
	if (hasBeenSent)
		return true;

	if (!isLoaded)
		return false;

	for (auto&& mesh : m_meshes)
	{
		if (!mesh->HasBeenSent())
			return false;
	}
	for (auto&& skeleton : m_skeletons)
	{
		if (!skeleton->HasBeenSent())
			return false;
	}
	hasBeenSent = true;
	return true;
}

void Resources::Model::SetupMeshComp(std::string path, Component::MeshComponent* meshComp)
{
	//Resources::Mesh* mesh = Resources::ResourcesManager::Get()->Find<Resources::Mesh>(path);

	meshComp->SetMeshToLoad(path);
}

float Resources::Model::GetMaxVerticeOnXYZ()
{
	return std::fmaxf(m_maxXYZ.x, std::fmaxf(m_maxXYZ.y, m_maxXYZ.z));
}

std::vector<Material*> Resources::Model::GetMaterialsForMesh(Resources::Mesh* mesh)
{
	std::vector<Material*> mats;
	auto iter = std::find(m_meshes.begin(), m_meshes.end(), mesh);
	if (iter != m_meshes.end())
	{
		size_t index = std::distance(m_meshes.begin(), iter);
		int materialIndex = 0;
		for (size_t i = 0; i < m_meshes.size(); i++)
		{
			if (i == index)
				break;
			materialIndex += (int)m_meshes[i]->GetSubMeshes().size();
		}
		if (m_materials.size() <= materialIndex)
			return mats;
		for (size_t i = 0; i < mesh->GetSubMeshes().size(); i++)
		{
			mats.push_back(this->m_materials[materialIndex + i]);
		}
	}
	return mats;
}

Texture* Resources::Model::GetThumbnail()
{
	if (!m_thumbnail)
	{
		auto filePath = std::regex_replace(p_path, std::regex("/"), "~");
		std::string thumbnailPath = ENGINEPATH"Textures/Thumbnails/" + filePath + ".tmb";
		std::string assetPath = "assets~";
		if (filePath.substr(0, assetPath.length()) == assetPath)
		{
			std::string folder = ENGINEPATH"Textures/Thumbnails/" + Resources::ResourcesManager::ProjectName();
			if (!std::filesystem::exists(folder))
			{
				std::filesystem::create_directory(folder);
			}
			thumbnailPath = folder + '/' + filePath + ".tmb";
		}
		m_thumbnail = Resources::ResourcesManager::Get()->Reload<Resources::Texture>(thumbnailPath);
	}
	return m_thumbnail;
}

void Resources::Model::Display(bool value)
{
	m_displayOnResourcesManager = value;
	for (auto& mesh : m_meshes)
	{
		mesh->Display(value);
	}
}
