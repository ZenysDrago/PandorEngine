#pragma once
#include <EditorUI/EditorWindow.h>
#include <Math/Maths.h>

namespace EditorUI
{
	class PANDOR_API PrefabWindow : public EditorWindow
	{
	private :
		bool m_hovered = false;
		bool m_visible = false;
		bool m_focused = false;
		Math::Vector2 m_windowsize;
		Math::Vector2 m_windowPos;

	public:
		PrefabWindow();
		~PrefabWindow();

		void Draw() override;

		Math::Vector2 GetWindowSize() { return m_windowsize; }
		Math::Vector2 GetWindowPos() { return m_windowPos; }
		Math::Vector2 GetMousePosition();
		bool IsFocused() { return m_focused; }
		bool IsHovered() { return m_hovered; }
		bool IsVisible();
	};
}