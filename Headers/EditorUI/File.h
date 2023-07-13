#pragma once
#include "PandorAPI.h"

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

namespace Resources
{
	class Texture;
	class IResources;
}

namespace EditorUI {
	enum class PANDOR_API FileType
	{
		Folder,
		None,
		Txt,
		Img,
		Mat,
		Mtl,
		Model,
		Scn,
		Shdr,
		Phm,
		Anim,
		AnimC,
		Cbm,
		Prefab,
	};

	class PANDOR_API File
	{
	public:
		std::string name;
		std::string directory;
		FileType type = FileType::None;
		std::vector<std::shared_ptr<File>> childrens;
		bool selected = false;

		Resources::Texture* icon = nullptr;
		Resources::IResources* resourceLink = nullptr;
		bool hovered = false;
	public:
		File();
		File(std::string filename);
		~File();

		void FoundChildren();
		void FindAllChilden();
		std::shared_ptr<File> GetParent();

		void DrawInFileExplorer(File*& clicked);

		void ShowInInspector();

	};
}