#include "pch.h"

#include <EditorUI/AnimatorWindow.h>
#include <Resources/AnimationController.h>

#include <Core/App.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/Inspector.h>
#include <EditorUI/File.h>

EditorUI::AnimatorWindow::AnimatorWindow()
{
	this->p_open = false;
}

EditorUI::AnimatorWindow::~AnimatorWindow()
{
}

void EditorUI::AnimatorWindow::Draw()
{
#ifndef PANDOR_GAME
	if (!m_animationController) {
		if (auto file = Core::App::Get().GetEditorUIManager().GetInspector().GetFileSelected()) {
			if (!file->resourceLink)
			{
				m_animationController = nullptr;
				return;
			}
			if (file->resourceLink && file->resourceLink->IsLoaded()) {
				if (file->resourceLink->GetType() == ResourcesType::AnimationController)
				{
					m_animationController = dynamic_cast<Resources::AnimationController*>(file->resourceLink);
					p_open = true;
				}
			}
		}
	}
	if (!p_open || !m_animationController)
		return;
	if (WrapperUI::IsKeyDown(Key::Key_LeftCtrl) && WrapperUI::IsKeyPressed(Key::Key_S))
	{
		m_animationController->Save();
	}
	if (WrapperUI::Begin("Animator Window", &p_open))
	{
		WrapperUI::DrawAnimatorList(m_animationController);
		WrapperUI::DrawAnimatorBackGround(m_animationController, m_stateSelected, m_linkSelected);
	}
	WrapperUI::End();
#endif
}

void EditorUI::AnimatorWindow::SetAnimationController(Resources::AnimationController* animC)
{
	m_animationController = animC;
}

