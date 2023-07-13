#pragma once
#include "PandorAPI.h"

#include "EditorWindow.h"
namespace Core {
	class GameObject;
}

namespace Resources
{
	class Model;
}

namespace EditorUI
{
	class PANDOR_API Hierarchy : public EditorWindow
	{
	private:
		Core::GameObject* m_rightClicked = nullptr;
		std::vector<Resources::Model*> m_waitingModels;
		bool m_openRightClick = false;
	public:
		bool m_sceneFocused = true;
		Hierarchy();
		~Hierarchy();

		void Draw() override;
		void DrawRightClick();

		void SetGameObjectRightClicked(Core::GameObject* node);
	};
}