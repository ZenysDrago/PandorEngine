#include "pch.h"

#include "../Resources/ResourcesManager.h"
#include <Resources/Material.h>
#include <Resources/Font.h>
#include <Resources/Animation.h>
#include <Resources/AnimationController.h>
#include <Resources/PhysicMaterial.h>
#include <Resources/Sound.h>
#include <Resources/Prefab.h>

using namespace Resources;
std::string ResourcesManager::m_engineResourcesPath = "";
std::string ResourcesManager::m_assetsPath = "";
std::string ResourcesManager::m_projectName = "";
std::string ResourcesManager::m_projectPath = "";

ResourcesManager* ResourcesManager::m_instance = nullptr;

ResourcesManager::ResourcesManager(Core::ThreadManager* _threadManager)
{
	threadManager = _threadManager;
}

std::string Resources::ResourcesManager::GetFileName(const std::string& path)
{
	std::string fullPath = path;

	size_t sepPos = fullPath.find_last_of("/\\") != std::string::npos ? fullPath.find_last_of("/\\") : 0;

	std::string fileName = fullPath.substr(sepPos == 0 ? sepPos : sepPos + 1);

	return fileName;
}

ResourcesManager::~ResourcesManager()
{
	DeleteAll();
}

void ResourcesManager::Delete(const std::string& name)
{
	auto newPath = name;
	Utils::ToPath(newPath);
	std::string msg;
	if (resourcesList.count(newPath) <= 0)
	{
		msg = "No resource of name : " + newPath + "found::abort ";
		PrintError(msg.c_str());
		return;
	}

	if (!resourcesList[newPath]->IsLoaded())
	{
		msg = "Resource : " + newPath + " is not loaded abort delete";
		PrintError(msg.c_str());
		return;
	}

	delete resourcesList[name];
	resourcesList[name] = nullptr;
	resourcesList.erase(name);
}


void Resources::ResourcesManager::DeleteAll()
{
	for (auto& it : resourcesList)
	{
		delete it.second;
		it.second = nullptr;
	}

	defaultShader = nullptr;
	unlitShader = nullptr;
	skyboxShader = nullptr;

	resourcesList.clear();
}
bool ResourcesManager::IsEverythingLoaded()
{
	for (auto& it : resourcesList)
	{
		if (it.second->ShouldBeLoaded() && !it.second->IsLoaded())
			return false;
	}
	return true;
}

bool ResourcesManager::IsEverythingSent()
{
	for (auto& it : resourcesList)
	{
		if (it.second->ShouldBeLoaded() && !it.second->HasBeenSent())
			return false;
	}
	return true;
}

std::unordered_map<std::string, IResources*> ResourcesManager::GetAllResources()
{
	return resourcesList;
}

void ResourcesManager::SetupInitalResources()
{
}

void Resources::ResourcesManager::LoadNecessaryResources()
{
	Create<Resources::Shader>(ENGINEPATH"Shaders/Skybox/skybox");
	std::string faces[6] = {
		ENGINEPATH"CubeMaps/DefaultSkybox/right.jpg",
		ENGINEPATH"CubeMaps/DefaultSkybox/left.jpg",
		ENGINEPATH"CubeMaps/DefaultSkybox/top.jpg",
		ENGINEPATH"CubeMaps/DefaultSkybox/bottom.jpg",
		ENGINEPATH"CubeMaps/DefaultSkybox/front.jpg",
		ENGINEPATH"CubeMaps/DefaultSkybox/back.jpg",
	};
	Create("CubeMaps/DefaultSkybox", faces);

	Create<Resources::Shader>(ENGINEPATH"Shaders/phong");
	Create<Resources::Shader>(ENGINEPATH"Shaders/unlit");
	Create<Resources::Shader>(ENGINEPATH"Shaders/UIShader");
	Create<Resources::Shader>(ENGINEPATH"Shaders/picking");
	Create<Resources::Shader>(ENGINEPATH"Shaders/outlining");
	Create<Resources::Shader>(ENGINEPATH"Shaders/KernelShader/kernel");
	Create<Resources::Shader>(ENGINEPATH"Shaders/PBR");
	Create<Resources::Shader>(ENGINEPATH"Shaders/Shadow/ShadowMap");

	std::string name = ENGINEPATH"Shaders/BillboardShader/billboard";
	Create(name, ENGINEPATH"Shaders/BillboardShader/billboard.vert", ENGINEPATH"Shaders/unlit.frag");

	// Create shader anim unlit
	name = ENGINEPATH"Shaders/AnimationShader/AnimationUnlit";
	Resources::Shader* animShader = Create(name, ENGINEPATH"Shaders/AnimationShader/animation.vert", ENGINEPATH"Shaders/unlit.frag");
	animShader->SetName("Animation Unlit");

	// Create shader anim Phong
	name = ENGINEPATH"Shaders/AnimationShader/AnimationPhong";
	animShader = Create(name, ENGINEPATH"Shaders/AnimationShader/animation.vert", ENGINEPATH"Shaders/phong.frag");
	animShader->SetName("Animation Phong");

	name = ENGINEPATH"Shaders/BillboardInstanceShader/instancing.vert";
	Resources::ResourcesManager::Get()->Create<Resources::VertexShader>(name);
}

