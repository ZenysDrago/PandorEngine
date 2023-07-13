#include "pch.h"

#include <Render/Framebuffer.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Shader.h>
#include <Resources/Texture.h>
#include <Core/App.h>
#include <Render/Camera.h>
#include <Utils/Utils.h>
#include <Core/Scene.h>
static std::unordered_map<size_t, bool> index = {};

Render::Framebuffer::Framebuffer()
{
}

Render::Framebuffer::~Framebuffer()
{
	//delete m_buffer;
	if (m_renderBuffer)
	{
		index.erase(m_index);
		delete m_renderBuffer;
		m_renderBuffer = nullptr;
	}
	if (m_material)
	{
		if (m_material->GetTexture())
		{
			Resources::ResourcesManager::Get()->Delete(m_material->GetTexture()->GetPath());
		}
		delete m_material;
		m_material = nullptr;
	}
	if (m_postProcess)
	{
		delete m_postProcess;
		m_postProcess = nullptr;
	}
}

void Render::Framebuffer::Initialize(const Math::Vector2& size)
{
	int freeIndex = 0;
	while (index.count(freeIndex) > 0) {
		freeIndex++;
	}
	m_index = freeIndex;
	index[m_index] = true;
	auto shader = Core::App::Get().resourcesManager->GetOrLoad<Resources::Shader>(ENGINEPATH"Shaders/InvertedShader/inverted");

	Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(ENGINEPATH"Models/Plane.obj");
	Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(ENGINEPATH"Models/Plane2.obj");
	// Create Texture.
	auto name = Utils::StringFormat("Framebuffer%d", m_index);
	auto texture = new Resources::Texture(name, Resources::ResourcesType::Texture);
	texture->isLoaded = true;
	ResourcesManager::Get()->Add(name, texture);
	m_material = new Resources::Material("FrameBufferMat", ResourcesType::Material);
	m_material->SetTexture(texture);
	if (shaderName != "null")
		m_material->SetShader(Core::App::Get().resourcesManager->GetOrLoad<Resources::Shader>(shaderName));
	else
		m_material->SetShader(shader);

	WrapperRHI::SendTexture(texture->ID, PR_RGBA, PR_TEXTURE2D, 1, PR_UNSIGNED_BYTE, (int)size.x, (int)size.y, NULL);

	m_renderBuffer = new Core::Wrapper::WrapperRHI::RenderBuffer();
	m_renderBuffer->Generate(texture->ID, size, (uint32_t)m_index);

	m_renderBuffer->UnBind();
	texture->SetSent();
}
void Render::Framebuffer::PreUpdate()
{
	auto windowSize = Core::App::Get().window->GetSize();
	if (!m_postProcess && m_enablePostProcessing) {
		m_postProcess = new Render::Framebuffer();
		m_postProcess->Initialize(windowSize);
	}
	if (m_size != windowSize) {
		m_renderBuffer->Bind();
		m_material->GetTexture()->Bind();
		m_renderBuffer->Resize(windowSize);
		m_size = windowSize;
		m_material->GetTexture()->UnBind();
		m_renderBuffer->UnBind();
	}
	if (m_enablePostProcessing && m_postProcess->m_size != windowSize)
	{
		m_postProcess->m_renderBuffer->Bind();
		m_postProcess->m_material->GetTexture()->Bind();
		m_postProcess->m_renderBuffer->Resize(m_size);
		m_postProcess->m_size = windowSize;
		m_postProcess->m_material->GetTexture()->UnBind();
		m_postProcess->m_renderBuffer->UnBind();
	}
#ifndef PANDOR_GAME
	m_renderBuffer->Bind();
#else
	if (m_enablePostProcessing && m_postProcess)
		m_postProcess->m_renderBuffer->Bind();
#endif
}
void Render::Framebuffer::Draw()
{
#ifndef PANDOR_GAME
	m_renderBuffer->UnBind();
	if (m_enablePostProcessing && m_postProcess) {
		m_postProcess->m_renderBuffer->Bind();

		WrapperRHI::ClearColorAndBuffer({ 0.2f, 0.3f, 0.3f, 1.0f });
		WrapperRHI::DepthActive();
		if (!m_plane)
		{
			m_plane = Resources::ResourcesManager::Get()->Find<Resources::Mesh>(ENGINEPATH"Models/Plane.obj:Plane");
		}
		if (m_plane)
			m_plane->Render(Math::Matrix4(), Math::Matrix4(), { m_material });

		m_postProcess->m_renderBuffer->UnBind();
	}
#else
	if(m_enablePostProcessing && m_postProcess) {
		if (m_postProcess->m_material->GetShader() != m_material->GetShader())
		{
			m_postProcess->m_material->SetShader(m_material->GetShader());
		}
		m_postProcess->m_renderBuffer->UnBind();
		WrapperRHI::ClearColor({ 0.2f, 0.3f, 0.3f, 1.0f });
		WrapperRHI::DepthDisable();
		if (!m_plane)
		{
			m_plane = Resources::ResourcesManager::Get()->Find<Resources::Mesh>(ENGINEPATH"Models/Plane2.obj:Plane2");
		}
		if (m_plane)
			m_plane->Render(Math::Matrix4(), Math::Matrix4(), { m_postProcess->m_material });

	}
#endif
}

