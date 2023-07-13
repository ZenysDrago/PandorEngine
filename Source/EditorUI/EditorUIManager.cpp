#include "pch.h"

#include <EditorUI/EditorUIManager.h>
#include <EditorUI/Console.h>
#include <EditorUI/MainBar.h>
#include <EditorUI/PerformanceWindow.h>
#include <EditorUI/Hierarchy.h>
#include <EditorUI/Inspector.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/ResourcesWindow.h>
#include <EditorUI/FileExplorer.h>
#include <EditorUI/CanvasWindow.h>
#include <EditorUI/GameWindow.h>
#include <EditorUI/TextEditorWindow.h>
#include <EditorUI/AnimatorWindow.h>
#include <EditorUI/PrefabWindow.h>

EditorUI::EditorUiManager::EditorUiManager()
{
}

EditorUI::EditorUiManager::~EditorUiManager()
{
	delete m_mainBar;
	m_mainBar = nullptr;
	delete m_performanceWindow;
	m_performanceWindow = nullptr;
	delete m_sceneWindow;
	m_sceneWindow = nullptr;
	delete m_hierarchy;
	m_hierarchy = nullptr;
	delete m_inspector;
	m_inspector = nullptr;
	delete m_resourcesWindow;
	m_resourcesWindow = nullptr;
	delete m_fileExplorer;
	m_fileExplorer = nullptr;
	delete m_canvasWindow;
	m_canvasWindow = nullptr;
	delete m_gameWindow;
	m_gameWindow = nullptr;
	delete m_textEditorWindow;
	m_textEditorWindow = nullptr;
	delete m_animatorWindow;
	m_animatorWindow = nullptr;
	delete m_prefabWindow;
	m_prefabWindow = nullptr;
}

void EditorUI::EditorUiManager::Initialize()
{
	m_console = Debug::Log::GetConsole();
	m_mainBar = new MainBar();
	m_performanceWindow = new PerformanceWindow();
	m_sceneWindow = new SceneWindow();
	m_hierarchy = new Hierarchy();
	m_inspector = new Inspector();
	m_resourcesWindow = new ResourcesWindow();
	m_fileExplorer = new FileExplorer();
	m_canvasWindow = new CanvasWindow();
	m_gameWindow = new GameWindow();
	m_textEditorWindow = new TextEditorWindow();
	m_animatorWindow = new AnimatorWindow();
	m_prefabWindow = new PrefabWindow();
}

void EditorUI::EditorUiManager::Draw()
{
	//WrapperUI::ShowDemoWindow();
	DrawMainDocking();
	m_mainBar->Draw();
	m_performanceWindow->Draw();
	m_canvasWindow->Draw();
	m_gameWindow->Draw();
	m_sceneWindow->Draw();
	m_hierarchy->Draw();
	m_inspector->Draw();
	m_resourcesWindow->Draw();
	m_textEditorWindow->Draw();
	m_fileExplorer->Draw();
	m_animatorWindow->Draw();
	m_console->Draw();
	m_prefabWindow->Draw();
}

void EditorUI::EditorUiManager::DrawMainDocking()
{
	WrapperUI::ShowMainDocking();
}

void EditorUI::EditorUiManager::Reset()
{
	delete m_mainBar;
	m_mainBar = nullptr;
	delete m_performanceWindow;
	m_performanceWindow = nullptr;
	delete m_sceneWindow;
	m_sceneWindow = nullptr;
	delete m_hierarchy;
	m_hierarchy = nullptr;
	delete m_inspector;
	m_inspector = nullptr;
	delete m_resourcesWindow;
	m_resourcesWindow = nullptr;
	delete m_fileExplorer;
	m_fileExplorer = nullptr;
	delete m_canvasWindow;
	m_canvasWindow = nullptr;
	delete m_gameWindow;
	m_gameWindow = nullptr;
	delete m_textEditorWindow;
	m_textEditorWindow = nullptr;
	delete m_prefabWindow;
	m_prefabWindow = nullptr;

	Initialize();
}
