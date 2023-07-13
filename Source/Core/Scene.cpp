#include "pch.h"

#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Core/App.h>
#include <Core/GameObject.h>
#include <Resources/Skeleton.h>
#include <Core/Wrappers/WrapperAudio.h>

#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/Inspector.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/GameWindow.h>
#include <EditorUI/PrefabWindow.h>
#endif

#include <Render/Camera.h>

#ifndef PANDOR_GAME
#include <Render/EditorGrid.h>
#include <Render/Gizmo.h>
#endif

#include <Render/FrameBuffer.h>

#include <LowRenderer/LightManager.h>

#include <Resources/Model.h>
#include <Resources/Mesh.h>
#include <Resources/SkeletalMesh.h>
#include <Resources/IResources.h>
#include <Resources/Material.h>
#include <Resources/Prefab.h>
#include <Resources/Mesh.h>

#include <Components/LightComponent.h>
#include <Components/CameraComponent.h>
#include <Components/ComponentsData.h>
#include <Components/ShaderComponent.h>
#include <Components/MeshComponent.h>
#include <Components/SkeletalMeshComponent.h>
#include <Components/ScriptComponent.h>
#include <Components/Rigidbody.h>
#include <Components/BoxCollider.h>

#include <Scripting/ScriptEngine.h>

#include <regex>

using namespace Core;

Core::Scene::Scene(std::string _path)
{
	path = _path;
	name = ExtractName(_path);
}

Core::Scene::~Scene()
{
}

#ifndef PANDOR_GAME
bool Core::Scene::RenderMaterialThumbnail(const std::string& path)
{
	Resources::Model* model = nullptr;
	Resources::Material* material = nullptr;
	Resources::Shader* shader = nullptr;
	Resources::Shader* previousShader = nullptr;

	if (NeededResourcesLoaded(path, model, material, shader)) {
		PrintLog("Rendering %s material thumbnail", path.c_str());

		// Initialize the scene
		GameObject* gameObject;
		auto cameraGO = new GameObject("Camera");
		auto camera = new Component::CameraComponent();
		auto light = new Component::DirectionalLight();
		camera->SetAspectRatio(1.f);
		cameraGO->AddComponent(camera);
		auto lightNode = new GameObject("Light");
		lightNode->AddComponent(light);
		lightNode->transform->SetLocalPosition({ 0, 90, 0 });
		lightNode->transform->SetLocalRotation(Vector3(135, 0, -35));
		float size = (model->GetMaxVerticeOnXYZ() * 0.75f);
		cameraGO->transform->SetLocalPosition(Math::Vector3(0, 0, -3));
		currentCamera = camera;
		m_VP = currentCamera->GetVPMatrix();
		camera->ClearColor = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);
		gameObject = model->ToGameObject();
		auto meshComps = gameObject->GetComponentsInChildren<Component::MeshComponent>();

		if (auto meshComp = meshComps[0]) {
			previousShader = material->GetShader();
			material->SetShader(shader, false);
			meshComp->GetMaterials()[0] = material;
		}

		LowRenderer::LightManager::getInstance().SetLightsUniforms(material->GetShader());
		Core::Wrapper::WrapperRHI::ShaderSendVec3(material->GetShader()->GetLocation("viewPos"), currentCamera->GetTransform()->GetWorldPosition());

		// Render
		camera->PreUpdate(1.f);
		WrapperRHI::ClearColorAndBuffer(camera->ClearColor);
		gameObject->DrawSelfAndChild(false);

		material->SetShader(previousShader, false);

		camera->PostUpdate();

		auto filePath = std::regex_replace(path, std::regex("/"), "~");
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
		camera->framebuffer->SaveFrameBufferToTexture(thumbnailPath.c_str(), { 256, 256 });
		delete gameObject;
		gameObject = nullptr;
		delete cameraGO;
		delete lightNode;
		cameraGO = nullptr;
		return true;
	}
	return false;
}

