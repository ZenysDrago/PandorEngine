#pragma once
#include "ResourcesManager.h"
#include <Core/Wrappers/WrapperUI.h>
using namespace Core::Wrapper;
#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"
#include "Model.h"
#include "Skybox.h"
#include <filesystem>
#include <algorithm>
#include <Utils/Utils.h>
#include <regex>

using namespace Resources;

template <typename T> inline void Resources::ResourcesManager::Add(std::string name, T* resource)
{
	Utils::ToPath(name);
	if (resourcesList.count(name) == 0)
		resourcesList.emplace(name, resource);
}


template <typename T> inline T* ResourcesManager::Create(std::string name)
{
	std::string fullPath = name;
	Utils::ToPath(name);
	// Make sure the given type is supported.
	bool isIResource = std::is_base_of<IResources, T>::value;
	if (!isIResource) { PrintError("ERROR TYPE"); return nullptr; }
	if (T::GetResourceType() != ResourcesType::Shader && !std::filesystem::exists(fullPath) && resourcesList.count(name) == 0)
	{
		PrintError("ERROR FILE %s DOES NOT EXIST", name.c_str());
		return nullptr;
	}

	/* Check if there is already a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		/* If resource exist with different type delete and create a new */
		if (resourcesList[name]->GetType() != T::GetResourceType())
			Delete(name);
		/* If resource exist with same type, return resource*/
		else if (resourcesList[name]->ShouldBeLoaded()) {
			return (T*)resourcesList[name];
		}
	}
	else
	{
		/*If no resources of same name create a new one */
		resourcesList.emplace(name, (IResources*)(new T(fullPath, T::GetResourceType())));
	}

	/* Add the load function of the resource to the task list of the thread Manager */
	auto resourceIt = resourcesList.find(name);
	if (resourceIt != resourcesList.end()) {
		T* resource = dynamic_cast<T*>(resourceIt->second);
		if (resource && !resource->ShouldBeLoaded()) {
			if (T::GetResourceType() == ResourcesType::Sound) // TEMP need to refactor the using of shouldBeLoaded
				resourcesList[name]->p_shouldBeLoaded = true;
#ifdef MULTITHREAD
			threadManager->Addtask(resource, &T::Load);
#else
			resource->Load();
#endif
		}
	}

	//PrintLog("Resources created : %s", name.c_str());

	/* Return the new resource */
	return (T*)resourcesList[name];
}

// Specific create behavior for the texture resources
inline Resources::Texture* ResourcesManager::Create(std::string name, unsigned int _texType, unsigned int _slot, unsigned int _pixelType)
{
	Utils::ToPath(name);
	/* Check if there is already a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		/* If resource exist with different type delete and create a new */
		if (resourcesList[name]->GetType() != Texture::GetResourceType())
			Delete(name);
		/* If resource exist with same type, return resource*/
		else if (resourcesList[name]->ShouldBeLoaded())
			return (Resources::Texture*)resourcesList[name];
	}
	else {
		/*If no resources of same name create a new one */
		resourcesList.emplace(name, (IResources*)(new Resources::Texture(name, Resources::Texture::GetResourceType(), _texType, _slot, _pixelType)));
	}

	/* Add the load function of the resource to the task list of the thread Manager */
	auto resourceIt = resourcesList.find(name);
	if (resourceIt != resourcesList.end()) {
		Resources::Texture* resource = dynamic_cast<Resources::Texture*>(resourceIt->second);
		if (resource) {
#ifdef MULTITHREAD
			threadManager->Addtask(resource, &Resources::Texture::Load);
#else
			resource->Load();
#endif
		}
	}

	PrintLog("Texture created : %s", name.c_str());

	/* Return the new resource */
	return (Resources::Texture*)resourcesList[name];
}

