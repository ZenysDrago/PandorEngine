#include "pch.h"

#include <Resources/Shader.h>
#include <Core/App.h>
#include <Utils/Loader.h>

using namespace Resources;

/* ===================================================================================================== */
/*											Shader Class											 	 */
/* ===================================================================================================== */

Shader::Shader(const std::string& _name, const std::string& vertexPath, const std::string& fragmentPath, ResourcesType _type) : IResources(_name, _type)
{
	p_shouldBeLoaded = true;
	vertShader = ResourcesManager::Get()->GetOrLoad<VertexShader>(vertexPath);
	fragShader = ResourcesManager::Get()->GetOrLoad<FragmentShader>(fragmentPath);

	if (vertShader)
		vertShader->AddShader(this);
	else
		PrintError("Vertex Shader %s Not Found", vertexPath.c_str());

	if (fragShader)
		fragShader->AddShader(this);
	else
		PrintError("Fragment Shader %s Not Found", fragmentPath.c_str());

	ResourcesManager* resourceManager = ResourcesManager::Get();
	Core::App::Get().AddResourceToSend(this->p_path);

	isLoaded = true;
}

Resources::Shader::Shader(const std::string& _name, ResourcesType _type) : IResources(_name, _type)
{
	p_shouldBeLoaded = true;
	std::string vertexPath = _name + ".vert";
	std::string fragmentPath = _name + ".frag";

	vertShader = ResourcesManager::Get()->GetOrLoad<VertexShader>(vertexPath);
	fragShader = ResourcesManager::Get()->GetOrLoad<FragmentShader>(fragmentPath);

	if (vertShader)
		vertShader->AddShader(this);
	else
		PrintError("Vertex Shader %s Not Found", vertexPath.c_str());

	if (fragShader)
		fragShader->AddShader(this);
	else
		PrintError("Fragment Shader %s Not Found", fragmentPath.c_str());

	ResourcesManager* resourceManager = ResourcesManager::Get();
	if (vertShader && fragShader)
		Core::App::Get().AddResourceToSend(this->p_path);


	isLoaded = true;
}

Shader::~Shader()
{
}

void Shader::Load()
{
	p_shouldBeLoaded = true;
	isLoaded = true;
}

void Shader::SendResource()
{
	if (!vertShader || !fragShader || !vertShader->IsLoaded() || !fragShader->IsLoaded())
		return;
	PrintLog("Sending Shader : %s", p_path.c_str());

	auto success = WrapperRHI::SendShader(vertShader->vertexFile, fragShader->fragmentFile, ID, this);

	if (!success)
		return;
	hasBeenSent = true;
	fragShader->SetSendingStatus(true);
	vertShader->SetSendingStatus(true);

	PrintLog("Shader sent : %s", p_path.c_str());
}

void Shader::Use()
{
	if (hasBeenSent)
	{
		WrapperRHI::ShaderUse(ID);
		if (!uniformSet)
			WrapperRHI::GetAllUniform(this);
	}
}

void Shader::Delete()
{
	WrapperRHI::ShaderDelete(ID);
}

void Shader::SetFragLoaded(bool _isFragLoaded)
{

	while (!shaderMutex.try_lock()) {}
	isFragLoaded = _isFragLoaded;
	CheckResourceLoaded();
	shaderMutex.unlock();
}

void Shader::SetVertLoaded(bool _isVertLoaded)
{
	while (!shaderMutex.try_lock()) {}
	isVertLoaded = _isVertLoaded;
	CheckResourceLoaded();
	shaderMutex.unlock();
}

void Shader::CheckResourceLoaded()
{
	if (vertShader->IsLoaded() && fragShader->IsLoaded())
	{
		isLoaded = true;
		Core::App::Get().AddResourceToSend(p_path);
	}
}

void Shader::Recompile()
{
	if (hasBeenSent)
	{
		PrintLog("Recompile Shader %s", p_path.c_str());
		auto success = WrapperRHI::SendShader(vertShader->vertexFile, fragShader->fragmentFile, ID, this);
	}
}

int Resources::Shader::GetLocation(const std::string& locationName)
{
	if (!HasBeenSent())
		return -1;
	if (m_locations.count(locationName))
		return m_locations[locationName];
	else
		return m_locations[locationName] = WrapperRHI::ShaderGetLocation(ID, locationName.c_str());

}

/* ===================================================================================================== */
/*											Fragment Shader Class										 */
/* ===================================================================================================== */

FragmentShader::FragmentShader(std::string _name, ResourcesType _type) : IResources(_name, _type)
{
}

FragmentShader::~FragmentShader()
{
}

void FragmentShader::Load()
{
	fragmentFile.clear();
	Core::App::Get().threadManager->Lock();
	fragmentFile = Utils::Loader::ReadFile(p_path);
	Core::App::Get().threadManager->Unlock();


	isLoaded = true;
	PrintLog("Fragment Shader Loaded : %s", p_path.c_str());
}

void FragmentShader::SendResource()
{
}

void FragmentShader::Recompile()
{
	fragmentFile = Utils::Loader::ReadFile(p_path);

	for (auto& shaders : GetShaders())
	{
		shaders->Recompile();
	}
}


/* ===================================================================================================== */
/*											Vertex Shader Class											 */
/* ===================================================================================================== */

VertexShader::VertexShader(std::string _name, ResourcesType _type) : IResources(_name, _type)
{
}

VertexShader::~VertexShader()
{
}

void VertexShader::Load()
{
	vertexFile.clear();
	Core::App::Get().threadManager->Lock();
	vertexFile = Utils::Loader::ReadFile(p_path);
	Core::App::Get().threadManager->Unlock();

	isLoaded = true;
	PrintLog("Vertex Shader Loaded : %s", p_path.c_str());
}

void VertexShader::SendResource()
{
}

void VertexShader::Recompile()
{
	vertexFile = Utils::Loader::ReadFile(p_path);

	for (auto& shaders : GetShaders())
	{
		shaders->Recompile();
	}
}
