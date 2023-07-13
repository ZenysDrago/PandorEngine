#include "pch.h"
#include <Resources/Sound.h>
#include <Core/App.h>

using namespace Resources;

Resources::Sound::Sound(std::string _name, ResourcesType _type) : IResources(_name, _type)
{
}

Resources::Sound::~Sound()
{
}

void Resources::Sound::Load()
{
	if (!isLoaded && LoadSound(Core::App::Get().audioManager, this->p_fullPath))
	{
		SetPosition({ 0,0,0 });
		isLoaded = true;
		hasBeenSent = true;
	}	
}

void Resources::Sound::SendResource()
{
}
