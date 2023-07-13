#include "pch.h"
#include <Resources/AnimationController.h>
#include <Resources/ResourcesManager.h>
#include <Resources/SkeletalMesh.h>
#include <Resources/Animation.h>
#include <Resources/Skeleton.h>

#include <Core/App.h>
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/AnimatorWindow.h>
#endif
#include <Utils/Loader.h>

#include <Components/Animator.h>
#include <Components/SkeletalMeshComponent.h>

Resources::AnimationController::~AnimationController()
{
	for (auto& state : states)
	{
		delete state.second;
		state.second = nullptr;
	}
	states.clear();
	for (auto& link : links)
	{
		delete link;
		link = nullptr;
	}
	links.clear();
}

void Resources::AnimationController::Load()
{
	if (p_shouldBeLoaded)
		return;
	p_shouldBeLoaded = true;
	Utils::Loader::ANIMC::Load(this, p_fullPath);
	isLoaded = true;
	hasBeenSent = true;
}

void Resources::AnimationController::SendResource()
{
}

void Resources::AnimationController::Save()
{
	Utils::Loader::ANIMC::Save(this);
}

void Resources::AnimationController::Create()
{
	AddState("Entry", { 500.f, 250.f }, Math::Vector4(0.2f, 0.8f, 0.2f, 1.f));
	AddState("AnyState", { 500.f, 500.f }, Math::Vector4(0.56f, 0.006f, 0.6f, 1.f));
}

void Resources::AnimationController::Update(Component::Animator* animator)
{
	if (animator->m_play) {
		UpdateConditions(animator);
		if (auto current = animator->m_currentState)
		{
			if (animator->m_transitionLink && animator->m_transitionLink->hasExitTime && !animator->m_endExitTime)
				UpdateExitTime(animator);
			else if (!animator->m_transitionLink)
				UpdateAnimation(animator, animator->m_currentState->animation);
			else
				UpdateTransition(animator);
		}
	}
}

void Resources::AnimationController::UpdateConditions(Component::Animator* animator)
{
	if (!animator->m_conditionUpdated)
		return;
	if (auto anyState = GetLinkByOutputState(GetStateByName("Any State")))
	{
		if (anyState->state2 != animator->m_currentState) {
			bool value = true;
			for (auto& condition : anyState->conditions)
			{
				if (condition.first && animator->m_parameters[condition.first->first] != condition.second)
					value = false;
			}
			ChangeAnimation(value, anyState, animator);

		}
	}
	if (auto link = GetLinkByOutputState(animator->m_currentState))
	{
		bool value = true;
		for (auto& condition : link->conditions)
		{
			if (condition.first && animator->m_parameters[condition.first->first] != condition.second)
				value = false;
		}
		ChangeAnimation(value, link, animator);
	}
	animator->m_conditionUpdated = false;
}

void Resources::AnimationController::ChangeAnimation(bool value, Resources::Link* link, Component::Animator* animator)
{
	if (!value)
		return;
	if (link->state2 && !link->hasExitTime)
	{
		animator->m_lastAnimation = animator->m_currentState->animation;
		animator->m_currentState = link->state2;
		animator->SetTransitionLink(link);
	}
	else if (link->state2)
	{
		animator->m_lastAnimation = animator->m_currentState->animation;
		animator->m_currentState = link->state2;
		animator->m_transitionLink = link;
		animator->m_endExitTime = false;
	}
}

void Resources::AnimationController::UpdateParameters(Component::Animator* animator, bool UpdateValue /*= false*/)
{
	if (!parameterUpdated)
		return;
	if (UpdateValue)
		animator->m_parameters = parameters;
	else
	{
		// Check for all parameters of animator if existing inside controller
		// If not find then erase
		auto it = animator->m_parameters.begin();
		while (it != animator->m_parameters.end()) {
			if (!parameters.count(it->first)) {
				it = animator->m_parameters.erase(it);
			}
			else {
				++it;
			}
		}
		// Check for all parameters of controller if existing inside animator
		// If not find then create it
		for (auto& param : parameters)
		{
			if (!animator->m_parameters.count(param.first))
			{
				animator->m_parameters[param.first] = param.second;
			}
		}
	}
}

void Resources::AnimationController::UpdateExitTime(Component::Animator* animator)
{
	float normal = 0.f;
	if (animator->m_lastAnimation)
		normal = Arithmetics::Normalize(animator->m_currentTime, 0.f, (float)animator->m_lastAnimation->KeyCount);
	if (normal < animator->m_transitionLink->exitTime && animator->m_lastAnimation)
	{
		UpdateAnimation(animator, animator->m_lastAnimation);
	}
	else
	{
		animator->m_endExitTime = true;
		animator->SetTransitionLink(animator->m_transitionLink);
	}
}

