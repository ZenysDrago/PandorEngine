#pragma once
#include "PandorAPI.h"

#include <EditorUI/EditorWindow.h>
#include <Resources/ResourcesManager.h>
#include <memory>
#include <string>
#include <vector>

namespace EditorUI
{
	class PANDOR_API FileExplorer : public EditorWindow
	{
	private:

		std::string m_path = Resources::ResourcesManager::AssetPath();
		std::shared_ptr<class File> m_current = nullptr;
		std::shared_ptr<class File> m_main = nullptr;
		std::shared_ptr<class File> m_rightClicked = nullptr;
		std::shared_ptr<class File> m_renameFile = nullptr;

		bool m_visible = false;
	public:
		FileExplorer();
		~FileExplorer();

		void Draw() override;

		void Refresh();

		void RefreshThis();

		void RightClickWindow();

		void Drop(const char** path, int count);

		static std::vector<std::string> FindFiles(const std::string& folderName, const std::string& extension); // Go through all files of this folder

	};
}