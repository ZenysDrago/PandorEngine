#pragma once
#include "PandorAPI.h"
namespace Core {
	namespace Wrapper
	{
		class WrapperWindow;
	}
	struct PANDOR_API ProjectData
	{
		std::string ProjectName;
		std::string ProjectPath;
	};


	class PANDOR_API ProjectManager
	{
	public:
		ProjectManager();
		~ProjectManager();

		static void Create();

		void Init();

		std::string Update();

		void Clear();

		static ProjectManager& Get() { return *m_projectManager; }
	private:
		static ProjectManager* m_projectManager;

		std::vector<ProjectData> m_projects = {};

		std::string m_projectPath = "";

		Core::Wrapper::WrapperWindow* window;

		std::string m_tempPath = "";
	private:
		void NewProjectPopup();
		void WriteFile();
		void CreateFiles(const ProjectData& projectData);

		void CreateCsProj(const ProjectData& projectData);

		void Draw(Math::Vector2 pos, Math::Vector2 size);

		void AddProject(std::string& path);

		std::string GetProjects();

	};
}