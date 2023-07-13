#pragma once
#include "PandorAPI.h"

#include <string>
#include <vector>

namespace Component
{
	class BaseComponent;
	class Transform;
	class MeshComponent;
	class LightComponent;
	class DirectionalLight;
	class SpotLight;
	class PointLight;
	class CameraComponent;
}

namespace EditorUI
{
	class EditorUiManager;
}

namespace Render
{
	class Camera;
	class EditorGrid;
	class Gizmo;
}
namespace Resources
{
	class Material;
	class Model;
	class Shader;
	class Bone;
	class Prefab;
	enum class ResourcesType;
}

namespace Core
{
	class GameObject;

	enum class SceneObjectType
	{
		GameObject,
		Bone,
		CustomComponent,
		ResourcesManager,
		Camera,
	};

	class Scene
	{
	private:
		class GameObject* m_sceneNode = nullptr;
		bool m_isCurrentScene = false;
		bool m_isPrefab = false;

		std::unordered_map<uint64_t, GameObject*> m_objectMap;

		std::string name = "";
		Scene* m_savedState = nullptr;

#ifndef PANDOR_GAME
		Render::Camera* m_editorCamera = nullptr;
#endif

		std::vector<Render::Camera*> m_cameraComponents = {};
#ifndef PANDOR_GAME
		EditorUI::EditorUiManager* m_editorUi;
		Render::EditorGrid* m_grid = nullptr;

		std::deque<std::pair<std::string, Resources::ResourcesType>> m_thumbnails;
#endif

		Math::Matrix4 m_projectionMatrix;
		Math::Matrix4 m_viewMatrix;
		Math::Matrix4 m_VP;
		Math::Matrix4 m_ortho;
		Math::Vector3 m_up;
		Math::Vector3 m_right;
		Math::Vector3 m_forward;


		Resources::Prefab* m_parentPrefab = nullptr; // Value set only if the scene is a prefab nullptr otherwise
		friend class Resources::Prefab;
	private:

		void ParseShaderName(std::string name);
		void LoadAllScenesResources(std::fstream& sceneFile);

		SceneObjectType StringToEnum(std::string& str);

		void FindObjects(std::string objectName, GameObject* object,  std::vector<GameObject*>& objectList);

		bool NeededResourcesLoaded(const std::string& path, Resources::Model*& sphere, Resources::Material*& material, Resources::Shader*& displayShader);
		void AddObjectToList(GameObject* object);

	public :
		void ChangeIndexObjectList(GameObject* object, uint64_t uuid);
		void ResetObjectToList(GameObject* object);
		Render::Camera* currentCamera;
		Render::Camera* mainCamera;
#ifndef PANDOR_GAME
		Render::Gizmo* drawGizmo;
#endif

		bool isLoaded = false;
		bool drawGrid = true;
		bool drawIcons = true;
	public:
		friend class SceneManager;
		std::string path;
		Scene(std::string _path);
		~Scene();
#ifndef PANDOR_GAME
		bool RenderMaterialThumbnail(const std::string& path);
		bool RenderModelThumbnail(const std::string& path);
		bool RenderMeshThumbnail(const std::string& path);
		void CreateThumbnail(const std::string& path, Resources::ResourcesType type);
#endif

		void Initialize();

		void BeginPlay();
		void Update();
#ifndef PANDOR_GAME
		void UpdatePrefabScene();
#endif
		void Delete();

		void Load();
		std::string ExtractName(std::string path);
		GameObject* CreateObject();
		Resources::Bone* CreateBoneObject();
		GameObject* CreateObject(std::string name);
		void AddObject(GameObject* gameObject, GameObject* parent = nullptr);
		void RemoveObject(GameObject* gameObject);
		void RemoveObject(std::string objectName);
		GameObject* FindObject(std::string objectName);
		std::vector<GameObject*> FindAllObject(std::string objectName);
#ifndef PANDOR_GAME
		void SaveSceneFile(std::string filePath);
#endif

		void ReadSceneFile(std::string filePath);

#ifndef PANDOR_GAME
		void SaveSceneState();
#endif
		void ReloadSceneState();

		GameObject* GetObjectByID(uint64_t ID);
		GameObject* GetObjectByName(std::string_view name);
		std::unordered_map<uint64_t, GameObject*>& GetObjectMap() { return m_objectMap; }
		
		std::string GetName() { return name; }
		class GameObject* GetSceneNode() { return m_sceneNode; }
		bool GetIsCurrentScene() { return m_isCurrentScene; }
		bool IsPrefabScene() { return m_isPrefab; }
		void SetAsCurrentScene(bool isCurrentScene) { m_isCurrentScene = isCurrentScene; }
		Resources::Prefab* GetPrefab() { if (m_isPrefab) { return m_parentPrefab; } return nullptr; }

#ifndef PANDOR_GAME
		void RenderClickedObject(bool sceneWindow = true);
		Render::Camera* GetEditorCamera() const { return m_editorCamera; }
#endif
		void AddCamera(Render::Camera* camera);
		void RemoveCamera(Render::Camera* camera);
		void SetCurrentCamera(Render::Camera* camera);

		Math::Matrix4 GetProjectionMatrix() const { return m_projectionMatrix; }
		Math::Matrix4 GetViewMatrix() const { return m_viewMatrix; }
		Math::Matrix4 GetVP() const { return m_VP; }
		Math::Matrix4 GetOrtho() const { return m_ortho; }
		Math::Vector3 GetUpVector() const { return m_up; }
		Math::Vector3 GetRightVector() const { return m_right; }

		Resources::Bone* BonePopup(const char* popupName);

	};
}