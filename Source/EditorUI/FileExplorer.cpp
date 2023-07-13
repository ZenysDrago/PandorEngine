#include "pch.h"

#include <EditorUI\FileExplorer.h>
#include <EditorUI\EditorUIManager.h>
#include <EditorUI\TextEditorWindow.h>
#include <EditorUI\AnimatorWindow.h>
#include <EditorUI\Inspector.h>
#include <EditorUI/PrefabWindow.h>
#include <EditorUI\File.h>

#include <Resources\Texture.h>
#include <Resources\Shader.h>
#include <Resources\Animation.h>
#include <Resources\AnimationController.h>
#include <Resources\PhysicMaterial.h>
#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Utils/Loader.h>

#include <shellapi.h>
#include <Resources/Prefab.h>


EditorUI::FileExplorer::FileExplorer()
{
	m_path = Resources::ResourcesManager::AssetPath().substr(0, Resources::ResourcesManager::AssetPath().size() - 1);
	this->Refresh();
}

EditorUI::FileExplorer::~FileExplorer()
{
}

void EditorUI::FileExplorer::Draw()
{
	if (!p_open)
		return;
	bool rightclick = false;
	WrapperUI::PushID("FileExplorer");
	WindowFlags flag = WindowFlags::None;
	if (m_visible = WrapperUI::Begin("FileExplorer", &p_open, flag))
	{
		// ------------- Folders List ------------- //
		WrapperUI::BeginChild("List", Math::Vector2(WrapperUI::GetWindowWidth() * 0.15f, 0), false, WindowFlags::AlwaysHorizontalScrollbar);
		File* clicked = nullptr;
		m_main->DrawInFileExplorer(clicked);
		if (clicked)
		{
			m_current = std::make_shared<File>(*clicked);
			m_current->FoundChildren();
		}
		WrapperUI::EndChild();
		WrapperUI::SameLine();
		WrapperUI::SeparatorEx((SeparatorFlags)((int)SeparatorFlags::Vertical | (int)SeparatorFlags::SpanAllColumns));
		WrapperUI::SameLine();
		WrapperUI::BeginChild("Main");
		// ------------- Back Button ------------- //
		if (WrapperUI::Button("Back"))
		{
			Core::App::Get().GetEditorUIManager().GetInspector().SetFileSelected(nullptr);
			auto path = m_current->directory.substr(m_current->directory.find_last_of('/'), m_current->directory.size());
			if (path != "/Assets")
				m_current = m_current->GetParent();
		}
		WrapperUI::SameLine();
		// ------------- Directory Input ------------- //
		char Path[513];
		strcpy_s(Path, 512, m_current->directory.c_str());
		if (WrapperUI::InputText("Path", Path, 512, InputTextFlags::EnterReturnsTrue))
		{
			std::string newPath = Path;
			if (newPath.substr(0, 9) == Resources::ResourcesManager::AssetPath()) {
				try
				{
					std::shared_ptr<File> tmp = std::make_shared<File>(Path);
					tmp->FoundChildren();
					m_current = tmp;
				}
				catch (const std::exception& e)
				{
					// Case when not found
					PrintError("Error opening folder %s : %s", Path, e.what());
				}
			}
		}
		WrapperUI::SameLine();
		// ------------- Refresh Button ------------- //
		if (WrapperUI::Button("Refresh"))
		{
			RefreshThis();
		}
		// ------------- Filter Text ------------- //
		WrapperUI::SameLine();
		static TextFilter filter;
		filter.Draw();
		std::vector<std::string> items;
		// ------------- Files Buttons ------------- //
		WrapperUI::Separator();
		if (WrapperUI::BeginChild("Child")) {
			int index = 0;
			for (auto& f : m_current->childrens) {
				if (!f || !f.get())
					continue;
				try
				{

				items.push_back(f->name);
				WrapperUI::PushID(index);
				WrapperUI::BeginGroup();
				if (filter.PassFilter(items[index].c_str())) {
					if (f->icon && f->icon->HasBeenSent())
						WrapperUI::ImageButton(f->icon->ID, Math::Vector2(86, 86));
					if (WrapperUI::IsItemHovered() && WrapperUI::IsMouseDoubleClicked(MouseButton::Left))
					{
						try
						{
							if (f->type == FileType::Folder) {
								f->FoundChildren();
								m_current = f;
								WrapperUI::EndGroup();
								WrapperUI::PopID();
								break;
							}
						}
						catch (const std::exception& e)
						{
							// Case when not accessible because of rights.
							LOG(Debug::LogType::L_ERROR, "Error opening folder %s : %s", f->directory.c_str(), e.what());
						}
					}
					else if (WrapperUI::IsItemHovered() && WrapperUI::IsMouseClicked(MouseButton::Left) && f->type != FileType::Folder)
					{
						Core::App::Get().GetEditorUIManager().GetInspector().SetFileSelected(f);
					}
					// Name of the file.
					if (f->hovered = WrapperUI::IsItemHovered())
					{
						WrapperUI::BeginTooltip();
						WrapperUI::Text(f->name.c_str());
						WrapperUI::EndTooltip();
					}
					if (WrapperUI::BeginDragDropSource(DragDropFlags::SourceAllowNullID))
					{
						static std::pair<std::string, FileType> file;
						file = (std::make_pair(Resources::ResourcesManager::ProjectPath() +'/' + f->directory, f->type));
						WrapperUI::SetDragDropPayload("File", &file, sizeof(std::pair<std::string, FileType>), Cond::None);
						WrapperUI::Text(f->name.c_str());
						WrapperUI::EndDragDropSource();
					}
					if (WrapperUI::BeginDragDropTarget()) {
						if (const Payload* payload = WrapperUI::AcceptDragDropPayload("File")) {
							std::string PayloadIndex = *(std::string*)payload->Data;
							if (f->type == FileType::Folder)
							{
								auto newPath = PayloadIndex.substr(PayloadIndex.find_last_of('/') + 1);
								newPath = f->directory + '/' + newPath;
								PrintLog("Changed File %s to %s", PayloadIndex.c_str(), newPath.c_str());
								try
								{
									std::filesystem::rename(PayloadIndex, newPath);
									RefreshThis();
								}
								catch (const std::exception& e)
								{
									PrintError("Failed to Rename file %s to %s, reason : %s", PayloadIndex.c_str(), newPath.c_str(), e.what());
								}
							}
						}
						WrapperUI::EndDragDropTarget();
					}
					if (f != m_renameFile) {
						WrapperUI::TextUnformatted(f->name.substr(0, 10).c_str());
					}
					else
					{
						char Name[64];
						strcpy_s(Name, 64, f->name.c_str());
						WrapperUI::SetNextItemWidth(86);
						if (WrapperUI::InputText("##RenameField", Name, 64, InputTextFlags::EnterReturnsTrue))
						{
							std::string directory = m_rightClicked->directory;
							auto NewPath = directory.substr(0, directory.find_last_of('/') + 1) + Name;
							std::filesystem::rename(directory, NewPath);
							Resources::ResourcesManager::Get()->Rename(m_rightClicked->directory, NewPath, m_rightClicked->type == FileType::Folder);
							RefreshThis();
							Utils::ToPath(NewPath);
							m_renameFile.reset();
						}
						if (!WrapperUI::IsItemClicked() && (WrapperUI::IsMouseClicked(MouseButton::Left) || WrapperUI::IsMouseClicked(MouseButton::Right)))
						{
							m_renameFile.reset();
						}
					}
				}
				WrapperUI::EndGroup();
				// At The End Because if file deleted -> crash.
				if (WrapperUI::IsMouseClicked(MouseButton::Right) && WrapperUI::IsItemHovered())
				{
					WrapperUI::OpenPopup("RightClick");
					m_rightClicked = f;
					rightclick = true;
				}
				RightClickWindow();
				auto windowSize = WrapperUI::GetWindowSize();
				int rounded = (int)round(windowSize.x / 105);
				if (rounded > 0) {
					if (index % rounded != rounded - 1)
					{
						WrapperUI::SameLine();
					}
				}

				index++;
				WrapperUI::PopID();
				}
				catch (std::exception e)
				{
					PrintError(e.what());
				}
			}
		}
		WrapperUI::EndChild();
		if (WrapperUI::IsMouseClicked(MouseButton::Right) && WrapperUI::IsWindowHovered(HoveredFlags::ChildWindows) && !rightclick)
		{
			WrapperUI::OpenPopup("RightClick");
			m_rightClicked = nullptr;
		}
		RightClickWindow();
		WrapperUI::EndChild();
	}
	WrapperUI::End();
	WrapperUI::PopID();

}

