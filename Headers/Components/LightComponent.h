#pragma once
#include "PandorAPI.h"

#include <Math/Maths.h>
#include <Components/BaseComponent.h>
namespace Render
{
	class EditorIcon;
}
namespace Resources
{
	class Shader;
}
namespace Core::Wrapper::WrapperRHI
{
	class Line;
}

namespace Component
{
	enum class PANDOR_API LightType
	{
		DIRECTIONAL,
		POINT,
		SPOT,
	};

	std::string EnumToString(LightType type);

	class PANDOR_API LightComponent : public IComponent<LightComponent>
	{
	protected:
		LightType p_type;
		Render::EditorIcon* p_icon;

	public:
		float intensity = 1.0f;
		Math::Vector3 color = Vector3(1, 1, 1);

	private:
		void OnDestroy() override;

	protected :
		static void ReadBaseLight(std::fstream& sceneFile, Component::LightComponent* component);

	public:
		LightComponent();
		~LightComponent();
		void Initialize() override;
		
		void EditorDraw() override;

		void DrawPicking(int ID) override;

		virtual void ShowInInspector() override;

		std::string GetComponentName() override { return "Light Component"; }
		virtual void SetUniforms(Resources::Shader* shaderProgram, const int index) {}

		LightType GetType() const { return p_type; }
	};

	class PANDOR_API DirectionalLight : public LightComponent
	{
	private:
	public:
		DirectionalLight();
		void SetUniforms(Resources::Shader* shaderProgram, const int index) override;
		void EditorDraw() override;
		std::ostream& operator<<(std::ostream& os) override;

		BaseComponent* Clone() const { return new DirectionalLight(*this); };
		std::string GetComponentName() override { return "Directional Light"; }

		void ReadComponent(std::fstream& sceneFile) override;
	};

	class PANDOR_API PointLight : public LightComponent
	{
	public:
		float radius = 10.0f;

	public:
		PointLight();
		void SetUniforms(Resources::Shader* shaderProgram, const int index) override;
		void ShowInInspector() override;
		void EditorDraw() override;
		std::ostream& operator<<(std::ostream& os) override;

		BaseComponent* Clone() const { return new PointLight(*this); };
		std::string GetComponentName() override { return "Point Light"; }

		void ReadComponent(std::fstream& sceneFile) override;
	};

	class PANDOR_API SpotLight : public LightComponent
	{
	public:
		float innerAngle = 45;
		float outerAngle = 63;

	public:
		SpotLight();
		void SetUniforms(Resources::Shader* shaderProgram, const int index) override;
		void ShowInInspector() override;
		void EditorDraw() override;
		std::ostream& operator<<(std::ostream& os) override;

		BaseComponent* Clone() const { return new SpotLight(*this); };
		std::string GetComponentName() override { return "Spot Light"; };

		void ReadComponent(std::fstream& sceneFile) override;
	};
}



