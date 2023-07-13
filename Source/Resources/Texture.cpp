#include "pch.h"

#include <Resources/Texture.h>
#include <Core/App.h>

using namespace Resources;
using namespace Core::Wrapper;

Texture::Texture(std::string _name, ResourcesType _type, unsigned int _texType, unsigned int _slot, unsigned int _pixelType) : IResources(_name, _type)
{
	m_texType = _texType;
	m_slot = _slot;
	m_pixelType = _pixelType;
}

void Resources::Texture::TexUni(Shader*& shader, const char* uniform, unsigned int unit)
{
	WrapperRHI::TextureData(shader, uniform, unit);
}

void Resources::Texture::Bind()
{
	if (hasBeenSent)
		WrapperRHI::TextureBind(ID, m_texType);
}

void Resources::Texture::UnBind()
{
	if (hasBeenSent)
		WrapperRHI::TextureUnBind(m_texType);
}

void Resources::Texture::Delete()
{
	if (hasBeenSent)
		WrapperRHI::TextureDelete(ID);
}

void Texture::ShowInInspector()
{
	int filter = m_filter == PR_LINEAR ? 0 : 1;
	if (WrapperUI::Combo("Filter", &filter, "Linear\0Nearest")) {
		m_filter = filter == 0 ? PR_LINEAR : PR_NEAREST;
		WrapperRHI::UpdateTexture(ID, m_filter, m_wrap);
	}
	int wrap = m_wrap == PR_REPEAT ? 0 : m_wrap == PR_CLAMP_TO_EDGE ? 1 : m_wrap == PR_CLAMP_TO_BORDER ? 2 : 3;
	if (WrapperUI::Combo("Wrapping", &wrap, "Repeat\0Clamp To Edge\0Clamp To Border\0Mirrored Repeat")) {
		m_wrap = wrap == 0 ? PR_REPEAT : wrap == 1 ? PR_CLAMP_TO_EDGE : wrap == 2 ? PR_CLAMP_TO_BORDER : PR_MIRRORED_REPEAT;
		WrapperRHI::UpdateTexture(ID, m_filter, m_wrap);
	}
}

void Texture::Active(int index)
{
	WrapperRHI::ActivateTexture(index);
}

Resources::Texture::~Texture()
{
	if (hasBeenSent)
		WrapperRHI::TextureDelete(ID);	
	if (bytes) {
		WrapperImage::ImageFree(bytes);
		bytes = nullptr;
	}

	//PrintLog("Texture is deleted ");
}

void Resources::Texture::Load()
{
	if (isLoaded)
		return;
	std::string extension = p_path.substr(p_path.find_last_of('.'));
	if (extension == ".tmb")
	{
		m_displayOnResourcesManager = false;
	}
	p_shouldBeLoaded = true;
	m_format = PR_RGBA;
	WrapperImage::FlipVerticalOnLoad(false);
	bytes = WrapperImage::Load(p_fullPath.c_str(), &widthImg, &heightImg, &numColCh, 4);
	isLoaded = true;

	//PrintLog("Resource loaded : %s", p_path.c_str());
	Core::App::Get().threadManager->Lock();
	Core::App::Get().AddResourceToSend(p_path);
	Core::App::Get().threadManager->Unlock();
}

void Resources::Texture::SendResource()
{
	if (hasBeenSent)
		return;
	//PrintLog("Sending Texture : %s", p_path.c_str());
	WrapperRHI::SendTexture(ID, m_format, m_texType, m_slot, m_pixelType, widthImg, heightImg, bytes, m_filter);
	WrapperImage::ImageFree(bytes);
	bytes = nullptr;
	hasBeenSent = true;
	//PrintLog("Resource Sent : %s", p_path.c_str());
}
