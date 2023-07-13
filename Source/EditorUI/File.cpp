#include "pch.h"

#include <Core/App.h>
#include <EditorUI/File.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/FileExplorer.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Texture.h>
#include <Resources/Shader.h>
#include <Resources/Material.h>
#include <Resources/Model.h>
#include <Resources/AnimationController.h>
#include <Resources/PhysicMaterial.h>
#include <Resources/Skybox.h>

#include <regex>
using namespace Resources;

// -------------- File --------------
EditorUI::File::File() {}


void RemoveFile(std::string& directory)
{
	PrintWarning("Deleted Thumbnail %s", directory.c_str());
	std::filesystem::remove_all(directory.c_str());
}

EditorUI::File::File(std::string filename) {
	directory = filename;
	this->name = filename.substr(filename.find_last_of("//\\") + 1);
	std::string extension = this->name.substr(this->name.find_last_of(".") + 1);
	if (std::filesystem::is_directory(directory))
	{
		this->type = EditorUI::FileType::Folder;
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/folder.png");
	}
	else if (extension == "png" || extension == "jpeg" || extension == "jpg")
	{
		this->type = EditorUI::FileType::Img;
		if (auto tex = ResourcesManager::Get()->GetOrLoad<Texture>(this->directory.c_str())) {
			this->icon = tex;
			this->resourceLink = tex;
		}
	}
	else if (extension == "tmb")
	{
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(this->directory.c_str());
		auto newDir = name.substr(0, name.find_last_of('.'));
		auto filePath = std::regex_replace(newDir, std::regex("~"), "/");
		if (!std::filesystem::exists(ResourcesManager::ProjectPath() + '/' + filePath) && !std::regex_search(filePath, std::regex("`")))
		{
			RemoveFile(directory);
		}
		else if (std::regex_search(filePath, std::regex("`")))
		{
			filePath = std::regex_replace(filePath, std::regex("`"), ":");
			auto modelPath = filePath.substr(0, filePath.find_last_of(':'));
			if (auto model = Resources::ResourcesManager::Get()->Find<Resources::Model>(modelPath))
			{
				if (model->IsLoaded() && model->HasBeenSent())
				{
					if (!Resources::ResourcesManager::Get()->Contain(filePath))
					{
						RemoveFile(directory);
					}
				}
			}
			else
			{
				RemoveFile(directory);
			}
		}
	}
	else if (extension == "mat")
	{
		this->type = EditorUI::FileType::Mat;
		std::string path = directory;
		Utils::ToPath(path);
		std::string filePath = std::regex_replace(path, std::regex("/"), "~");
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
		this->icon = ResourcesManager::Get()->Reload<Resources::Texture>(thumbnailPath);
		this->resourceLink = ResourcesManager::Get()->Find<Resources::Material>(directory);
	}
	else if (extension == "obj" || extension == "fbx")
	{
		this->type = EditorUI::FileType::Model;
		std::string path = directory;
		Utils::ToPath(path);
		std::string filePath = std::regex_replace(path, std::regex("/"), "~");
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
		this->icon = ResourcesManager::Get()->Reload<Resources::Texture>(thumbnailPath);
		this->resourceLink = ResourcesManager::Get()->Find<Model>(directory);
	}
	else if (extension == "vert")
	{
		this->type = EditorUI::FileType::Shdr;
		this->resourceLink = ResourcesManager::Get()->Find<VertexShader>(directory);
	}
	else if (extension == "frag")
	{
		this->type = EditorUI::FileType::Shdr;
		this->resourceLink = ResourcesManager::Get()->Find<FragmentShader>(directory);
	}
	else if (extension == "scn")
	{
		this->type = EditorUI::FileType::Scn;
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/scene.png");
	}
	else if (extension == "anim")
	{
		this->type = EditorUI::FileType::Anim;
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/anim.png");
	}
	else if (extension == "ac")
	{
		this->type = EditorUI::FileType::AnimC;
		this->resourceLink = ResourcesManager::Get()->Find<AnimationController>(directory);
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/animc.png");
	}
	else if (extension == "phm")
	{
		this->type = EditorUI::FileType::Phm;
		this->resourceLink = ResourcesManager::Get()->Find<PhysicMaterial>(directory);
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/phm.png");
	}
	else if (extension == "cbm")
	{
		this->type = EditorUI::FileType::Cbm;
		auto skyBox = ResourcesManager::Get()->Find<Resources::CubeMap>(directory);
		this->resourceLink = skyBox;
	}
	else if (extension == "prefab")
	{
		this->type = EditorUI::FileType::Prefab;
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/prefab.png");
	}
	else if (extension == "wav" || extension == "mp3" || extension == "ogg")
	{
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/audio file.png");
	}
	else if (extension == "cs")
	{
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/scripts.png");
	}
	else
	{
		this->type = EditorUI::FileType::None;
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/Blank.png");
	}
	if (!icon)
		this->icon = ResourcesManager::Get()->GetOrLoad<Texture>(ENGINEPATH"Textures/icons/Blank.png");
}