bool Scene::RenderModelThumbnail(const std::string& path)
{
	// Load model
	auto model = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(path);
	if (!model || !model->HasBeenSent()) {
		return false;
	}

	// Initialize scene
	auto CameraGO = new GameObject("Camera");
	auto camera = new Component::CameraComponent();
	auto light = new Component::DirectionalLight();
	camera->SetAspectRatio(1.f);
	CameraGO->AddComponent(camera);
	auto lightNode = new GameObject("Light");
	lightNode->AddComponent(light);
	lightNode->transform->SetLocalPosition({ 0, 90, 0 });
	lightNode->transform->SetLocalRotation(Vector3(135, 0, -35));
	float size = (model->GetMaxVerticeOnXYZ() * 0.75f);
	CameraGO->transform->SetLocalPosition(Math::Vector3(-3, 4, -3) * size);
	CameraGO->transform->SetLocalRotation(Math::Vector3(45.f, 45.f, 0));
	currentCamera = camera;
	m_VP = currentCamera->GetVPMatrix();
	camera->ClearColor = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);

	// Set shader and materials
	auto GO = model->ToGameObject();
	auto MeshComps = GO->GetComponentsInChildren<Component::MeshComponent>();
	for (auto&& meshComp : MeshComps) {
		for (auto&& mat : meshComp->GetMaterials()) {
			if (!mat || !mat->GetShader())
				continue;
			LowRenderer::LightManager::getInstance().SetLightsUniforms(mat->GetShader());
			Core::Wrapper::WrapperRHI::ShaderSendVec3(mat->GetShader()->GetLocation("viewPos"), currentCamera->GetTransform()->GetWorldPosition());
		}
	}

	// Render
	camera->PreUpdate(1.f);
	WrapperRHI::ClearColorAndBuffer(camera->ClearColor);
	GO->DrawSelfAndChild(false);
	camera->PostUpdate();

	// Save thumbnail
	auto filePath = std::regex_replace(path, std::regex("/"), "~");
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
	camera->framebuffer->SaveFrameBufferToTexture(thumbnailPath.c_str(), { 256, 256 });
	// Clean up
	delete CameraGO;
	CameraGO = nullptr;
	delete GO;
	GO = nullptr;
	delete lightNode;

	return true;
}
bool Core::Scene::NeededResourcesLoaded(const std::string& path, Resources::Model*& sphere, Resources::Material*& material, Resources::Shader*& displayShader)
{
	sphere = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(ENGINEPATH"Models/Sphere.obj");
	material = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(path);
	if (material && material->IsLoaded()) {
		if (!material->GetShader() || !material->GetShader()->GetFrag())
			return false;
		auto name = Utils::StringFormat("%s + %s", Resources::ResourcesManager::Get()->GetDefaultShader()->GetVertex()->GetPath().c_str(), material->GetShader()->GetFrag()->GetPath().c_str());
		displayShader = Resources::ResourcesManager::Get()->Find<Resources::Shader>(name);
		if (!displayShader) {
			displayShader = Resources::ResourcesManager::Get()->Create(name, Resources::ResourcesManager::Get()->GetDefaultShader()->GetVertex()->GetPath(), material->GetShader()->GetFrag()->GetPath());
			displayShader->SetName(Utils::StringFormat("%s + %s", Resources::ResourcesManager::Get()->GetDefaultShader()->GetVertex()->GetName().c_str(), material->GetShader()->GetFrag()->GetName()));
			Core::App::Get().AddResourceToSend(name);
		}
	}
	return (sphere && sphere->HasBeenSent() && material && material->IsLoaded() && displayShader->HasBeenSent());
}
#endif

void Core::Scene::Initialize()
{
	name = ExtractName(path);

}

#ifndef PANDOR_GAME
void Core::Scene::RenderClickedObject(bool sceneWindow /*= true*/)
{
	if (auto gameObject = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected())
	{
		if (gameObject->GetScene()->m_isPrefab && sceneWindow || !gameObject->GetScene()->m_isPrefab && !sceneWindow)
			return;

		if (auto meshComp = gameObject->GetComponent<Component::MeshComponent>())
		{
			if (auto mesh = meshComp->GetMesh())
			{
				mesh->Render(gameObject->transform->GetModelMatrix() * Core::App::Get().sceneManager->GetCurrentScene()->GetVP(), gameObject->transform->GetModelMatrix(), meshComp->GetMaterials(), false, true, true);
			}
		}
		else if (auto skelMeshComp = gameObject->GetComponent<Component::SkeletalMeshComponent>())
		{
			if (auto skelMesh = skelMeshComp->GetMesh())
			{
				skelMesh->Render(gameObject->transform->GetModelMatrix(), skelMeshComp->GetMaterials(), skelMeshComp->GetSkeleton(), true);
			}
		}
	}
}
#endif