Resources::Shader* ResourcesManager::GetDefaultShader()
{
	if (defaultShader)
		return defaultShader;
	else
		return defaultShader = Find<Resources::Shader>(ENGINEPATH"Shaders/phong");
}

Resources::Shader* ResourcesManager::GetUnlitShader()
{
	if (unlitShader)
		return unlitShader;
	else
		return unlitShader = Find<Resources::Shader>(ENGINEPATH"Shaders/unlit");
}

Resources::Shader* ResourcesManager::GetSkyboxShader()
{
	if (skyboxShader)
		return skyboxShader;
	else
		return skyboxShader = Find<Resources::Shader>(ENGINEPATH"Shaders/Skybox/skybox");
}

Mesh* ResourcesManager::MeshPopup(const std::string& popupName)
{
	float size = 48;
	Resources::Mesh* out = nullptr;
	int id = 0;
	if (WrapperUI::BeginPopupModal(popupName.c_str(), (bool*)0)) {
		static TextFilter filter;
		filter.Draw();
		WrapperUI::BeginChild("MaterialChild", WrapperUI::GetWindowSize() - Math::Vector2(25.f, 85.f), true);
		for (auto& resource : resourcesList)
		{
			if (auto res = dynamic_cast<Resources::Mesh*>(resource.second))
			{
				if (!filter.PassFilter(res->GetName().c_str()) || !res->MustBeDisplay())
					continue;
				WrapperUI::PushID(id++);
				WrapperUI::BeginGroup();
				if (res->GetThumbnail())
					WrapperUI::Image(res->GetThumbnail()->ID, Math::Vector2(size, size));
				else
					WrapperUI::Dummy(Math::Vector2(size, size));
				WrapperUI::SameLine();
				if (WrapperUI::Button(res->GetName().c_str(), Math::Vector2(WrapperUI::GetWindowSize().x, size))) {
					out = res;
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::EndGroup();
				if (WrapperUI::IsItemHovered())
				{
					WrapperUI::BeginTooltip();
					WrapperUI::TextUnformatted(res->GetPath().c_str());
					WrapperUI::EndTooltip();
				}
				WrapperUI::PopID();
			}
		}
		WrapperUI::EndChild();
		if (WrapperUI::Button("Close"))
		{
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
	return out;
}

Resources::Material* ResourcesManager::MaterialPopup(const std::string& popupName)
{
	Resources::Material* out = nullptr;
	int id = 0;
	if (WrapperUI::BeginPopupModal(popupName.c_str(), (bool*)0)) {
		static TextFilter filter;
		filter.Draw();
		WrapperUI::BeginChild("MaterialChild", WrapperUI::GetWindowSize() - Math::Vector2(25.f, 85.f), true);
		for (auto& resource : resourcesList)
		{
			if (auto res = dynamic_cast<Resources::Material*>(resource.second))
			{
				if (!filter.PassFilter(res->GetName().c_str()) || !res->MustBeDisplay())
					continue;
				WrapperUI::PushID(id++);
				WrapperUI::BeginGroup();
				if (res->GetThumbnail())
					WrapperUI::Image(res->GetThumbnail()->ID, Math::Vector2(32, 32));
				else
					WrapperUI::Dummy(Math::Vector2(32, 32));
				WrapperUI::SameLine();
				if (WrapperUI::Button(res->GetName().c_str(), Math::Vector2(WrapperUI::GetWindowSize().x, 32))) {
					out = res;
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::EndGroup();
				if (WrapperUI::IsItemHovered())
				{
					WrapperUI::BeginTooltip();
					WrapperUI::TextUnformatted(res->GetPath().c_str());
					WrapperUI::EndTooltip();
				}
				WrapperUI::PopID();
			}
		}
		WrapperUI::EndChild();
		if (WrapperUI::Button("Close"))
		{
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
	return out;
}


Model* ResourcesManager::ModelPopup(const std::string& popupName)
{
	Resources::Model* out = nullptr;
	int id = 0;
	if (WrapperUI::BeginPopupModal(popupName.c_str(), (bool*)0)) {
		static TextFilter filter;
		filter.Draw();
		WrapperUI::BeginChild("ModelChild", WrapperUI::GetWindowSize() - Math::Vector2(25.f, 85.f), true);
		for (auto& resource : resourcesList)
		{
			if (auto res = dynamic_cast<Resources::Model*>(resource.second))
			{
				if (!filter.PassFilter(res->GetName().c_str()) || !res->MustBeDisplay())
					continue;
				WrapperUI::PushID(id++);
				WrapperUI::BeginGroup();
				if (res->GetThumbnail())
					WrapperUI::Image(res->GetThumbnail()->ID, Math::Vector2(48, 48));
				else
					WrapperUI::Dummy(Math::Vector2(48, 48));
				WrapperUI::SameLine();
				if (WrapperUI::Button(res->GetName().c_str(), Math::Vector2(WrapperUI::GetWindowSize().x, 48))) {
					out = res;
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::EndGroup();
				if (WrapperUI::IsItemHovered())
				{
					WrapperUI::BeginTooltip();
					WrapperUI::TextUnformatted(res->GetPath().c_str());
					WrapperUI::EndTooltip();
				}
				WrapperUI::PopID();
			}
		}
		WrapperUI::EndChild();
		if (WrapperUI::Button("Close"))
		{
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
	return out;
}

Resources::Texture* ResourcesManager::TexturePopup(const char* popupName)
{
	Resources::Texture* out = nullptr;
	int id = 0;
	if (WrapperUI::BeginPopupModal(popupName, (bool*)0, WindowFlags::AlwaysAutoResize)) {
		static TextFilter filter;
		filter.Draw();
		for (auto resource : resourcesList)
		{
			if (!resource.second->HasBeenSent())
				continue;
			if (auto res = dynamic_cast<Resources::Texture*>(resource.second))
			{
				if (!filter.PassFilter(res->GetName().c_str()) || !res->MustBeDisplay())
					continue;
				WrapperUI::PushID(id++);
				WrapperUI::Image(res->ID, Math::Vector2(16, 16));
				WrapperUI::SameLine();
				if (WrapperUI::Selectable(res->GetName().c_str(), false, SelectableFlags::SpanAllColumns)) {
					out = res;
					WrapperUI::CloseCurrentPopup();
				}
				if (WrapperUI::IsItemHovered())
				{
					WrapperUI::BeginTooltip();
					WrapperUI::TextUnformatted(res->GetPath().c_str());
					WrapperUI::EndTooltip();
				}
				WrapperUI::PopID();
			}
		}
		if (WrapperUI::Button("Close"))
		{
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
	return out;
}

#include <regex>
void ResourcesManager::ResourceLoad(const std::string& path)
{
	auto ext = path.substr(path.find_last_of('.') + 1);
	if (ext == "obj" || ext == "fbx")
	{
		Add<Resources::Model>(path, new Model(path, ResourcesType::Model));
	}
	else if (ext == "jpg" || ext == "png" || ext == "jpeg")
	{
		Create<Resources::Texture>(path);
	}
	else if (ext == "ttf")
	{
		Add<Resources::Font>(path, new Font(path, ResourcesType::Font));
	}
	else if (ext == "mat")
	{
		Add<Resources::Material>(path, new Material(path, ResourcesType::Material));
	}
	else if (ext == "prefab")
	{
		Add<Resources::Prefab>(path, new Prefab(path, ResourcesType::Prefab));
	}
	else if (ext == "anim")
	{
		Add<Resources::Animation>(path, new Animation(path, ResourcesType::Animation));
	}
	else if (ext == "ac")
	{
		Add<Resources::AnimationController>(path, new AnimationController(path, ResourcesType::AnimationController));
	}
	else if (ext == "phm")
	{
		Add<Resources::PhysicMaterial>(path, new PhysicMaterial(path, ResourcesType::PhysicMaterial));
	}
	else if (ext == "cbm")
	{
		Add<Resources::CubeMap>(path, new CubeMap(path, ResourcesType::Skybox));
	}
	else if (ext == "wav" || ext == "MP3" || ext == "FLAC" || ext == "mp3"|| ext == "flac")
	{
		Add<Resources::Sound>(path, new Sound(path, ResourcesType::Sound));
	}
	if (ext == "tmb")
	{
#ifndef PANDOR_GAME
		std::string resourcePath = path.substr(path.find_last_of("//\\") + 1);
		resourcePath = resourcePath.substr(0, resourcePath.find_last_of('.'));;
		resourcePath = std::regex_replace(resourcePath, std::regex("~"), "/");
		if (!std::filesystem::exists(ResourcesManager::ProjectPath() + '/' + resourcePath)
			&& !std::filesystem::exists(resourcePath)
			&& !std::regex_search(resourcePath, std::regex("`")))
		{
			// if the file does not exist and it's not a mesh
			PrintWarning("Deleted Thumbnail %s", path.c_str());
			std::filesystem::remove_all(path.c_str());
		}
		else if (std::regex_search(resourcePath, std::regex("`")))
		{
			resourcePath = std::regex_replace(resourcePath, std::regex("`"), ":");
			auto modelPath = resourcePath.substr(0, resourcePath.find_last_of(':'));
			if (!std::filesystem::exists(ResourcesManager::ProjectPath() + '/' + modelPath)
				&& !std::filesystem::exists(modelPath))
			{
				// if the file is a mesh and model not found
				PrintWarning("Deleted Thumbnail %s", path.c_str());
				std::filesystem::remove_all(path.c_str());
			}
			else if (auto model = Resources::ResourcesManager::Get()->Find<Resources::Model>(modelPath))
			{
				if (model->IsLoaded() && model->HasBeenSent())
				{
					if (!Resources::ResourcesManager::Get()->Contain(resourcePath)
						&& !Resources::ResourcesManager::Get()->Contain(ResourcesManager::ProjectPath() + '/' + resourcePath))
					{
						// if the file is a mesh and model loaded and mesh not found
						PrintWarning("Deleted Thumbnail %s", path.c_str());
						std::filesystem::remove_all(path.c_str());
					}
				}
			}

		}
#endif
	}
}



void ResourcesManager::FilesLoad(const std::string& path)
{
	if (!std::filesystem::exists(path))
		return;
	auto dirIt = std::filesystem::directory_iterator(path);
	for (const auto& entry : dirIt) {
		auto ext = entry.path().string().substr(entry.path().string().find_last_of('.') + 1);
		if (entry.is_directory())
		{
			FilesLoad(entry.path().string());
		}
		else if (!Contain(entry.path().string())) {
			ResourceLoad(entry.path().string());
		}
	}
}


bool ResourcesManager::Contain(const std::string& filename)
{
	return resourcesList.count(filename) != 0;
}
