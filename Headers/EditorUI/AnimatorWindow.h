#pragma once
#include "PandorAPI.h"
#include <EditorUI/EditorWindow.h>

namespace Resources
{
	class AnimationController;
	struct StateRect;
	struct Link;
}
namespace EditorUI
{
	class PANDOR_API AnimatorWindow : public EditorWindow
	{
	public:
		AnimatorWindow();
		~AnimatorWindow();

		void Draw() override;

		void SetAnimationController(Resources::AnimationController* animC);
		Resources::AnimationController* GetAnimationController() const { return m_animationController; }
	private:
		friend Resources::AnimationController;
		Resources::AnimationController* m_animationController = nullptr;
		Resources::StateRect* m_stateSelected = nullptr;
		Resources::Link* m_linkSelected = nullptr;
	};
}