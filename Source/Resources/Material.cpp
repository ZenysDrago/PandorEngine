#include "pch.h"

#include <Resources/Material.h>
#include <Resources/ResourcesManager.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Utils/Loader.h>
#include <regex>

#include <Core/App.h>

Resources::Material::Material(std::string _name, ResourcesType _type) : IResources(_name, _type)
{
	this->SetShader(ResourcesManager::Get()->GetDefaultShader());
	this->hasBeenSent = true;
}

Resources::Material::~Material()
{
}

void Resources::Material::Load()
{
	p_shouldBeLoaded = true;
	if (isLoaded)
		return;
	Utils::Loader::MAT::Load(this, p_fullPath);
	std::string filePath = std::regex_replace(p_path, std::regex("/"), "~");
	filePath = std::regex_replace(filePath, std::regex("\\\\"), "~");
	 

#ifndef PANDOR_GAME
	// Check the last update of the thumbnails and the model file, if the models was update after the thumbnail creation then Create a new
	std::string thumbnailPath = ENGINEPATH"Textures/Thumbnails/" + filePath + ".tmb";
	try {
		if (std::filesystem::exists(thumbnailPath)) {
			// Get the last update time of the file
			const auto last_update_thumbnail = std::filesystem::last_write_time(thumbnailPath);

			// Get the time of the last update of the model.
			const auto last_update_material = std::filesystem::last_write_time(p_fullPath);

			// Print the time difference in seconds
			if (last_update_material > last_update_thumbnail)
			{
				Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Material);
			}
		}
		else {
			Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Material);
		}
	}
	catch (const std::exception& ex) {
		PrintError("An error occurred: %s", ex.what());
	}
#endif

	isLoaded = true;
}

void Resources::Material::Save()
{
	Utils::Loader::MAT::Save(this);

#ifndef PANDOR_GAME
	Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Material);
#endif
}

void Resources::Material::SendResource()
{
}

Texture* Resources::Material::GetThumbnail()
{
	if (!m_thumbnail)
	{
		auto filePath = std::regex_replace(p_fullPath, std::regex("/"), "~");
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

void Resources::Material::SetShader(Resources::Shader* shader, bool createOthers /*= true*/)
{
	if (shader == m_shader)
		return;
	m_shader = shader;
	if (createOthers) {
		if (!m_shader->GetVertex())
			return;
		auto name = Utils::StringFormat("%s + %s", m_shader->GetVertex()->GetPath().c_str(), ENGINEPATH"Shaders/outlining.frag");
		m_outlineShader = Resources::ResourcesManager::Get()->Find<Resources::Shader>(name);
		if (!m_outlineShader) {
			m_outlineShader = Resources::ResourcesManager::Get()->Create(name, m_shader->GetVertex()->GetPath(), ENGINEPATH"Shaders/outlining.frag");
			m_outlineShader->SetName(Utils::StringFormat("%s + %s", m_shader->GetVertex()->GetName().c_str(), "outlining.frag"));
			Core::App::Get().AddResourceToSend(name);
		}

		name = Utils::StringFormat("%s + %s", m_shader->GetVertex()->GetPath().c_str(), ENGINEPATH"Shaders/picking.frag");
		m_pickingShader = Resources::ResourcesManager::Get()->Find<Resources::Shader>(name);
		if (!m_pickingShader) {
			m_pickingShader = Resources::ResourcesManager::Get()->Create(name, m_shader->GetVertex()->GetPath(), ENGINEPATH"Shaders/picking.frag");
			m_pickingShader->SetName(Utils::StringFormat("%s + %s", m_shader->GetVertex()->GetName().c_str(), "picking.frag"));
			Core::App::Get().AddResourceToSend(name);
		}
	}
}

std::ostream& Resources::Material::operator<<(std::ostream& os) const
{
	os << "===" << "\n";
	os << "Material" << "\n";
	os << p_path << "\n";
	return os;
}

void Resources::Material::ShowInInspector()
{
	// Shader
	if (WrapperUI::Button("Shader"))
	{
		WrapperUI::OpenPopup("ShaderPopup");
	}
	if (auto sha = Core::App::Get().resourcesManager->ResourcePopup<Resources::Shader>("ShaderPopup"))
		SetShader(sha);
	WrapperUI::SameLine();
	if (GetShader())
		WrapperUI::TextUnformatted(GetShader()->GetName().c_str());
	else
		WrapperUI::TextUnformatted("None");

	// Textures
	
	if (WrapperUI::ChangeTextureButton(m_texture, "Change Albedo"))
	{
		SetTexture(m_texture);
	}
	if (WrapperUI::ChangeTextureButton(m_normalMap, "Change Normal Map"))
	{
		SetNormalMap(m_normalMap);
	}
	if (WrapperUI::ChangeTextureButton(m_roughnessMap, "Change Roughness Map"))
	{
		SetRoughnessMap(m_roughnessMap);
	}
	if (WrapperUI::ChangeTextureButton(m_metallicMap, "Change Metallic Map"))
	{
		SetMetallicMap(m_metallicMap);
	}
	if(m_roughnessMap == nullptr)
		WrapperUI::DragFloat("Roughness", &roughness, 0.001f, 0.0f, 1.0f);
	if (m_metallicMap == nullptr)
		WrapperUI::DragFloat("Metallic", &metallic, 0.001f, 0.0f, 1.0f);

	// Parameters
	WrapperUI::ColorEdit4("Ambient", &m_ambient.x);
	WrapperUI::ColorEdit4("Diffuse", &m_diffuse.x);
	WrapperUI::ColorEdit4("Specular", &m_specular.x);

	if (WrapperUI::Button("Save"))
	{
		Save();
	}
}