inline Resources::Skybox* Resources::ResourcesManager::Create(std::string name, std::string faces[6])
{
	std::string fullFaces[6];
	std::copy(faces, faces + 6, fullFaces);
	std::string fullPath = name;
	Utils::ToPath(name);

	for (int i = 0; i < 6; i++)
		Utils::ToPath(faces[i]);

	/* Check if there is already a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		/* If resource exist with different type delete and create a new */
		if (resourcesList[name]->GetType() != SixSided::GetResourceType())
			Delete(name);
		/* If resource exist with same type, return resource*/
		else if (resourcesList[name]->ShouldBeLoaded())
			return (Resources::SixSided*)resourcesList[name];
	}
	else {
		/*If no resources of same name create a new one */
		resourcesList.emplace(name, (IResources*)(new Resources::SixSided(fullPath, fullFaces)));
	}

	/* Add the load function of the resource to the task list of the thread Manager */
	auto resourceIt = resourcesList.find(name);
	if (resourceIt != resourcesList.end()) {
		Resources::Skybox* resource = dynamic_cast<Resources::Skybox*>(resourceIt->second);
		if (resource) {
#ifdef MULTITHREAD
			threadManager->Addtask(resource, &Resources::Skybox::Load);
#else
			resource->Load();
#endif
		}
	}

	PrintLog("Skybox created : %s", name.c_str());

	/* Return the new resource */
	return (Resources::Skybox*)resourcesList[name];
}


// Specific create behavior for the shader resources
inline Resources::Shader* ResourcesManager::Create(std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
{
	name = vertexPath;
	name += " + ";
	name += fragmentPath;

	Utils::ToPath(name);
	/* Check if there is already a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		/* If resource exist with different type delete and create a new */
		if (resourcesList[name]->GetType() != Shader::GetResourceType())
			Delete(name);
		/* If resource exist with same type, return resource*/
		else
			return (Shader*)resourcesList[name];
	}
	else {
		/*If no resources of same name create a new one */
		resourcesList.emplace(name, (IResources*)(new Shader(name, vertexPath, fragmentPath, Shader::GetResourceType())));
	}
	PrintLog("Shader created : %s ", name.c_str());
	/* Return the new resource */
	return (Shader*)resourcesList[name];
}

template <typename T> inline T* ResourcesManager::GetOrLoad(std::string name)
{
	std::string fullPath = name;
	Utils::ToPath(name);
	// Make sure the given type is supported.
	bool isIResource = std::is_base_of<IResources, T>::value;
	if (!isIResource) { PrintError("ERROR TYPE"); return nullptr; }


	/* Check if there is a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		/* If resource exist with different type abort */
		if (resourcesList[name]->GetType() != T::GetResourceType())
		{
			std::string msg = "Resources : " + name + " has been found with a different type // return nullptr";
			PrintWarning(msg.c_str());
			return nullptr;
		}
		/* If resource exist with same type, return resource*/
		else if (resourcesList[name]->ShouldBeLoaded())
		{
			return (T*)resourcesList[name];
		}
	}

	/*If resource doesn't exist or not loaded yet create it */
	return Create<T>(fullPath);
}

template <typename T>
T* Resources::ResourcesManager::Reload(std::string name)
{
	std::string fullpath = name;
	Utils::ToPath(name);
	if (!std::filesystem::exists(name))
		return nullptr;
	// Make sure the given type is supported.
	bool isIResource = std::is_base_of<IResources, T>::value;
	if (!isIResource) { PrintError("ERROR TYPE"); return nullptr; }


	/* Check if there is a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		/* If resource exist with different type abort */
		if (resourcesList[name]->GetType() != T::GetResourceType())
		{
			std::string msg = "Resources : " + name + " has been found with a different type // return nullptr";
			PrintWarning(msg.c_str());
			return nullptr;
		}
	}
	else {
		/*If no resources of same name create a new one */
		resourcesList.emplace(name, (IResources*)(new T(fullpath, T::GetResourceType())));
	}


	/* Add the load function of the resource to the task list of the thread Manager */
	auto resourceIt = resourcesList.find(name);
	if (resourceIt != resourcesList.end()) {
		T* resource = dynamic_cast<T*>(resourceIt->second);
		resource->p_shouldBeLoaded = false;
		resource->isLoaded = false;
		resource->hasBeenSent = false;
		if (resource && !resource->ShouldBeLoaded()) {
#ifdef MULTITHREAD
			threadManager->Addtask(resource, &Resources::Texture::Load);
#else
			resource->Load();
#endif
		}
	}

	/*If resource doesn't exist or not loaded yet create it */
	return Create<T>(fullpath);
}


