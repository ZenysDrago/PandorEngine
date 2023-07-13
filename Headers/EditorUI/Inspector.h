#pragma once
#include "PandorAPI.h"

#include "EditorWindow.h"
namespace Core
{
	class GameObject;
}


namespace EditorUI
{
	class PANDOR_API Inspector : public EditorWindow
	{
	private:
		Core::GameObject* m_selected;
		std::shared_ptr<class File> m_fileSelected;
	public:
		Inspector();
		~Inspector();

		void Draw() override;

		void SetGameObjectSelected(Core::GameObject* gameObject);
		Core::GameObject* GetGameObjectSelected() { return m_selected; }

		void SetFileSelected(std::shared_ptr<File> file);
		std::shared_ptr<class File> GetFileSelected() { return m_fileSelected; }

	};
}