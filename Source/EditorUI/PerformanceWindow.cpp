#include "pch.h"

#include <Core/App.h>
#include <EditorUI/PerformanceWindow.h>
#include <Core/Wrappers/WrapperRHI.h>

EditorUI::PerformanceWindow::PerformanceWindow()
{

}

EditorUI::PerformanceWindow::~PerformanceWindow()
{
}

void EditorUI::PerformanceWindow::Draw()
{
	WrapperUI::ShowPerformanceWindow("Performance", &p_open);
}

