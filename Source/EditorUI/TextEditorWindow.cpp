#include "pch.h"
#include <EditorUI/TextEditorWindow.h>

#include <ImGui/TextEditor.h>

EditorUI::TextEditorWindow::TextEditorWindow()
{

}

EditorUI::TextEditorWindow::~TextEditorWindow()
{
	for (auto& tab : Tabs)
	{
		delete tab;
		tab = nullptr;
	}
}

void EditorUI::TextEditorWindow::AddTab(const std::string& title, const std::string& path, Language language, const std::string& content)
{
	Tabs.push_back(new TextEditor());
	Tabs.back()->SetTitle(title);
	Tabs.back()->SetOpen(true);
	Tabs.back()->SetPath(path);
	switch (language)
	{
	case EditorUI::Language::GLSL:
		Tabs.back()->SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
		break;
	case EditorUI::Language::CPP:
		Tabs.back()->SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
		break;
	case EditorUI::Language::CSHARP:
		Tabs.back()->SetLanguageDefinition(TextEditor::LanguageDefinition::C());
		break;
	default:
		break;
	}
	Tabs.back()->SetText(content);
}

void EditorUI::TextEditorWindow::Draw()
{
	if (!p_open || Tabs.size() == 0)
		return;
	if (ImGui::Begin("TextEditor", &p_open)) {
		ImGui::BeginTabBar("Edit");
		int i = 0;
		for (auto&& tab : Tabs) {
			if (!tab->IsOpen()) {
				delete Tabs[i];
				Tabs[i] = nullptr;
				Tabs.erase(Tabs.begin() + i);
				break;
			}
			tab->Render();
			i++;
		}

		ImGui::EndTabBar();
	}
	ImGui::End();
}

bool EditorUI::TextEditorWindow::IsTabOpen(std::string tabName)
{
	for (auto&& tab : Tabs) {
		if (tab->GetPath() == tabName && tab->IsOpen()) {
			return true;
		}
	}
	return false;
}

