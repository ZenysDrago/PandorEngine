#include "pch.h"
#include <Components/Animator.h>
#include <Components/SkeletalMeshComponent.h>
#include <Core/GameObject.h>

#include <Resources/Animation.h>
#include <Resources/AnimationController.h>
#include <Resources/Skeleton.h>
#include <Resources/ResourcesManager.h>

Component::Animator::Animator()
{

}

Component::Animator::~Animator()
{
	if (m_skeletalMesh && m_skeletalMesh->m_skeleton && m_skeletalMesh->m_skeleton->RootBone)
	{
		m_skeletalMesh->m_skeleton->RootBone->SetDefault();
	}
}

void Component::Animator::ShowInInspector()
{
	if (m_animationController)
	{
		WrapperUI::TextUnformatted(m_animationController->GetPath().c_str());
	}
	if (WrapperUI::Button("Animation"))
	{
		WrapperUI::OpenPopup("AnimationContollerPopup");
	}
	if (auto anim = Resources::ResourcesManager::Get()->ResourcePopup<Resources::AnimationController>("AnimationContollerPopup"))
	{
		m_animationController = Resources::ResourcesManager::Get()->GetOrLoad<Resources::AnimationController>(anim->GetPath());
	}

	WrapperUI::Checkbox("Play", &m_play);
}

void Component::Animator::Update()
{
	if (m_animationController)
	{
		m_animationController->parameterUpdated = false;
	}
}

void Component::Animator::EditorUpdate()
{
	if (m_animationController)
	{
		m_animationController->UpdateParameters(this, true);
	}
}

void Component::Animator::GameUpdate()
{
	// Get SkeletalMeshComponent
	if (!m_skeletalMesh)
	{
		m_skeletalMesh = gameObject->GetComponent<SkeletalMeshComponent>();
	}

	if (m_animationController && !m_currentState)
	{
		m_currentState = m_animationController->GetStateByName("Entry");
	}
	if (m_animationController)
	{
		m_animationController->UpdateParameters(this, false);
	}
	if (m_animationController && m_skeletalMesh && m_animationController->IsLoaded())
	{
		m_animationController->Update(this);
	}


	// TMP
	if (WrapperUI::IsKeyPressed(Key::Key_Space) && m_play)
	{
		active = !active;
	}
	if (m_animationController )
		SetBoolean("ShouldRun", active);
}

void Component::Animator::SetBoolean(const std::string& name, bool value)
{
	if (m_parameters.count(name))
	{
		m_parameters[name] = value;
	}
	m_conditionUpdated = true;
}

void Component::Animator::SetSkeletalMesh(class SkeletalMeshComponent* _skeletalMesh)
{
	m_skeletalMesh = _skeletalMesh;
}

void Component::Animator::SetTransitionLink(Resources::Link* link)
{
	m_transitionLink = link;
	m_currentTime = 0.f;
	m_elapsedTime = 0.f;
}

void Component::Animator::IncrementTime(Resources::Animation* anim)
{
	if (m_currentTime < anim->KeyCount + 1)
		m_currentTime += fmodf(WrapperUI::GetDeltaTime() * 30 * m_currentState->speed, (float)anim->KeyCount);

	if (m_currentTime > anim->KeyCount && m_currentState->loop)
		m_currentTime = 0;
	else if (m_currentTime <= 0 && m_currentState->loop)
		m_currentTime = (float)anim->KeyCount;
}

void Component::Animator::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		m_animationController = Resources::ResourcesManager::Get()->GetOrLoad<AnimationController>(line);

	while (getline(sceneFile, line) && line != "end") {}
}

std::ostream& Component::Animator::operator<<(std::ostream& os)
{
	if (m_animationController)
		os << m_animationController->GetPath() << '\n';
	else
		os << "nullptr" << '\n';
	return os;
}
