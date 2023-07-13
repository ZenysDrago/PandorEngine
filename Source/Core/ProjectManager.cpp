#include "pch.h"
#include "Core/ProjectManager.h"
#include <Utils/Loader.h>
#include <Core/App.h>
#include <EditorUI/MainBar.h>

Core::ProjectManager* Core::ProjectManager::m_projectManager;

std::string GetLine(const char* data, uint32_t& pos)
{
	std::string line;
	while (data[pos] != '\0' && data[pos] != '\n')
	{
		line.push_back(data[pos]);
		pos++;
	}
	if (line[0] == '\t')
		line = line.substr(line.find_first_not_of('\t'));
	return line;
}

std::string GetString(std::string line)
{
	line = line.substr(line.find_first_of(' '));
	line = line.substr(line.find_first_of(':') + 2);
	line = line.substr(0, line.find_first_of('\r'));
	line = line.substr(0, line.find_first_of('\n'));
	return line;
}


Core::ProjectManager::ProjectManager()
{

}

Core::ProjectManager::~ProjectManager()
{
}

void Core::ProjectManager::Create()
{
	m_projectManager = new Core::ProjectManager();

	m_projectManager->Init();
}

void Core::ProjectManager::Init()
{
	auto data = Utils::Loader::ReadFile("Projects.txt");
	uint32_t pos = 0;
	std::string currentLine;
	while (pos < data.size())
	{
		currentLine = GetLine(data.c_str(), pos);
		if (currentLine.substr(0, 4) == "Name")
		{
			m_projects.push_back(ProjectData());
			m_projects.back().ProjectName = GetString(currentLine);
		}
		if (currentLine.substr(0, 4) == "Path")
		{
			m_projects.back().ProjectPath = GetString(currentLine);
			if (!std::filesystem::exists(m_projects.back().ProjectPath))
				m_projects.erase(m_projects.begin() + m_projects.size() - 1);
		}
		pos++;
	}

	AppInit init =
	{
		1600, 900,
		4,5,
		"Pandor Hub",
		true,
	};

	window = WrapperWindow::CreateInstance();
	window->Initialize(init);
	WrapperUI::Initialize(window->GetWindow());
}

std::string Core::ProjectManager::Update()
{
	while (!window->ShouldClose() && m_projectPath.empty())
	{
		window->PollEvents();
		WrapperUI::NewFrame();

		Draw(window->GetWindowPos(), window->GetSize());

		WrapperUI::EndFrame();
		window->SwapBuffers();
	}
	WriteFile();
	return m_projectPath;
}

void Core::ProjectManager::NewProjectPopup()
{
	if (WrapperUI::BeginPopupModal("ProjectCreation"))
	{
		const char* buttonName = (m_tempPath.empty() ? "Set Path" : m_tempPath.c_str());
		WrapperUI::SetNextItemWidth(WrapperUI::GetContentRegionAvail().x);
		if (WrapperUI::Button(buttonName))
		{
			auto string = Utils::SaveFile("", window);
			m_tempPath = string;
		}
		if (!m_tempPath.empty() && WrapperUI::Button("Create Project")) {
			m_projects.push_back(ProjectData());
			m_projects.back().ProjectName = m_tempPath.substr(m_tempPath.find_last_of('\\') + 1);
			m_projects.back().ProjectPath = m_tempPath;
			WriteFile();
			CreateFiles(m_projects.back());
			WrapperUI::CloseCurrentPopup();
		}
		if (WrapperUI::Button("Close"))
		{
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
}

void Core::ProjectManager::WriteFile()
{
	std::ofstream _file("Projects.txt"); // Open file with append mode
	if (_file) {
		std::string content;
		content = GetProjects();
		_file.write(content.c_str(), content.size());
		_file.close();
	}
}

std::string csprojContent = R"(
<Project Sdk="Microsoft.NET.Sdk">

	<PropertyGroup>
		<TargetFramework>netstandard2.1</TargetFramework>
		<RootNamespace>Sandbox</RootNamespace>
	</PropertyGroup>

	<PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|AnyCPU'">
	  <OutputPath>bin/</OutputPath>
	</PropertyGroup>

	<ItemGroup>
		<Reference Include="Pandor-ScriptCore">
			<HintPath>%s</HintPath>
		</Reference>
	</ItemGroup>

</Project>
)";

std::string slnContent = R"(Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 16
VisualStudioVersion = 16.0.33130.400
MinimumVisualStudioVersion = 10.0.40219.1
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = "%s", "%s.csproj", "{3A94E5DE-2459-4D33-AE9A-67F838EB2616}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{3A94E5DE-2459-4D33-AE9A-67F838EB2616}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		{3A94E5DE-2459-4D33-AE9A-67F838EB2616}.Debug|Any CPU.Build.0 = Debug|Any CPU
		{3A94E5DE-2459-4D33-AE9A-67F838EB2616}.Release|Any CPU.ActiveCfg = Release|Any CPU
		{3A94E5DE-2459-4D33-AE9A-67F838EB2616}.Release|Any CPU.Build.0 = Release|Any CPU
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {B86C4845-8B9A-44C8-B0B6-0327EF56E284}
	EndGlobalSection
EndGlobal
)";

