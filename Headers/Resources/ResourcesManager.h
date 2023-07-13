#pragma once
#include "PandorAPI.h"

#include "IResources.h"
#include <unordered_map>
#include <Core/ThreadManager.h>
#include <Core/Wrappers/WrapperRHI.h>
#include <vector>
#define ENGINEPATH (std::string)"EngineResources/" + 

namespace Resources
{
	class Texture;
	class Shader;
	class Skybox;

	class PANDOR_API ResourcesManager
	{
	private:
		static  ResourcesManager* m_instance;

		std::unordered_map<std::string, IResources*> resourcesList;
		Core::ThreadManager* threadManager;

		Shader* defaultShader = nullptr;
		Shader* unlitShader = nullptr;
		Shader* skyboxShader = nullptr;
		static std::string m_engineResourcesPath;
		static std::string m_assetsPath;
		static std::string m_projectName;
		static std::string m_projectPath;

		ResourcesManager(Core::ThreadManager* _threadManager);

		std::string GetFileName(const std::string& path);
	public:
		static ResourcesManager* Get() { return m_instance; }
		static void CreateInstance(Core::ThreadManager* _threadManager, const std::string& projectPath, const std::string& projectName) {
			m_instance = new ResourcesManager(_threadManager);
			m_instance->m_projectPath = projectPath;
			m_instance->m_assetsPath = projectPath + "/Assets/";
			m_instance->m_projectName = projectName;
		}

		static std::string AssetPath() { return m_assetsPath; }
		static std::string ProjectName() { return m_projectName; }
		static std::string ProjectPath() { return m_projectPath; }
		static std::string EnginePath() { return m_engineResourcesPath; }

		ResourcesManager(const ResourcesManager&) = delete;
		~ResourcesManager();

		std::unordered_map<std::string, IResources*> GetAllResources();
		template <typename T> inline std::vector<T*> GetAllResourcesOfType();

		template <typename T> void Add(std::string name, T* resource);

		// Create a new Resource
		template <typename T> T* Create(std::string name);

		// Specific create behavior for the texture resources
		Resources::Texture* Create(std::string name, unsigned int _texType = PR_TEXTURE2D, unsigned int _slot = PR_TEXTURE0, unsigned int _pixelType = PR_UNSIGNED_BYTE);
		// Specific create behavior for the Skybox resources
		Resources::Skybox* Create(std::string name, std::string faces[6]);
		// Specific create behavior for the shader resources
		Resources::Shader* Create(std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

		// Find Resource if doesn't exist create it
		template <typename T> T* GetOrLoad(std::string name);

		template <typename T> T* Reload(std::string name);

		void Rename(const std::string& prevName, const std::string& newName, bool directory = false);

		// Find Resource if doesn't exist return nullptr
		template <typename T> T* Find(std::string name);
		// Draw of the ResourcePopup
		template <typename T> T* ResourcePopup(const std::string& popupName);

		class Material* MaterialPopup(const std::string& popupName);

		class Mesh* MeshPopup(const std::string& popupName);

		class Model* ModelPopup(const std::string& popupName);

		Resources::Texture* TexturePopup(const char* popupName);

		bool Contain(const std::string& popupName);

		void Delete(const std::string& name);
		void DeleteAll();

		void FilesLoad(const std::string& path);

		void ResourceLoad(const std::string& path);

		bool IsEverythingLoaded();
		bool IsEverythingSent();

		void SetupInitalResources();
		void LoadNecessaryResources();

		friend std::ostream& operator<<(std::ostream& os, const ResourcesManager& rm)
		{
			os << "===" << "\n";
			os << "ResourcesManager" << "\n";

			for (auto resource : rm.resourcesList)
			{
				os << (int)resource.second->GetType();
				os << "\n";
				os << resource.second->GetPath();
				os << "\n";
			}

			os << "end" << "\n";
			return os;
		}

		//Getters
		Shader* GetDefaultShader();
		Shader* GetUnlitShader();
		Shader* GetSkyboxShader();
	};

}

#include "ResourcesManager.inl"