void Render::Framebuffer::SaveFrameBufferToTexture(std::string name, const Vector2& size)
{
	unsigned char* data = new unsigned char[(size_t)m_size.x * (size_t)m_size.y * 4];
	m_renderBuffer->Read((int)m_size.x, (int)m_size.y, data);
	unsigned char* out_data = new unsigned char[(size_t)size.x * (size_t)size.y * 4];
	WrapperImage::Resize((int)m_size.x, (int)m_size.y, data, (int)size.x, (int)size.y, out_data);
	WrapperImage::Save(name.c_str(), (int)size.x, (int)size.y, out_data);
	delete[] data;
	data = nullptr;
	delete[] out_data;
	out_data = nullptr;
}

unsigned int Render::Framebuffer::GetFrameBufferTexture()
{
	if (m_enablePostProcessing && m_postProcess)
		return m_postProcess->m_material->GetTexture()->ID;
	else
		return m_material->GetTexture()->ID;
}

Resources::Texture* Render::Framebuffer::GetTexture()
{
	return m_material->GetTexture();
}

void Render::Framebuffer::SetShader(Resources::Shader* shader)
{
	m_material->SetShader(shader);
}

Resources::Shader* Render::Framebuffer::GetShader()
{
	return m_material->GetShader();
}

std::ostream& Render::Framebuffer::operator<<(std::ostream& os)
{
	os << "===" << "\n";
	os << "FrameBuffer" << "\n";

	if (m_postProcess)
		m_postProcess->operator<<(os);
	else
		os << "nullptr" << "\n";

	if (m_material->GetShader())
		os << m_material->GetShader()->GetPath() << "\n";
	else
		os << "nullptr" << "\n";

	os << m_enablePostProcessing << "\n";

	os << "end" << "\n";
	return os;
}

void Render::Framebuffer::ReadFrameBufferFromFile(Render::Framebuffer* framebuffer, std::fstream& sceneFile)
{
	std::string line;
	if (!framebuffer) {
		framebuffer = new Render::Framebuffer();
		framebuffer->Initialize({ 800, 600 });
	}

	if (getline(sceneFile, line) && line != "end")
	{
		if (line != "FrameBuffer")
		{
			PrintError("Bad reading of framebuffer");
			delete framebuffer;
			framebuffer = nullptr;
			return;
		}
	}

	if (getline(sceneFile, line) && line != "end")
	{
		if (line == "nullptr")
			framebuffer->m_postProcess = nullptr;
		else
			ReadFrameBufferFromFile(framebuffer->m_postProcess, sceneFile);
	}

	if (getline(sceneFile, line) && line != "end")
	{
		if (line != "nullptr")
			framebuffer->shaderName = line;
	}

	if (getline(sceneFile, line) && line != "end")
		framebuffer->m_enablePostProcessing = (bool)std::atoi(line.c_str());

	while (getline(sceneFile, line) && line != "end") {}
}