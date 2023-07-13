#include "pch.h"

#include <Components/ShaderComponent.h>
#include <Core/GameObject.h>
#include <Resources/Shader.h>
#include <Components/MeshComponent.h>
#include <Resources/Mesh.h>
#include <Resources/Material.h>

using namespace Component;

Component::ShaderComponent::ShaderComponent()
{
	Initialize();
}

void Component::ShaderComponent::Initialize()
{

}

void Component::ShaderComponent::ShowInInspector()
{
	if (m_shaderList.size() == 0)
		return;

	WrapperUI::Text("Shader component have %d shader stored", m_shaderList.size());

	if (WrapperUI::TreeNode("Shaders"))
	{
		for (int i = 0; i < m_shaderList.size(); i++)
		{
			WrapperUI::PushID(i);
			if (WrapperUI::TreeNodeEx(m_shaderList[i]->GetName().c_str()))
			{
				ShowUniforms(m_shaderList[i]);
				WrapperUI::TreePop();
			}
			WrapperUI::PopID();
		}

		WrapperUI::TreePop();
	}
}

void Component::ShaderComponent::SetGameObject(Core::GameObject* go)
{
	this->gameObject = go;

	UpdateShaderList();
}

void Component::ShaderComponent::UpdateShaderList()
{
	m_shaderList.clear();
	std::vector<Component::MeshComponent*> meshComponents = gameObject->GetComponentsInChildren<Component::MeshComponent>();

	if (meshComponents.size() == 0)
	{
		PrintError("Didn't find Mesh component abort ShaderComponent initialization");
		return;
	}

	for (auto mesh : meshComponents)
	{
		std::vector<Resources::Material*> materials = mesh->GetMaterials();
		for (auto mat : materials)
		{
			m_shaderList.push_back(mat->GetShader());
		}
	}
}

void Component::ShaderComponent::ShowUniforms(Resources::Shader* shader)
{
	WrapperUI::Text("Uniforms : ");
	WrapperUI::Text(shader->GetName().c_str());

	std::vector<Resources::ShaderVariables>& dataFinal = shader->GetShaderVariables();
	for (auto uniform : dataFinal)
	{
		float values[16] = {};
		int	  ivalue = 0;
		const char* name = uniform.name.c_str();
		bool bvalue;

		switch (uniform.type)
		{
		case PR_VECTOR2:
			WrapperRHI::ShaderGetVec2(shader->ID, uniform.index, values);
			if (WrapperUI::SliderFloat2(name, values, -50, 50))
			{
				Vector2 v = { values[0], values[1] };
				WrapperRHI::ShaderSendVec2(uniform.location, v);
			}
			break;
		case PR_VECTOR3:
			WrapperRHI::ShaderGetVec3(shader->ID, uniform.index, values);
			if (WrapperUI::SliderFloat3(name, values, -50, 50))
			{
				Vector3 v = { values[0], values[1] , values[2] };
				WrapperRHI::ShaderSendVec3(uniform.location, v);
			}
			break;
		case PR_VECTOR4:
			WrapperRHI::ShaderGetVec4(shader->ID, uniform.index, values);
			if (WrapperUI::SliderFloat4(name, values, -50, 50))
			{
				Vector4 v = { values[0], values[1] , values[2], values[3] };
				WrapperRHI::ShaderSendVec4(uniform.location, v);
			}
			break;
		case PR_MAT4:
		{
			WrapperRHI::ShaderGetMat4(shader->ID, uniform.index, values);
			if (WrapperUI::SliderMat4(name, values, -50, 50))
			{
				Matrix4 mat = values;
				WrapperRHI::ShaderSendMat4(uniform.location, mat);
			}
			break;
		}
		case PR_BOOL:
			WrapperRHI::ShaderGetBool(shader->ID, uniform.index, &bvalue);
			if (WrapperUI::Checkbox(name, &bvalue))
			{
				WrapperRHI::ShaderSendBool(uniform.location, bvalue);
			}
			break;
		case PR_FLOAT:
			WrapperRHI::ShaderGetFloat(shader->ID, uniform.index, values);
			if (WrapperUI::SliderFloat(name, values, -50, 50))
			{
				WrapperRHI::ShaderSendFloat(uniform.location, values[0]);
			}
			break;
		case PR_INT:
			WrapperRHI::ShaderGetInt(shader->ID, uniform.index, &ivalue);
			if (WrapperUI::SliderInt(name, &ivalue, -50, 50))
			{
				WrapperRHI::ShaderSendInt(uniform.location, ivalue);
			}
			break;
		default:
			WrapperUI::Text("Uniform of name %s is not in a supported type :: type is %d", name, uniform.type);
			break;
		}
	}
}
