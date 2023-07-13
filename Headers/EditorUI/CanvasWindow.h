#pragma once
#include <EditorUI/EditorWindow.h>
namespace Core
{
	class GameObject;
}
namespace EditorUI
{
	class CanvasWindow : public EditorWindow
	{
	public:
		CanvasWindow();
		~CanvasWindow();

		void Draw() override;
		Core::GameObject* GetCanvas() const { return m_canvas; }
		void SetCanvas(Core::GameObject* val) { m_canvas = val; }
	private:
		Core::GameObject* m_canvas;
	};
}