template <typename T> inline T* ResourcesManager::Find(std::string name)
{
	Utils::ToPath(name);
	/* Check if there is a resource with this name */
	if (resourcesList.count(name) != 0)
	{
		return dynamic_cast<T*>(resourcesList.find(name)->second);
	}
	return nullptr;
}

template <typename T> inline std::vector<T*> Resources::ResourcesManager::GetAllResourcesOfType()
{
	std::vector<T*> map;
	for (auto&& resource : resourcesList)
	{
		if (!resource.second)
			continue;
		if (resource.second->GetType() == T::GetResourceType())
		{
			map.push_back(dynamic_cast<T*>(resource.second));
		}
	}
	return map;
}

template <typename T> T* Resources::ResourcesManager::ResourcePopup(const std::string& popupName)
{
	T* out = nullptr;
	int id = 0;
	if (WrapperUI::BeginPopupModal(popupName.c_str(), (bool*)0, WindowFlags::AlwaysAutoResize)) {
		static TextFilter filter;
		filter.Draw();
		for (auto&& resource : resourcesList)
		{
			if (resource.second->ShouldBeLoaded() && !resource.second->HasBeenSent())
				continue;
			if (dynamic_cast<T*>(resource.second))
			{
				if (!filter.PassFilter(resource.second->GetName().c_str()))
					continue;
				WrapperUI::PushID(id++);
				if (WrapperUI::Selectable(resource.second->GetName().c_str(), false, (SelectableFlags)SelectableFlagsPrivate::SpanAvailWidth)) {
					out = dynamic_cast<T*>(resource.second);
					WrapperUI::CloseCurrentPopup();
				}
				if (WrapperUI::IsItemHovered())
				{
					WrapperUI::BeginTooltip();
					WrapperUI::TextUnformatted(resource.second->GetPath().c_str());
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

inline void ResourcesManager::Rename(const std::string& prevName, const std::string& newName, bool isFolder)
{
	auto pname = prevName;
	Utils::ToPath(pname);
	if (!isFolder) {
		auto iter = resourcesList.find(pname);
		if (iter != resourcesList.end()) {
			// Remove the resource from the map
			IResources* resource = iter->second;
			auto newPath = newName;
			Utils::ToPath(newPath);
			resource->p_fullPath = newName;
			resource->p_path = newPath;
			auto pos = resource->p_path.find_last_of('/') + 1;
			if (pos < resource->p_path.size())
				resource->p_name = resource->p_path.substr(pos);
			else
				resource->p_name = resource->p_path;

			pos = resource->p_name.find_last_of(':') + 1;
			if (pos < resource->p_name.size())
				resource->p_name = resource->p_name.substr(pos);

			resourcesList.erase(pname);

			// Insert the resource into the map with the new key
			resourcesList.insert({ newPath, resource });
		}
		else
		{
			PrintError("%s Not found", prevName.c_str());
		}
	}
	else
	{
		for (auto& res : resourcesList)
		{
			if (std::regex_search(res.first, std::regex(pname)))
			{
				// Remove the resource from the map
				IResources* resource = res.second;

				std::string newPath = newName + '/' + resource->p_name;
				Utils::ToPath(newPath);

				std::string prevPath = resource->p_path;

				resource->p_path = newPath;

				// Remove Previous Resource
				resourcesList.erase(prevPath);

				// Insert the resource into the map with the new key
				resourcesList.insert({ newPath, resource });
			}
		}
	}
}