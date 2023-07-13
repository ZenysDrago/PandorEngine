#include "pch.h"
#include <Resources/Font.h>

#include <Core/Wrappers/WrapperFont.h>

#include <Core/App.h>

Resources::Font::~Font()
{
}

void Resources::Font::Load()
{
	if (isLoaded)
		return;
	isLoaded = true;
	Core::App::Get().AddResourceToSend(this->GetPath());

}

void Resources::Font::SendResource()
{
	Core::Wrapper::WrapperFont::LoadFont(this);

	this->hasBeenSent = true;
}