void EditorUI::FileExplorer::Refresh()
{
	m_main = std::make_shared<EditorUI::File>(m_path);
	m_main->FoundChildren();
	m_current = m_main;
	m_main->FindAllChilden();
}

void EditorUI::FileExplorer::RefreshThis()
{
	Core::App::Get().GetEditorUIManager().GetInspector().SetFileSelected(nullptr);
	try
	{
		m_main->FoundChildren();
		m_main->FindAllChilden();
		m_current->FoundChildren();
	}
	catch (const std::exception& e)
	{
		PrintError("Error opening folder %s : %s", m_current->directory, e.what());
		m_current = m_main;
	}
}

void EditorUI::FileExplorer::RightClickWindow()
{
	if (WrapperUI::BeginPopup("RightClick", WindowFlags::NoDecoration))
	{
		if (WrapperUI::BeginMenu("Create"))
		{
			if (WrapperUI::Button("Animation Controller"))
			{
				auto path = m_current->directory + "/NewAnimationController.ac";
				Resources::AnimationController* animC = new Resources::AnimationController(path, ResourcesType::AnimationController);
				animC->Create();
				animC->Save();
				animC->p_shouldBeLoaded = true;
				animC->isLoaded = true;
				Resources::ResourcesManager::Get()->Add(path, animC);
				RefreshThis();
				WrapperUI::CloseCurrentPopup();
			}
			if (WrapperUI::Button("Material"))
			{
				auto path = m_current->directory + "/NewMaterial.mat";
				Resources::Material* mat = new Resources::Material(path, ResourcesType::Material);
				mat->Save();
				mat->p_shouldBeLoaded = true;
				mat->isLoaded = true;
				Resources::ResourcesManager::Get()->Add(path, mat);
				RefreshThis();
				WrapperUI::CloseCurrentPopup();
			}
			if (WrapperUI::Button("Physic Material"))
			{
				auto path = m_current->directory + "/NewPhysicMaterial.phm";
				Resources::PhysicMaterial* mat = new Resources::PhysicMaterial(path, ResourcesType::PhysicMaterial);
				mat->Save();
				mat->p_shouldBeLoaded = true;
				mat->isLoaded = true;
				mat->SendResource();
				Resources::ResourcesManager::Get()->Add(path, mat);
				RefreshThis();
				WrapperUI::CloseCurrentPopup();
			}
			WrapperUI::EndMenu();
		}
		if (WrapperUI::Button("New Folder"))
		{
			std::filesystem::create_directory(m_current->directory + "/NewFolder");
			m_current->FoundChildren();
			WrapperUI::CloseCurrentPopup();
		}
		if (m_rightClicked != nullptr)
		{
			if (!m_rightClicked->resourceLink && WrapperUI::Button("Import"))
			{
				Core::App::Get().resourcesManager->ResourceLoad(m_rightClicked->directory);
				m_rightClicked->resourceLink = Core::App::Get().resourcesManager->Find<Resources::IResources>(m_rightClicked->directory);
			}
			WrapperUI::Separator();
			// ------------- Rename ------------- //
			char Name[64];
			strcpy_s(Name, 64, m_rightClicked->name.c_str());
			if (WrapperUI::Button("Rename"))
			{
				m_renameFile = m_rightClicked;
				WrapperUI::CloseCurrentPopup();
			}
			switch (m_rightClicked->type)
			{
				// ------------- Shaders ------------- //
			case FileType::Shdr:
			{
				// ------------- Show in Explorer ------------- //
				if (WrapperUI::Button("Recompile"))
				{
					if (auto vertShader = dynamic_cast<Resources::VertexShader*>(m_rightClicked->resourceLink))
					{
						vertShader->Recompile();
					}
					else if (auto fragShader = dynamic_cast<Resources::FragmentShader*>(m_rightClicked->resourceLink))
					{
						fragShader->Recompile();
					}
					WrapperUI::CloseCurrentPopup();
				}
				if (WrapperUI::Button("Edit"))
				{
					// Settings Parameters
					if (Core::App::Get().GetEditorUIManager().GetTextEditor().IsTabOpen(m_rightClicked->directory.c_str()))
						break;
					TextEditorWindow& textEditor = Core::App::Get().GetEditorUIManager().GetTextEditor();
					uint32_t size = 0;
					bool sucess;
					auto text = Utils::Loader::ReadFile(m_rightClicked->directory.c_str(), size, sucess);
					std::string cuted = text;
					cuted = cuted.substr(0, size);

					textEditor.AddTab(m_rightClicked->name, m_rightClicked->directory, Language::GLSL, cuted);

					delete[] text;
					text = nullptr;
					WrapperUI::CloseCurrentPopup();
				}
				break;
			}
			// ------------- Scene ------------- //
			case FileType::Scn:
			{
				if (WrapperUI::Button("Switch Scene"))
				{
					Core::SceneManager* sceneManager = Core::SceneManager::Get();
					Core::Scene* selectedScene = sceneManager->FindSceneByName(m_rightClicked->directory);
					if (!selectedScene)
						selectedScene = sceneManager->CreateSceneFromPath(m_rightClicked->directory);
					sceneManager->SwitchScene(selectedScene);
					WrapperUI::CloseCurrentPopup();
				}
				break;
			}
			// ------------- Models ------------- //
			case FileType::Model:
			{
				if (!m_rightClicked->resourceLink || !m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Load"))
				{
					Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(m_rightClicked->directory);
				}
				if (WrapperUI::Button("Create Thumbnail"))
				{
					Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(m_rightClicked->directory, ResourcesType::Model);
				}
				break;
			}
			// ------------- Material ------------- //
			case FileType::Mat:
			{
				if (m_rightClicked->resourceLink && !m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Load"))
				{
					Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(m_rightClicked->directory);
				}
				if (WrapperUI::Button("Create Thumbnail"))
				{
					Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(m_rightClicked->directory, ResourcesType::Material);
				}
				break;
			}
			// ------------- Animation ------------- //
			case FileType::Anim:
			{
				if (m_rightClicked->resourceLink && !m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Load"))
				{
					Resources::ResourcesManager::Get()->GetOrLoad<Resources::Animation>(m_rightClicked->directory);
				}
				break;
			}
			// ------------- Animation Controller ------------- //
			case FileType::AnimC:
			{
				if (!m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Load"))
				{
					Resources::ResourcesManager::Get()->GetOrLoad<Resources::AnimationController>(m_rightClicked->directory);
				}
				if (m_rightClicked->resourceLink && m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Edit"))
				{
					Core::App::Get().GetEditorUIManager().GetAnimatorWindow().SetAnimationController(dynamic_cast<Resources::AnimationController*>(m_rightClicked->resourceLink));
				}
				break;
			}
			// ------------- Physic Material ------------- //
			case FileType::Phm:
			{
				if (m_rightClicked->resourceLink && !m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Load"))
				{
					Resources::ResourcesManager::Get()->GetOrLoad<Resources::PhysicMaterial>(m_rightClicked->directory);
				}
				break;
			}
			// ------------- Texture ------------- //
			case FileType::Img:
			{
				if (m_rightClicked->resourceLink && !m_rightClicked->resourceLink->p_shouldBeLoaded && WrapperUI::Button("Reload"))
				{
					Resources::ResourcesManager::Get()->Reload<Resources::Texture>(m_rightClicked->directory);
				}
				break;
			}
			case FileType::Prefab:
			{
				Resources::Prefab* prefab = ResourcesManager::Get()->GetOrLoad<Resources::Prefab>(m_rightClicked->directory);
				Core::SceneManager* sceneManager = Core::SceneManager::Get();

				if (WrapperUI::Button("OpenPrefab"))
				{
					prefab->OpenPrefabScene();
					WrapperUI::CloseCurrentPopup();
				}
				if (WrapperUI::Button("Add to scene"))
				{
					EditorUI::PrefabWindow& prefabWindow = Core::App::Get().GetEditorUIManager().GetPrefabWindow();

					if (!(prefabWindow.IsFocused()))
						prefab->AddPrefabToScene(sceneManager->GetCurrentScene());
					else
						prefab->AddPrefabToScene(sceneManager->GetCurrentPrefabScene());

					WrapperUI::CloseCurrentPopup();
				}
				if (sceneManager->GetCurrentPrefabScene() && sceneManager->GetCurrentPrefabScene()->GetPrefab() == prefab)
				{
					if (WrapperUI::Button("Leave Prefab"))
					{
						prefab->exitPopup = true;

						WrapperUI::CloseCurrentPopup();
					}
				}

				break;
			}
			default:
				break;
			}
			// ------------- Show in Explorer ------------- //
			if (WrapperUI::Button("Show in Explorer"))
			{
				auto dir = m_rightClicked->directory;
				dir = dir.substr(0, dir.find_last_of('/'));
				if (m_rightClicked->directory.substr(0, m_rightClicked->directory.find_first_of('/')) == "Resources")
				{
					dir = std::filesystem::current_path().string() + '/' + dir;
				}
				ShellExecuteA(NULL, "open", dir.c_str(), NULL, NULL, SW_SHOWDEFAULT);
			}
			// ------------- Delete ------------- //
			if (WrapperUI::Button("Delete"))
			{
				WrapperUI::OpenPopup("Delete");
			}
			if (WrapperUI::BeginPopupModal("Delete"))
			{
				WrapperUI::Text("Are you sure ?");
				if (WrapperUI::Button("Yes"))
				{
					try
					{
						std::filesystem::remove_all(Resources::ResourcesManager::ProjectPath() + '/' + m_rightClicked->directory.c_str());
						//Resources::ResourcesManager::Get()->Delete(m_rightClicked->directory);
						m_rightClicked->resourceLink = nullptr;
						m_rightClicked = nullptr;
					}
					catch (const std::exception& e)
					{
						LOG(Debug::LogType::L_ERROR, "Error Delete file %s", e.what());
					}
					RefreshThis();
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::SameLine();
				if (WrapperUI::Button("No"))
				{
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::EndPopup();
			}
		}
		WrapperUI::EndPopup();
	}
}

void EditorUI::FileExplorer::Drop(const char** path, int count)
{
	if (!m_visible)
		return;
	for (int i = 0; i < count; i++) {
		std::string stringPath = path[i];
		std::string newPath = stringPath.substr(stringPath.find_last_of('\\') + 1);
		newPath = m_current->directory + '/' + newPath;
		std::filesystem::rename(stringPath, newPath);
	}
	RefreshThis();
}