Resources::Bone* Scene::BonePopup(const char* popupName)
{

	Resources::Bone* out = nullptr;
	int id = 0;
	if (WrapperUI::BeginPopupModal(popupName, (bool*)0, WindowFlags::AlwaysAutoResize)) {
		static TextFilter filter;
		filter.Draw();
		for (auto& go : m_sceneNode->GetAllChildren())
		{
			if (auto bone = dynamic_cast<Resources::Bone*>(go)) {

				if (!filter.PassFilter(bone->GetName().c_str()) || !bone->GetSkeleton() || bone->GetSkeleton()->RootBone != bone)
					continue;
				WrapperUI::PushID(id++);
				if (WrapperUI::Selectable(bone->GetName().c_str(), false, SelectableFlags::SpanAllColumns)) {
					out = bone;
					WrapperUI::CloseCurrentPopup();
				}
				if (WrapperUI::IsItemHovered())
				{
					WrapperUI::BeginTooltip();
					WrapperUI::TextUnformatted(bone->GetParent()->GetName().c_str());
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

void Scene::SetCurrentCamera(Render::Camera* camera)
{
	currentCamera = camera;
	if (currentCamera) {
		currentCamera->CreateFrustum();
		m_viewMatrix = currentCamera->GetViewMatrix();
		m_projectionMatrix = currentCamera->GetProjectionMatrix();
		m_VP = m_viewMatrix * m_projectionMatrix;
		m_ortho = currentCamera->GetOrtho();
		auto worldRot = currentCamera->GetTransform()->GetWorldRotation();
		m_up = worldRot * Vector3::Up();
		m_right = worldRot * Vector3::Right();
		m_forward = worldRot * Vector3::Forward();
	}
}

#ifndef PANDOR_GAME
bool Scene::RenderMeshThumbnail(const std::string& path)
{
	auto mesh = Resources::ResourcesManager::Get()->Find<Resources::Mesh>(path);
	if (mesh && mesh->GetModel()->HasBeenSent())
	{
		// Initialize scene
		auto model = mesh->GetModel();
		auto CameraGO = new GameObject("Camera");
		auto camera = new Component::CameraComponent();
		auto light = new Component::DirectionalLight();
		camera->SetAspectRatio(1.f);
		CameraGO->AddComponent(camera);
		auto lightNode = new GameObject("Light");
		lightNode->AddComponent(light);
		lightNode->transform->SetLocalPosition({ 0, 90, 0 });
		lightNode->transform->SetLocalRotation(Vector3(135, 0, -35));
		float size = (model->GetMaxVerticeOnXYZ() * 0.75f);
		CameraGO->transform->SetLocalPosition(Math::Vector3(-3, 4, -3) * size);
		CameraGO->transform->SetLocalRotation(Math::Vector3(45.f, 45.f, 0));
		currentCamera = camera;
		m_VP = currentCamera->GetVPMatrix();
		camera->ClearColor = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f);

		auto GOMesh = new Core::GameObject();
		// Set shader and materials
		auto MeshComps = new Component::MeshComponent();
		GOMesh->AddComponent(MeshComps);
		auto mat = model->GetMaterialsForMesh(mesh);
		for (int i = 0; i < mat.size(); i++)
		{
			MeshComps->AddMaterial(mat[i]);
		}
		MeshComps->SetMesh(mesh);
		for (auto&& mat : MeshComps->GetMaterials()) {
			if (!mat || !mat->GetShader())
				continue;
			LowRenderer::LightManager::getInstance().SetLightsUniforms(mat->GetShader());
			Core::Wrapper::WrapperRHI::ShaderSendVec3(mat->GetShader()->GetLocation("viewPos"), currentCamera->GetTransform()->GetWorldPosition());
		}

		// Render
		camera->PreUpdate(1.f);
		WrapperRHI::ClearColorAndBuffer(camera->ClearColor);
		GOMesh->DrawSelfAndChild(false);
		camera->PostUpdate();

		// Save thumbnail
		auto filePath = std::regex_replace(path, std::regex("/"), "~");
		filePath = std::regex_replace(filePath, std::regex(":"), "`");
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
		camera->framebuffer->SaveFrameBufferToTexture(thumbnailPath.c_str(), { 256, 256 });

		// Clean up
		delete lightNode;
		delete GOMesh;
		GOMesh = nullptr;
		delete CameraGO;
		CameraGO = nullptr;
		return true;
	}
	return false;
}

void Core::Scene::CreateThumbnail(const std::string& path, Resources::ResourcesType type)
{
	auto newPath = path;
	Utils::ToPath(newPath);
	Core::App::Get().threadManager->Lock();
	m_thumbnails.push_front(std::make_pair(path, type));
	Core::App::Get().threadManager->Unlock();
}
#endif

void Scene::RemoveCamera(Render::Camera* camera)
{
	auto it = std::find(m_cameraComponents.begin(), m_cameraComponents.end(), camera);
	if (it != m_cameraComponents.end()) {
		m_cameraComponents.erase(it);
		if (camera == mainCamera)
		{
			mainCamera = nullptr;
		}
	}
	if (!mainCamera && !m_cameraComponents.empty())
	{
		m_cameraComponents.front()->SetAsMainCamera();
	}
}

void Scene::AddCamera(Render::Camera* camera)
{
	if (m_cameraComponents.empty())
	{
		camera->SetAsMainCamera();
	}
	m_cameraComponents.push_back(camera);
}

void Core::Scene::BeginPlay()
{
	m_sceneNode->Awake();
	m_sceneNode->Start();
}

void Core::Scene::Update()
{
	Core::SceneManager::Get()->SetCurrentScene(this);

#ifdef PANDOR_GAME
	while (!ResourcesManager::Get()->IsEverythingLoaded() && !ResourcesManager::Get()->IsEverythingSent()) {}
	if (Core::App::Get().GetGameState() != GameState::Play)
	{
		Core::App::Get().SetGameState(GameState::Play);
	}
#endif
	size_t index = 0;
	m_sceneNode->UpdateSelfAndChild(index);

	Core::App::Get().physic->Update();

#ifndef PANDOR_GAME
	auto size = Core::App::Get().GetEditorUIManager().GetSceneWindow().GetWindowSize();
	auto mouseWinPos = Core::App::Get().GetEditorUIManager().GetSceneWindow().GetMousePosition();

	if (GetEditorCamera()->IsVisible()) {
		SetCurrentCamera(GetEditorCamera());
		LowRenderer::LightManager::getInstance().SendLightToAllShaders();

		WrapperRHI::ClearColorAndBuffer({ 0.1f, 0.1f, 0.1f, 1.0f });

		Core::App::Get().shadowMap->BeginShadowMapGeneration();
		m_sceneNode->DrawModelForShadow();
		Core::App::Get().shadowMap->EndShadowMapGeneration((uint32_t)Core::App::Get().window->GetSize().x, (uint32_t)Core::App::Get().window->GetSize().y);

		currentCamera->PreUpdate(size.x / size.y);
		WrapperRHI::DepthActive();
		WrapperRHI::ClearColorAndBuffer(currentCamera->ClearColor);

		currentCamera->DrawSkybox();

		currentCamera->Inputs();

		mouseWinPos *= Core::App::Get().window->GetSize() / size;

		if (currentCamera->PickingUpdate()) {

			unsigned char data[4];
			WrapperRHI::PushToGPU(data, (int)mouseWinPos.x, (int)mouseWinPos.y);

			currentCamera->ReadPickingID(data);
		}

		if (drawGrid)
			m_grid->Draw();

		RenderClickedObject();


		m_sceneNode->DrawSelfAndChild(true);

		if (Core::GameObject* gameObject = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected())
		{
			if (gameObject->GetScene() == this) {
				WrapperRHI::DepthRange({ 0, 0.1f });
				drawGizmo->Draw(gameObject->transform);
				WrapperRHI::DepthRange({ 0.2f, 1 });
			}
		}

		Core::App::Get().GetEditorUIManager().Draw();

		currentCamera->PostUpdate();
	}
	size = Core::App::Get().GetEditorUIManager().GetGameWindow().GetSize();
#else
	auto size = Core::App::Get().window->GetSize();
#endif
	for (auto&& camera : m_cameraComponents)
	{
		if (!camera->IsVisible())
			continue;
		SetCurrentCamera(camera);

		LowRenderer::LightManager::getInstance().SendLightToAllShaders();

		currentCamera->PreUpdate(size.x / size.y);
		WrapperRHI::DepthActive();
		WrapperRHI::ClearColorAndBuffer(currentCamera->ClearColor);

		currentCamera->DrawSkybox();
		m_sceneNode->DrawSelfAndChild(false);

		currentCamera->PostUpdate();
	}
#ifndef PANDOR_GAME
	if (m_thumbnails.size() != 0) {
		if (m_thumbnails.front().second == ResourcesType::Material) {
			if (RenderMaterialThumbnail(m_thumbnails.front().first))
			{
				Core::App::Get().threadManager->Lock();
				m_thumbnails.pop_front();
				Core::App::Get().threadManager->Unlock();
				SetCurrentCamera(m_editorCamera);
			}
		}
		else if (m_thumbnails.front().second == ResourcesType::Model) {
			if (RenderModelThumbnail(m_thumbnails.front().first))
			{
				Core::App::Get().threadManager->Lock();
				m_thumbnails.pop_front();
				Core::App::Get().threadManager->Unlock();
				SetCurrentCamera(m_editorCamera);
			}
		}
		else if (m_thumbnails.front().second == ResourcesType::Mesh)
		{
			if (RenderMeshThumbnail(m_thumbnails.front().first))
			{
				Core::App::Get().threadManager->Lock();
				m_thumbnails.pop_front();
				Core::App::Get().threadManager->Unlock();
				SetCurrentCamera(m_editorCamera);
			}
		}
		else if (m_thumbnails.front().second == ResourcesType::Mesh)
		{
			if (RenderMeshThumbnail(m_thumbnails.front().first))
			{
				Core::App::Get().threadManager->Lock();
				m_thumbnails.pop_front();
				Core::App::Get().threadManager->Unlock();
				SetCurrentCamera(m_editorCamera);
			}
		}
	}
#endif
}

#ifndef PANDOR_GAME
void Core::Scene::UpdatePrefabScene()
{
	if (m_parentPrefab->exitPopup)
		m_parentPrefab->ExitPrefabPopup();

	if (GetEditorCamera()->IsVisiblePrefab()) {
		Core::SceneManager::Get()->SetCurrentScene(this);

		size_t index = 0;
		m_sceneNode->UpdateSelfAndChild(index);

		auto size = Core::App::Get().GetEditorUIManager().GetPrefabWindow().GetWindowSize();
		auto mouseWinPos = Core::App::Get().GetEditorUIManager().GetPrefabWindow().GetMousePosition();

		SetCurrentCamera(GetEditorCamera());
		LowRenderer::LightManager::getInstance().SendLightToAllShaders();

		currentCamera->PreUpdate(size.x / size.y);
		WrapperRHI::DepthActive();
		WrapperRHI::ClearColorAndBuffer(currentCamera->ClearColor);

		currentCamera->Inputs(false);

		mouseWinPos *= Core::App::Get().window->GetSize() / size;

		if (currentCamera->PickingUpdate(false)) {

			unsigned char data[4];
			WrapperRHI::PushToGPU(data, (int)mouseWinPos.x, (int)mouseWinPos.y);

			currentCamera->ReadPickingID(data, false);
		}

		if (drawGrid)
			m_grid->Draw();

		RenderClickedObject(false);

		m_sceneNode->DrawSelfAndChild(true);

		if (Core::GameObject* gameObject = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected())
		{
			if (gameObject->GetScene() == this) {
				WrapperRHI::DepthRange({ 0, 0.1f });
				drawGizmo->Draw(gameObject->transform, false);
				WrapperRHI::DepthRange({ 0.2f, 1 });
			}
		}

		currentCamera->PostUpdate();
	}
}
#endif

void Core::Scene::Delete()
{
	m_parentPrefab = nullptr;
	isLoaded = false;
#ifndef PANDOR_GAME
	delete m_grid;
	m_grid = nullptr;
#endif
	delete m_sceneNode;
	m_sceneNode = nullptr;
#ifndef PANDOR_GAME
	delete m_editorCamera;
	m_editorCamera = nullptr;
	if (drawGizmo)
	{
		drawGizmo->Delete();
		delete drawGizmo;
		drawGizmo = nullptr;
	}
#endif
}

GameObject* Core::Scene::CreateObject()
{
	GameObject* gameObj = new GameObject();
	AddObjectToList(gameObj);
	return gameObj;
}

Resources::Bone* Core::Scene::CreateBoneObject()
{
	Bone* gameObj = new Bone();
	AddObjectToList(gameObj);
	return gameObj;
}

GameObject* Core::Scene::CreateObject(std::string name)
{
	GameObject* gameObj = new GameObject(name);
	AddObjectToList(gameObj);
	return gameObj;
}

void Core::Scene::AddObjectToList(GameObject* object)
{
	uint64_t index;
	for (index = 0; index < m_objectMap.size(); index++)
	{
		if (m_objectMap.count(index + 1))
			continue;
		break;
	}
	object->uuid = index + 1;
	m_objectMap[index + 1] = object;
}

void Scene::ChangeIndexObjectList(GameObject* gameObject, uint64_t uuid)
{
	if (uuid == 0)
		return;

	if (m_objectMap.count(uuid) && m_objectMap.at(uuid) != gameObject)
	{
		return;
	}
	else
	{
		for (auto& [id, object] : m_objectMap)
		{
			if (object == gameObject)
			{
				m_objectMap.erase(id);
				break;
			}
		}
		m_objectMap[uuid] = gameObject;
		gameObject->uuid = uuid;
	}
}

void Scene::ResetObjectToList(GameObject* object)
{
	RemoveObject(object);
	AddObjectToList(object);
}

void Core::Scene::AddObject(GameObject* gameObject, GameObject* parent)
{
	if (parent)
		parent->AddChildren(gameObject);
	else if (!m_sceneNode)
		m_sceneNode = gameObject;
	else
		m_sceneNode->AddChildren(gameObject);

	gameObject->SetScene(this);
}

void Core::Scene::RemoveObject(GameObject* gameObject)
{
	for (auto& [id, object] : m_objectMap)
	{
		if (object == gameObject)
		{
			m_objectMap.erase(id);
			gameObject->uuid = 0;
			break;
		}
	}
}

void Core::Scene::RemoveObject(std::string objectName)
{
	GameObject* object = FindObject(objectName);
	if (!object)
	{
		PrintError("Object of name : %s doesn't exist abort remove", objectName);
		return;
	}

	object->RemoveFromParent();
}

GameObject* Core::Scene::FindObject(std::string objectName)
{
	if (!m_sceneNode)
		return nullptr;

	if (m_sceneNode->GetName() == objectName)
		return m_sceneNode;

	for (GameObject* gameObject : m_sceneNode->GetChildrens())
	{
		if (gameObject->GetName() == objectName)
			return gameObject;
	}

	PrintWarning("Couldn't find gameobject of name : %s in current scene", objectName.c_str());
	return nullptr;
}

std::vector<GameObject*> Core::Scene::FindAllObject(std::string objectName)
{
	if (!m_sceneNode)
		return std::vector<GameObject*>();

	std::vector<GameObject*> objectList;

	if (m_sceneNode->GetName() == objectName)
		objectList.push_back(m_sceneNode);

	FindObjects(objectName, m_sceneNode, objectList);

	if (objectList.size() != 0)
		return objectList;


	PrintWarning("Couldn't find any gameobject of name : %s in current scene", objectName.c_str());
	return std::vector<GameObject*>();
}

#ifndef PANDOR_GAME
void Core::Scene::SaveSceneFile(std::string filePath)
{
	if (m_isPrefab)
	{
		m_parentPrefab->SavePrefab();
		return;
	}

	//name = ExtractName(filePath);
	/* Create or open file */
	std::fstream sceneFile;
	sceneFile.open(filePath.c_str(), std::fstream::trunc | std::fstream::out);

	if (filePath != ENGINEPATH"savePlay.scn")
	{
		name = filePath.substr(filePath.find_last_of('\\') + 1);
		name = name.substr(name.find_last_of('/') + 1);
		name = name.substr(0, name.find_last_of("."));
	}
	this->GetSceneNode()->SetName(name);
	if (sceneFile.is_open())
	{
		sceneFile << name + "\n";
		m_editorCamera->operator<<(sceneFile);
		//sceneFile << *Resources::ResourcesManager::Get();
		m_sceneNode->operator<<(sceneFile);
		sceneFile << "\n";
		sceneFile << m_isCurrentScene;
		sceneFile << "\n";
	}

	/* Close file */
	sceneFile.close();
	PrintLog("Scene file has been written correctly");
}
#endif

void Core::Scene::ReadSceneFile(std::string filePath)
{
	std::fstream sceneFile;
	sceneFile.open(filePath.c_str());

	bool bswitch = false;
	int i = 0;
	std::string line;
	if (sceneFile.is_open())
	{
		while (std::getline(sceneFile, line))
		{
			if (i == 0 && path == "")
				this->name = line;
			else
				this->name = ExtractName(path);

			if (bswitch)
			{
				SceneObjectType type = StringToEnum(line);
				switch (type)
				{
				case SceneObjectType::GameObject:
					if (m_sceneNode)
					{
						auto gb = CreateObject();
						gb->ReadGameObject(sceneFile, this);
						m_sceneNode->AddChildren(gb);
					}
					else
					{
						m_sceneNode = new GameObject(name);
						m_sceneNode->ReadGameObject(sceneFile, this);
					}
					break;
				case SceneObjectType::Bone:
				{
					if (m_sceneNode)
					{
						auto gb = CreateBoneObject();
						gb->ReadGameObject(sceneFile, this);
						m_sceneNode->AddChildren(gb);
					}
					else
					{
						m_sceneNode = CreateBoneObject();
						m_sceneNode->ReadGameObject(sceneFile, this);
					}
					break;
				}
				case SceneObjectType::ResourcesManager:
					//LoadAllScenesResources(sceneFile);
					break;
				case SceneObjectType::Camera:
#ifndef PANDOR_GAME
					getline(sceneFile, line);
					getline(sceneFile, line);
					Component::Transform* transformCamera = new Component::Transform();
					transformCamera->ReadComponent(sceneFile);
					Render::Camera::ReadCameraFromFile(m_editorCamera, sceneFile, transformCamera);
#else
					while (getline(sceneFile, line) && line != "end") {}
					while (getline(sceneFile, line) && line != "end") {}
#endif
					break;
				}

				bswitch = false;
			}

			if (line == "===")
				bswitch = true;

			i++;
		}
	}

	if (!m_sceneNode)
	{
		m_sceneNode = new GameObject(name);
	}
	else if (name != "savePlay")
	{
		m_sceneNode->SetName(name);
	}
	sceneFile.close();
	isLoaded = true;
}

GameObject* Core::Scene::GetObjectByID(uint64_t ID)
{
	GameObject* null = nullptr;
	if (ID == 0)
		return null;

	if (m_objectMap.count(ID))
	{
		return m_objectMap[ID];
	}

	return null;
}

GameObject* Core::Scene::GetObjectByName(std::string_view name)
{
	for (auto& [id, object] : m_objectMap)
	{
		if (name == object->GetName())
			return object;
	}

	return nullptr;
}

#ifndef PANDOR_GAME
void Core::Scene::SaveSceneState()
{
	SaveSceneFile(ENGINEPATH"savePlay.scn");
}
#endif

void Core::Scene::ReloadSceneState()
{
	delete m_sceneNode;
	m_sceneNode = nullptr;
	isLoaded = false;
	ReadSceneFile(ENGINEPATH"savePlay.scn");
	std::remove((ENGINEPATH"savePlay.scn").c_str());
}

void Core::Scene::ParseShaderName(std::string name)
{
	std::string delimiter = " + ";
	size_t pos = name.find(delimiter);
	if (pos == std::string::npos)
	{
		Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(name);
		return;
	}

	std::string vertex = name.substr(0, pos);
	std::string fragment = name.substr(pos + delimiter.length());
	std::string nameShader;
	Resources::ResourcesManager::Get()->Create(nameShader, vertex, fragment);
}

void Core::Scene::LoadAllScenesResources(std::fstream& sceneFile)
{
	std::string line;

	while (getline(sceneFile, line) && line != "end")
	{
		int type = stoi(line);

		Resources::ResourcesType eType = (Resources::ResourcesType)type;

		switch (eType)
		{
		case Resources::ResourcesType::Texture:
			if (getline(sceneFile, line) && line != "end")
				Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(line);
			break;
		case Resources::ResourcesType::FragmentShader:
			if (getline(sceneFile, line) && line != "end")
				Resources::ResourcesManager::Get()->GetOrLoad<Resources::FragmentShader>(line);
			break;
		case Resources::ResourcesType::VertexShader:
			if (getline(sceneFile, line) && line != "end")
				Resources::ResourcesManager::Get()->GetOrLoad<Resources::VertexShader>(line);
			break;
		case Resources::ResourcesType::Shader:
			if (getline(sceneFile, line) && line != "end")
				ParseShaderName(line);
			break;
		case Resources::ResourcesType::Model:
			if (getline(sceneFile, line) && line != "end")
				Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(line);
			break;
		default:
			if (getline(sceneFile, line) && line != "end") {}
			break;
		}
	}
}

SceneObjectType Core::Scene::StringToEnum(std::string& str)
{
	if (str == "GameObject")
		return SceneObjectType::GameObject;
	if (str == "Bone")
		return SceneObjectType::Bone;
	if (str == "ResourcesManager")
		return SceneObjectType::ResourcesManager;
	if (str == "Camera")
		return SceneObjectType::Camera;
	else
		return SceneObjectType::CustomComponent;

}

void Core::Scene::FindObjects(std::string objectName, GameObject* object, std::vector<GameObject*>& objectList)
{
	for (GameObject* gameObject : object->GetChildrens())
	{
		if (gameObject->GetName() == objectName)
			objectList.push_back(gameObject);

		FindObjects(objectName, gameObject, objectList);
	}
}

void Core::Scene::Load()
{
	Core::SceneManager::Get()->SetCurrentScene(this);
#ifndef PANDOR_GAME
	auto size = Core::App::Get().GetEditorUIManager().GetSceneWindow().GetWindowSize();

	if (!m_editorCamera)
	{
		m_editorCamera = new Render::Camera((int)size.x, (int)size.y, Math::Vector3(0.0f, 0.0f, 2.0f));
		m_editorCamera->Init();
		m_editorCamera->CameraWithMouse({ 0,0 });
	}
#endif

	if (m_isPrefab)
	{
		if (m_parentPrefab)
			m_parentPrefab->LoadScene();
	}
	else
	{
		ReadSceneFile(path);
	}

#ifndef PANDOR_GAME
	m_editorUi = &Core::App::Get().GetEditorUIManager();
	m_grid = new Render::EditorGrid();
	m_grid->Initialize();
	drawGizmo = new Render::Gizmo;
	drawGizmo->CreateModel();
	SetCurrentCamera(m_editorCamera);

#else
	SetCurrentCamera(mainCamera);
#endif
	isLoaded = true;
}

std::string Core::Scene::ExtractName(std::string path)
{
	if (path.empty())
	{
		std::cout << "Error: Empty string" << std::endl;
		return "";
	}

	size_t sepPos = path.find_last_of("/\\");
	size_t dotPos = path.find_last_of(".");
	if (sepPos == std::string::npos || dotPos == std::string::npos)
	{
		std::cout << "Error: Invalid file path" << std::endl;
		return "";
	}
	std::string fileName = path.substr(sepPos + 1, dotPos - sepPos - 1);

	return fileName;
}
