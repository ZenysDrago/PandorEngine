#pragma once
#include <Components/BaseComponent.h>

namespace Resources
{
	class AnimationController;
	class Animation;
	struct Link;
	struct StateRect;
}
namespace Component 
{
	class Animator : public IComponent<Animator>
	{
	private:
		friend Resources::AnimationController;

		Resources::AnimationController* m_animationController = nullptr;
		class SkeletalMeshComponent* m_skeletalMesh = nullptr;

		bool m_play = true;

		std::unordered_map<std::string, bool> m_parameters;

		Resources::StateRect* m_currentState;
		bool m_conditionUpdated = true;

		float m_currentTime = 0.0f;

		Resources::Animation* m_lastAnimation = nullptr;
		Resources::Link* m_transitionLink = nullptr;
		float m_elapsedTime = 0.0f; // Transition Elapsed Time.

		bool m_endExitTime = false;
		//TMP
		bool active = false;

	public:
		Animator();
		~Animator();

		std::string GetComponentName() override { return "Animator"; }

		void ShowInInspector() override;

		void Update() override;

		void EditorUpdate() override;

		void GameUpdate() override;

		void SetBoolean(const std::string& name, bool value);

		void SetSkeletalMesh(class SkeletalMeshComponent* _skeletalMesh);

		void SetTransitionLink(Resources::Link* link);

		void IncrementTime(Resources::Animation* anim);

		void ReadComponent(std::fstream& sceneFile) override;

		std::ostream& operator<<(std::ostream& os) override;

	
	};
}