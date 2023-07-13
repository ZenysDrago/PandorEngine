#pragma once
#include "PandorAPI.h"

namespace EditorUI
{
	class PANDOR_API EditorUiManager
	{
	private:
		class Console* m_console;
		class MainBar* m_mainBar;
		class PerformanceWindow* m_performanceWindow;
		class SceneWindow* m_sceneWindow;
		class Hierarchy* m_hierarchy;
		class Inspector* m_inspector;
		class ResourcesWindow* m_resourcesWindow;
		class FileExplorer* m_fileExplorer;
		class CanvasWindow* m_canvasWindow;
		class GameWindow* m_gameWindow;
		class TextEditorWindow* m_textEditorWindow;
		class AnimatorWindow* m_animatorWindow;
		class PrefabWindow* m_prefabWindow;


	public:
		EditorUiManager();
		~EditorUiManager();

		int themeIndex = 0;

		void Initialize();

		void Draw();
		void DrawMainDocking();
		void Reset();

		class Console& GetConsole() { return *m_console; }
		class MainBar& GetMainBar() { return *m_mainBar; }
		class PerformanceWindow& GetPerformanceWindow() { return *m_performanceWindow; }
		class SceneWindow& GetSceneWindow() { return *m_sceneWindow; }
		class Hierarchy& GetHierarchy() { return *m_hierarchy; }
		class Inspector& GetInspector() { return *m_inspector; }
		class ResourcesWindow& GetResourceWindow() { return *m_resourcesWindow; }
		class FileExplorer& GetFileExplorer() { return *m_fileExplorer; }
		class CanvasWindow& GetCanvasWindow() { return *m_canvasWindow; }
		class GameWindow& GetGameWindow() { return *m_gameWindow; }
		class TextEditorWindow& GetTextEditor() { return *m_textEditorWindow; }
		class AnimatorWindow& GetAnimatorWindow() { return *m_animatorWindow; }
		class PrefabWindow& GetPrefabWindow() { return*m_prefabWindow; }
	};
}