void Resources::AnimationController::UpdateTransition(Component::Animator* animator)
{
	if (!animator->m_lastAnimation || !animator->m_transitionLink->state2->animation) {
		animator->m_transitionLink = nullptr;
		return;
	}

	animator->IncrementTime(animator->m_currentState->animation);

	auto skel = animator->m_skeletalMesh->GetSkeleton();
	if (skel && skel->RootBone)
	{
		skel->RootBone->CrossUpdate(animator->m_transitionLink->transitionDuration, animator->m_elapsedTime, animator->m_lastAnimation, animator->m_transitionLink->state2->animation);
	}

	animator->m_elapsedTime += WrapperUI::GetDeltaTime();
	if (animator->m_elapsedTime >= animator->m_transitionLink->transitionDuration)
	{
		animator->m_currentTime = 0.f;
		animator->m_transitionLink = nullptr;
	}
}

void Resources::AnimationController::UpdateAnimation(Component::Animator* animator, Resources::Animation* anim)
{
	if (!anim || !anim->HasBeenSent())
		return;
	animator->IncrementTime(anim);

	auto skel = animator->m_skeletalMesh->GetSkeleton();
	if (skel && skel->RootBone)
	{
		skel->RootBone->UpdateBone(anim, animator->m_currentTime);
	}
}

Resources::StateRect* Resources::AnimationController::GetStateByName(const std::string& name)
{
	if (states.count(name))
	{
		return states[name];
	}
	// Return a default Person struct if no matching name is found
	return nullptr;
}

void Resources::AnimationController::AddParameter(std::string name)
{
	if (parameters.count(name) > 0) {
		// if the key already exists, add a number at the end until a new key is formed
		int i = 1;
		std::string newKey = name + std::to_string(i);
		while (parameters.count(newKey) > 0) {
			i++;
			newKey = name + std::to_string(i);
		}
		// insert the new key-value pair into the map
		parameters[newKey] = false;
	}
	else {
		// insert the key-value pair directly into the map
		parameters[name] = false;
	}
	parameterUpdated = true;
}

void Resources::AnimationController::RenameParameter(const std::string name, const std::string newName)
{
	auto it = parameters.find(name);
	if (it != parameters.end())
	{
		parameters[newName] = it->second;
		// Check if a link get this parameter.
		for (auto& link : links)
		{
			for (int i = 0; i < link->conditions.size(); i++)
			{
				if (link->conditions[i].first->first == name)
				{
					link->conditions[i] = std::make_pair(&(*parameters.find(newName)), link->conditions[i].second);
				}
			}
		}
		parameters.erase(it);
	}
	parameterUpdated = true;
}

void Resources::AnimationController::DeleteParameter(const std::string& name)
{
	if (parameters.count(name))
	{
		// Check if a link get this parameter.
		for (auto& link : links)
		{
			for (int i = 0; i < link->conditions.size(); i++)
			{
				if (link->conditions[i].first->first == name)
				{
					link->conditions.erase(link->conditions.begin() + i);
				}
			}
		}
		parameters.erase(name);
	}
	parameterUpdated = true;
}

Resources::StateRect* Resources::AnimationController::AddState(std::string name, Math::Vector2 pos, const Math::Vector4& color)
{
	std::string newKey = name;
	if (states.count(name) > 0) {
		// if the key already exists, add a number at the end until a new key is formed
		int i = 1;
		newKey = name + std::to_string(i);
		while (states.count(newKey) > 0) {
			i++;
			newKey = name + std::to_string(i);
		}

		states[newKey] = (new StateRect{ newKey, nullptr, pos, color });
	}
	else {
		// insert the key-value pair directly into the map
		states[newKey] = (new StateRect{ newKey, nullptr, pos, color });
	}
	return states[newKey];
}

void Resources::AnimationController::RenameState(const std::string name, const std::string newName)
{
	auto it = states.find(name);
	if (it != states.end())
	{
		states[newName] = it->second;
		states.erase(it);
	}
}

void Resources::AnimationController::RemoveState(StateRect* _state)
{
#ifndef	PANDOR_GAME
	int index = 0;
	for (auto& state : states)
	{
		if (state.second == _state && state.second->color == Vector4(0.97f, 0.469f, 0.0f, 1.0f))
		{
			for (auto& link : links)
			{
				if (link->state1 == state.second || link->state2 == state.second)
				{
					RemoveLink(link);
				}
			}
			if (state.second == Core::App::Get().GetEditorUIManager().GetAnimatorWindow().m_stateSelected)
				Core::App::Get().GetEditorUIManager().GetAnimatorWindow().m_stateSelected = nullptr;
			delete state.second;
			state.second = nullptr;
			states.erase(state.first);
			break;
		}
		index++;
	}
#endif
}

