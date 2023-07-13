#pragma once
#include "PandorAPI.h"
#include <Resources/IResources.h>

namespace Component
{
	class Animator;
}
namespace Resources
{
	struct StateRect
	{
		std::string name;
		class Animation* animation;
		Math::Vector2 pos;
		Math::Vector4 color;
		bool selected;
		bool loop = true;
		float speed = 1.0f;

		void ShowInInspector();
	};

	struct Link
	{
		StateRect* state1;
		StateRect* state2;
		bool hasExitTime = true; //Exit at a certain time
		float exitTime = 0.75f;
		float transitionDuration = 0.25f;
		std::vector<std::pair<std::pair<const std::string, bool>*, bool>> conditions;
		AnimationController* animC;

		void ShowInInspector();
	};

	class PANDOR_API AnimationController : public IResources
	{
	public:
		AnimationController(std::string _path, ResourcesType _type) : IResources(_path, _type)
		{
			//AddState("Entry", { 0.f, 0.f }, Math::Vector4(0.2f, 0.8f, 0.2f, 1.f));
		}
		~AnimationController();

		void Load() override;
		void SendResource() override;
		void Save();
		void Create();

		void Update(Component::Animator* animator);

		void UpdateConditions(Component::Animator* animator);

		void ChangeAnimation(bool value, Resources::Link* anyState, Component::Animator* animator);

		void UpdateParameters(Component::Animator* animator, bool UpdateValue = false);

		void UpdateExitTime(Component::Animator* animator);

		void UpdateTransition(Component::Animator* animator);

		void UpdateAnimation(Component::Animator* animator, Resources::Animation* anim);

		StateRect* GetStateByName(const std::string& name);

		static ResourcesType GetResourceType() { return ResourcesType::AnimationController; }

		void AddParameter(std::string name);
		void RenameParameter(const std::string name, const std::string newName);
		void DeleteParameter(const std::string& name);

		StateRect* AddState(std::string name, Math::Vector2 pos, const Math::Vector4& color);
		void RenameState(const std::string name, const std::string newName);
		void RemoveState(StateRect* _state);

		Link* AddLink(StateRect* state1, StateRect* states2);
		Link* GetLinkByOutputState(StateRect* state1);
		void RemoveLink(Link* _state);

		void ShowInInspector() override;

	public:
		std::unordered_map<std::string, bool> parameters;
		std::unordered_map<std::string, StateRect*> states;
		std::vector<Link*> links;
		
		bool parameterUpdated = true;

	private:
	};
}