EditorUI::File::~File() {}

void EditorUI::File::FoundChildren()
{
	childrens.clear();
	if (directory.substr(this->directory.find_last_of("/") + 1) == this->directory)
		this->directory += '/';
	if (!std::filesystem::exists(this->directory))
		return;
	for (const auto& entry : std::filesystem::directory_iterator(this->directory)) {
		std::string dir = entry.path().generic_string().data();
		try
		{
			childrens.push_back(std::make_shared<File>(dir));
		}
		catch (const std::exception&)
		{

		}
	}
}

void EditorUI::File::FindAllChilden()
{
	childrens.clear();
	if (directory.substr(this->directory.find_last_of("/") + 1) == this->directory)
		this->directory += '/';
	if (this->type == FileType::Folder) {
		for (const auto& entry : std::filesystem::directory_iterator(this->directory)) {
			std::string dir = entry.path().generic_string().data();
			try
			{
				childrens.push_back(std::make_shared<File>(dir));
			}
			catch (const std::exception&)
			{

			}
		}
		for (auto child : childrens)
		{
			child->FindAllChilden();
		}
	}
}

std::shared_ptr<EditorUI::File> EditorUI::File::GetParent()
{
	auto dir = this->directory.substr(0, this->directory.find_last_of("/"));
	auto par = std::make_shared<File>(dir);
	par->FoundChildren();
	return par;
}
void EditorUI::File::DrawInFileExplorer(File*& clicked)
{
	if (this->type == FileType::Folder)
	{
		auto flag = TreeNodeFlags::Leaf;
		for (auto&& child : childrens) {
			if (child->type == FileType::Folder) {
				flag = TreeNodeFlags::None;
				break;
			}
		}
		WrapperUI::BeginGroup();
		if (icon && icon->HasBeenSent())
			WrapperUI::Image(icon->ID, Math::Vector2(16, 16));
		WrapperUI::SameLine();
		if (WrapperUI::TreeNodeEx(this->name.c_str(), flag)) {
			if (WrapperUI::IsItemClicked())
			{
				clicked = this;
			}
			for (auto&& child : childrens)
			{
				child->DrawInFileExplorer(clicked);
			}
			WrapperUI::TreePop();
		}
		WrapperUI::EndGroup();

		if (WrapperUI::BeginDragDropTarget()) {
			if (const Payload* payload = WrapperUI::AcceptDragDropPayload("File")) {
				std::string PayloadIndex = *(std::string*)payload->Data;
				auto newPath = PayloadIndex.substr(PayloadIndex.find_last_of('/') + 1);
				newPath = directory + '/' + newPath;
				PrintLog("Changed File %s to %s", PayloadIndex.c_str(), newPath.c_str());
				try
				{
					std::filesystem::rename(PayloadIndex, newPath);
					Core::App::Get().GetEditorUIManager().GetFileExplorer().RefreshThis();
				}
				catch (const std::exception& e)
				{
					PrintError("Failed to Rename file %s to %s, reason : %s", PayloadIndex.c_str(), newPath.c_str(), e.what());
				}

			}
			WrapperUI::EndDragDropTarget();
		}
	}
}

void EditorUI::File::ShowInInspector()
{
	if (!resourceLink)
		return;
	try
	{
		//ResourceLink->ShowInInspector();
	}
	catch (const std::exception&)
	{

	}
}