#pragma once
#include <EditorUI/EditorWindow.h>


namespace EditorUI
{
	class GameWindow : public EditorWindow
	{
	private:
		Math::Vector2 m_size = { 800, 600 };
		bool m_visible = false;
		bool m_hovered = false;

	public:
		GameWindow();
		~GameWindow();

		void Draw() override;


		Math::Vector2 GetSize() { return m_size; }
		bool IsHovered() { return m_hovered; }
		bool IsVisible();
	};
}