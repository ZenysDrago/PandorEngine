#pragma once
#include "PandorAPI.h"
#include <EditorUI/EditorWindow.h>

#include <vector>
#include <string>

class TextEditor;

namespace EditorUI
{
	enum class Language
	{
		GLSL,
		CPP,
		CSHARP
	};

	class PANDOR_API TextEditorWindow : public EditorWindow
	{
	public:
		TextEditorWindow();
		~TextEditorWindow();

		void AddTab(const std::string& title, const std::string& path, Language language, const std::string& content);

		void Draw() override;

		bool IsTabOpen(std::string tabName);

		std::vector<TextEditor*> Tabs;

	private:

	};
}