Resources::Link* Resources::AnimationController::AddLink(StateRect* state1, StateRect* state2)
{
	links.push_back(new Link{ state1, state2 });
	links.back()->animC = this;
	return links.back();
}

Resources::Link* Resources::AnimationController::GetLinkByOutputState(StateRect* state1)
{
	if (!state1)
		return nullptr;
	for (auto& link : links)
	{
		if (link->state1 == state1)
			return link;
	}
	return nullptr;
}

void Resources::AnimationController::RemoveLink(Link* _link)
{
#ifndef PANDOR_GAME
	int index = 0;
	for (auto& link : links)
	{
		if (link == _link)
		{
			if (link == Core::App::Get().GetEditorUIManager().GetAnimatorWindow().m_linkSelected)
				Core::App::Get().GetEditorUIManager().GetAnimatorWindow().m_linkSelected = nullptr;
			delete link;
			link = nullptr;
			links.erase(links.begin() + index);
			break;
		}
		index++;
	}
#endif
}

void Resources::AnimationController::ShowInInspector()
{
#ifndef PANDOR_GAME
	auto animatorWindow = Core::App::Get().GetEditorUIManager().GetAnimatorWindow();
	if (animatorWindow.m_linkSelected)
	{
		animatorWindow.m_linkSelected->ShowInInspector();
	}
	else if (animatorWindow.m_stateSelected)
	{
		std::string lastName = animatorWindow.m_stateSelected->name;
		animatorWindow.m_stateSelected->ShowInInspector();
		if (lastName != animatorWindow.m_stateSelected->name)
		{
			RenameState(lastName, animatorWindow.m_stateSelected->name);
		}
	}
#endif
}


void Resources::StateRect::ShowInInspector()
{
	if (color != Vector4(0.97f, 0.469f, 0.0f, 1.0f))
		return;
	char Name[64];
	strcpy_s(Name, 64, name.c_str());
	if (WrapperUI::InputText("Rename", Name, 64, InputTextFlags::EnterReturnsTrue))
	{
		name = Name;
	}
	if (WrapperUI::Button("Animation"))
	{
		WrapperUI::OpenPopup("AnimationPopup");
	}
	if (auto anim = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Animation>("AnimationPopup"))
	{
		this->animation = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Animation>(anim->GetPath());
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(this->animation ? this->animation->GetName().c_str() : "None");
	WrapperUI::InputFloat("Speed", &speed);
	WrapperUI::Checkbox("Loop", &loop);
}

void Resources::Link::ShowInInspector()
{
	WrapperUI::Checkbox("Has Exit Time", &hasExitTime);
	WrapperUI::BeginDisabled(!hasExitTime);
	WrapperUI::InputFloat("Exit Time", &exitTime);
	WrapperUI::EndDisabled();
	WrapperUI::DragFloat("Transition Duration", &transitionDuration);
	WrapperUI::Separator();
	if (WrapperUI::Button("Add Condition"))
	{
		this->conditions.push_back(std::make_pair(nullptr, true));
	}
	int i = 0;
	// Define an array of option strings for the combo box
	std::vector<const char*> options;
	for (auto& parameter : animC->parameters)
	{
		options.push_back(parameter.first.c_str());
	}
	const char** optionPtrs = options.data();
	int numOptions = static_cast<int>(options.size());

	for (auto& condition : conditions)
	{
		WrapperUI::PushID(i);

		// Get the index of the currently selected option
		int selectedIndex = -1;
		for (int j = 0; j < numOptions; j++)
		{
			if (condition.first != nullptr && strcmp(condition.first->first.c_str(), options[j]) == 0)
			{
				selectedIndex = j;
				break;
			}
		}


		// Show the combo box and update the selected option
		if (WrapperUI::Combo("##conditionCombo", &selectedIndex, optionPtrs, numOptions))
		{
			if (selectedIndex >= 0 && selectedIndex < numOptions)
			{
				const char* selectedOption = options[selectedIndex];
				condition.first = &(*animC->parameters.find(selectedOption));
			}
			else
			{
				condition.first = nullptr;
			}
		}
		if (condition.first != nullptr)
		{
			WrapperUI::SameLine();
			if (WrapperUI::Checkbox("##conditionCheckbox", &condition.second))
			{
				animC->parameterUpdated = true;
			}
		}
		WrapperUI::SameLine();
		if (WrapperUI::Button("Remove"))
		{
			conditions.erase(conditions.begin() + i);
		}
		i++;
		WrapperUI::PopID();
	}

}

