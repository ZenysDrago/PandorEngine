#include <pch.h>
#include <Resources/PhysicMaterial.h>
#include <Core/App.h>
#include <Core/Wrappers/WrapperPhysic.h>
#include <Utils/Loader.h>

void Resources::PhysicMaterial::Load()
{
	p_shouldBeLoaded = true;
	if (isLoaded)
		return;
	uint32_t size;
	bool sucess;
	auto data = Utils::Loader::ReadFile(p_fullPath.c_str(), size, sucess);
	uint32_t pos = 0;
	if (!sucess)
	{
		PrintError("Failed to load %s", p_fullPath.c_str());
		return;
	}
	while (pos != size)
	{
		if (data[pos] == 's')
		{
			m_staticFriction = Utils::Loader::OBJ::GetFloat(data, pos, 5);
		}
		else if (data[pos] == 'd')
		{
			m_dynamicFriction = Utils::Loader::OBJ::GetFloat(data, pos, 5);
		}
		else if (data[pos] == 'r')
		{
			m_restitution = Utils::Loader::OBJ::GetFloat(data, pos, 5);
			break;
		}
		else
		{
			Utils::Loader::OBJ::SkipLine(data, pos);
		}
	}
	delete[] data;
	data = nullptr;
	isLoaded = true;
	SendResource();
}

void Resources::PhysicMaterial::SendResource()
{
	if (hasBeenSent)
		return;
	PrintLog("Sent : %s\n", p_path.c_str());
	hasBeenSent = true;
}

void Resources::PhysicMaterial::Save()
{
	std::string output;
	output += Utils::StringFormat("sf : %f\n", m_staticFriction);
	output += Utils::StringFormat("df : %f\n", m_dynamicFriction);
	output += Utils::StringFormat("re : %f\n", m_restitution);

	FILE* file;
	fopen_s(&file, GetFullPath().c_str(), "w");
	if (file)
	{
		PrintLog("Creating File Physic Material %s!", GetFullPath().c_str());
		fputs(output.c_str(), file);
		fclose(file);
	}
}

void Resources::PhysicMaterial::ShowInInspector()
{
	bool change = false;
	change |= WrapperUI::InputFloat("Static Friction", &m_staticFriction, 0);
	change |= WrapperUI::InputFloat("Dynamic Friction", &m_dynamicFriction, 0);
	change |= WrapperUI::InputFloat("Restitution", &m_restitution, 0);
	if (change)
		Core::App::Get().physic->UpdateMaterial(p_path, m_staticFriction, m_dynamicFriction, m_restitution);

	if (WrapperUI::Button("Save"))
		Save();
}

void Resources::PhysicMaterial::CreateMaterial()
{
	Core::App::Get().physic->CreateMaterial(p_path, m_staticFriction, m_dynamicFriction, m_restitution);
}

