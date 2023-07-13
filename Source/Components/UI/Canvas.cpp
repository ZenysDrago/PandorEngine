#include "pch.h"

#include <Components/UI/Canvas.h>
#include <Core/App.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/CanvasWindow.h>

void Component::UI::Canvas::ShowInInspector()
{
#ifndef PANDOR_GAME
	if (WrapperUI::Button("Open Canvas Window"))
	{
		Core::App::Get().GetEditorUIManager().GetCanvasWindow().SetOpen(true);
		Core::App::Get().GetEditorUIManager().GetCanvasWindow().SetCanvas(this->gameObject);
	}
#endif
}