void Core::ProjectManager::CreateFiles(const ProjectData& projectData)
{
	std::filesystem::create_directory(projectData.ProjectPath);
	std::filesystem::create_directory(projectData.ProjectPath + "/Assets");
	CreateCsProj(projectData);

	// Replace placeholders with actual values
	size_t placeholderPos = slnContent.find("%s");
	while (placeholderPos != std::string::npos) {
		slnContent.replace(placeholderPos, 2, projectData.ProjectName);
		placeholderPos = slnContent.find("%s", placeholderPos + projectData.ProjectName.length());
	}

	// Replace the second occurrence of "%s" with projectFilename
	placeholderPos = slnContent.find("%s");
	while (placeholderPos != std::string::npos) {
		slnContent.replace(placeholderPos, 2, projectData.ProjectName);
		placeholderPos = slnContent.find("%s", placeholderPos + projectData.ProjectName.length());
	}

	// Create the .sln file
	std::ofstream slnFile(projectData.ProjectPath + '/' + projectData.ProjectName + ".sln");
	if (slnFile.is_open()) {
		slnContent = Utils::StringFormat(slnContent, projectData.ProjectName.c_str());
		slnFile << slnContent << std::endl;
		slnFile.close();
		PrintLog(std::string(projectData.ProjectName + ".sln file created successfully.\n").c_str());
	}
	else {
		PrintError(std::string(projectData.ProjectName + ".sln file created failed.\n").c_str());
		return;
	}
}

void Core::ProjectManager::CreateCsProj(const ProjectData& projectData)
{
	// Replace placeholders with actual values
	size_t placeholderPos = csprojContent.find("%s");
	while (placeholderPos != std::string::npos) {
		csprojContent.replace(placeholderPos, 2, std::filesystem::current_path().string() + "/Resources/Scripts/netstandard2.1/Pandor-ScriptCore.dll");
		placeholderPos = csprojContent.find("%s", placeholderPos + projectData.ProjectName.length());
	}
	// Create the .csproj file
	std::ofstream csprojFile(projectData.ProjectPath + '/' + projectData.ProjectName + ".csproj");
	if (csprojFile.is_open()) {
		csprojFile << csprojContent << std::endl;
		csprojFile.close();
		PrintLog(std::string(projectData.ProjectName + ".csproj file created successfully.\n").c_str());
	}
	else {
		PrintError(std::string(projectData.ProjectName + ".csproj file created failed.\n").c_str());
		return;
	}
}

std::string Core::ProjectManager::GetProjects()
{
	std::string content;
	for (auto&& proj : m_projects)
	{
		content += "------------------------------\n";
		content += Utils::StringFormat("Name : %s\n", proj.ProjectName.c_str());
		content += Utils::StringFormat("Path : %s\n", proj.ProjectPath.c_str());
	}
	return content;
}

void Core::ProjectManager::Draw(Math::Vector2 pos, Math::Vector2 size)
{
	WrapperUI::Begin("Project", (bool*)true, (WindowFlags)((int)WindowFlags::NoDecoration | (int)WindowFlags::NoMove));
	WrapperUI::SetWindowPos(pos);
	WrapperUI::SetWindowSize(size);
	if (WrapperUI::BeginChild("Sidebar")) {
		WrapperUI::SetWindowFontScale(2.f);
		WrapperUI::TextUnformatted("Projects");
		WrapperUI::SetWindowFontScale(1.f);
		if (WrapperUI::Button("New Project"))
		{
			WrapperUI::OpenPopup("ProjectCreation");
		}
		WrapperUI::SameLine();
		if (WrapperUI::Button("Add Project"))
		{
			std::string path = Utils::OpenFile(".sln", window);
			if (!path.empty())
			{
				path = path.substr(0, path.find_last_of('\\'));
				AddProject(path);
			}
		}
		NewProjectPopup();

		static TextFilter filter;
		filter.Draw();

		WrapperUI::Separator();

		int i = 0;
		for (auto&& proj : m_projects)
		{
			WrapperUI::PushID(i++);
			if (!filter.PassFilter(proj.ProjectName.c_str())) continue;
			if (WrapperUI::Button(proj.ProjectName.c_str(), Vector2(WrapperUI::GetWindowWidth() - 100, 100.f)))
			{
				m_projectPath = proj.ProjectPath;
				CreateCsProj(proj);
			}
			WrapperUI::SameLine();
			if (WrapperUI::Button("Destroy"))
			{
				m_projects.erase(m_projects.begin() + i - 1);
				WrapperUI::PopID();
				break;
			}
			WrapperUI::PopID();
		}

		WrapperUI::EndChild();
	}
	WrapperUI::End();
}
#include <algorithm>

void Core::ProjectManager::AddProject(std::string& path)
{
	auto it = std::find_if(m_projects.begin(), m_projects.end(), [path](const ProjectData& s) {
		return s.ProjectPath == path;
		});

	// Return true if the value is found, false otherwise
	if (it != m_projects.end())
		return;

	m_projects.push_back(ProjectData());
	m_projects.back().ProjectName = path.substr(path.find_last_of('\\') + 1);
	m_projects.back().ProjectPath = path;
}

void Core::ProjectManager::Clear()
{
	WrapperUI::Destroy();

	window->Terminate();
	delete window;
	window = nullptr;
}

