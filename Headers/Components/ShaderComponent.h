#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>

namespace Resources
{
	class Shader;
}

namespace Component
{
	class MeshComponent;

	class PANDOR_API ShaderComponent : public IComponent<ShaderComponent>
	{
	private:
		std::vector<Resources::Shader*> m_shaderList;

		void ShowUniforms(Resources::Shader* shader);

	public : 
		ShaderComponent();
		void Initialize() override;
		void ShowInInspector() override;

		std::string GetComponentName() override { return "Shader Component"; };
		void SetGameObject(Core::GameObject* go) override;
		void UpdateShaderList();

	};
}