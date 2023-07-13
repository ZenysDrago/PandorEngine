#include "pch.h"

#include <Resources/Skybox.h>
#include <Core/App.h>

using namespace Resources;
using namespace Core::Wrapper;

static float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

static unsigned int skyboxVAO, skyboxVBO;
static size_t verticesSize = sizeof(skyboxVertices) / sizeof(float);

Skybox::Skybox(std::string _name, ResourcesType _type) : IResources(_name, _type)
{
	if (!skyboxVAO)
		WrapperRHI::GenVertex(skyboxVAO, skyboxVBO, skyboxVertices, verticesSize);
}

void Resources::Skybox::TexUni(Shader*& shader, const char* uniform, unsigned int unit)
{
	WrapperRHI::TextureData(shader, uniform, unit);
}

void Resources::Skybox::Bind()
{
	if (hasBeenSent)
		WrapperRHI::TextureBind(ID, PR_TEXTURE_CUBE_MAP);
}

void Resources::Skybox::UnBind()
{
	if (hasBeenSent)
		WrapperRHI::TextureUnBind(PR_TEXTURE_CUBE_MAP);
}

void Resources::Skybox::Delete()
{
	if (hasBeenSent)
		WrapperRHI::TextureDelete(ID);
}

void Skybox::Active(int index)
{
	WrapperRHI::ActivateTexture(index);
}

void Skybox::ShowInInspector()
{
	//WrapperUI::Image(ID, Math::Vector2(128, 128));
}

void Resources::Skybox::Render(const Math::Matrix4& VP)
{
	Shader* skyboxShader = Resources::ResourcesManager::Get()->GetSkyboxShader();
	if (!skyboxShader || !skyboxShader->HasBeenSent())
		return;

	WrapperRHI::SetDephtFunc(true);
	skyboxShader->Use();
	WrapperRHI::ShaderSendMat4(skyboxShader->GetLocation("VP"), VP);
	Active(0);
	Bind();
	WrapperRHI::ShaderSendInt(skyboxShader->GetLocation("skybox"), 0);
	WrapperRHI::BindVAO(skyboxVAO);
	WrapperRHI::DrawArrays(0, 36, false, true);
	WrapperRHI::SetDephtFunc(false);
}

Resources::Skybox::~Skybox()
{
	PrintLog("Skybox is deleted ");
}

void Resources::Skybox::Load()
{
	if (isLoaded)
		return;

	p_shouldBeLoaded = true;

	WrapperImage::FlipVerticalOnLoad(false);

	SpecificLoad();
	isLoaded = true;

	Core::App::Get().threadManager->Lock();
	Core::App::Get().AddResourceToSend(p_path);
	Core::App::Get().threadManager->Unlock();
}

void Resources::Skybox::SendResource()
{
	if (hasBeenSent)
		return;

	SpecificSend();
	hasBeenSent = true;
}

//-----------------------------------------------------------------------------------

void Resources::CubeMap::SpecificLoad()
{
	m_texData = WrapperImage::Load(p_fullPath.c_str(), &m_texWidth, &m_texHeight, &m_nrChannels, 0);
}

void Resources::CubeMap::SpecificSend()
{
	WrapperRHI::SendCubeMap(ID, m_texWidth, m_texHeight, m_nrChannels, m_texData);
	WrapperImage::ImageFree(m_texData);
	m_texData = nullptr;
}

Resources::CubeMap::~CubeMap()
{
	if (hasBeenSent)
		WrapperRHI::TextureDelete(ID);
	if (m_texData) {
		WrapperImage::ImageFree(m_texData);
		m_texData = nullptr;
	}
}

Resources::SixSided::SixSided(std::string _name, std::string faces[6]) : Skybox(_name, ResourcesType::Skybox)
{
	for (int i = 0; i < 6; i++)
		m_faces[i] = faces[i];
}

Resources::SixSided::~SixSided()
{
	if (hasBeenSent)
		WrapperRHI::TextureDelete(ID);
	for (auto&& data : m_texData) {
		if (data) {
			WrapperImage::ImageFree(data);
			data = nullptr;
		}
	}
}

void Resources::SixSided::SpecificLoad()
{
	int nrChannels;
	for (int i = 0; i < 6; i++)
		m_texData[i] = WrapperImage::Load(m_faces[i].c_str(), &m_texWidths[i], &m_texHeights[i], &nrChannels, 0);
}

void Resources::SixSided::SpecificSend()
{
	WrapperRHI::SendSixSided(ID, m_texWidths, m_texHeights, m_texData);
	for (int i = 0; i < 6; i++) {
		WrapperImage::ImageFree(m_texData[i]);
		m_texData[i] = nullptr;
	}
}
