#include "pch.h"

#include <Components/LightComponent.h>
#include <LowRenderer/LightManager.h>
#include <Resources/Shader.h>
#ifndef PANDOR_GAME
#include <Render/EditorIcon.h>
#endif
#include <Core/Wrappers/WrapperRHI.h>
#include <Core/GameObject.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Components/Transform.h>
#include <Resources/ResourcesManager.h>

using namespace Core::Wrapper::WrapperRHI;
using namespace Component;

LightComponent::LightComponent()
{
}

Component::LightComponent::~LightComponent()
{
}

void Component::LightComponent::Initialize()
{
	LowRenderer::LightManager::getInstance().Create(this);
#ifndef PANDOR_GAME
	p_icon = new Render::EditorIcon();
	p_icon->Initialize();
	p_icon->SetIcon(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(ENGINEPATH"Textures/icons/Light.png"));
#endif
}

void LightComponent::DrawPicking(int ID)
{
#ifndef PANDOR_GAME
	p_icon->DrawPicking(gameObject->transform->GetWorldPosition(), ID);
#endif
}

void Component::LightComponent::OnDestroy()
{
	LowRenderer::LightManager::getInstance().Delete(this);
#ifndef PANDOR_GAME
	delete p_icon;
	p_icon = nullptr;
#endif
}

void Component::LightComponent::ReadBaseLight(std::fstream& sceneFile, Component::LightComponent* component)
{
	std::string line;

	if (getline(sceneFile, line))
		if (line != "end")
			component->p_componentName = line;

	if (getline(sceneFile, line))
		if (line != "end")
			component->intensity = (float)std::atof(line.c_str());

	if (getline(sceneFile, line))
		if (line != "end")
			component->color = Math::ParseVector3(line);

}
void LightComponent::EditorDraw()
{
#ifndef PANDOR_GAME
	p_icon->Draw(gameObject->transform->GetWorldPosition(), gameObject->IsSelected());
#endif
}

void LightComponent::ShowInInspector()
{
	WrapperUI::DragFloat("Intensity", &intensity, 1.0f, 0.0f, 500.0f);
	WrapperUI::ColorEdit3("Color", &color.x);
}

DirectionalLight::DirectionalLight()
{
	p_type = LightType::DIRECTIONAL;
}

void Component::DirectionalLight::SetUniforms(Resources::Shader* shaderProgram, const int index)
{
	std::string indexStr = std::to_string(index);
	bool enable = gameObject->GetScene() == Core::SceneManager::Get()->GetCurrentScene();
	ShaderSendInt(shaderProgram->GetLocation("dirLights[" + indexStr + "].enable"), enable);
	ShaderSendFloat(shaderProgram->GetLocation("dirLights[" + indexStr + "].intensity"), intensity);
	ShaderSendVec3(shaderProgram->GetLocation("dirLights[" + indexStr + "].color"), color);
	ShaderSendVec3(shaderProgram->GetLocation("dirLights[" + indexStr + "].direction"), gameObject->transform->GetForwardVector());
}
void DirectionalLight::EditorDraw()
{
	LightComponent::EditorDraw();
	if (!gameObject->IsSelected())
		return;
	auto worldPosition = gameObject->transform->GetWorldPosition();

	WrapperRHI::Line::Get().Color = Vector4(0.980f, 0.804f, 0.0196f, 1.0f);
	WrapperRHI::Line::Get().Draw(worldPosition, worldPosition + gameObject->transform->GetForwardVector() * 5.f);
}

std::ostream& Component::DirectionalLight::operator<<(std::ostream& os)
{
	os << color << "\n";
	os << intensity << "\n";

	return os;
}

void DirectionalLight::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line))
		if (line != "end")
			color = Math::ParseVector3(line);
	if (getline(sceneFile, line))
		if (line != "end")
			intensity = std::stof(line);
	while (getline(sceneFile, line) && line != "end") {}
}

PointLight::PointLight()
{
	p_type = LightType::POINT;
}

void Component::PointLight::SetUniforms(Resources::Shader* shaderProgram, const int index)
{
	std::string indexStr = std::to_string(index);

	ShaderSendInt(shaderProgram->GetLocation("pointLights[" + indexStr + "].enable"), gameObject->GetScene() == Core::SceneManager::Get()->GetCurrentScene());
	ShaderSendFloat(shaderProgram->GetLocation("pointLights[" + indexStr + "].intensity"), intensity);
	ShaderSendVec3(shaderProgram->GetLocation("pointLights[" + indexStr + "].color"), color);
	ShaderSendVec3(shaderProgram->GetLocation("pointLights[" + indexStr + "].position"), gameObject->transform->GetWorldPosition());
	ShaderSendFloat(shaderProgram->GetLocation("pointLights[" + indexStr + "].radius"), radius);

	
}

void PointLight::ShowInInspector()
{
	LightComponent::ShowInInspector();
	WrapperUI::DragFloat("Attenuation Radius", &radius, 1.0f, 0.0f, 1000.0f);
}

void PointLight::EditorDraw()
{
	LightComponent::EditorDraw();
	if (!gameObject->IsSelected())
		return;
	auto worldPosition = gameObject->transform->GetWorldPosition();
	WrapperRHI::Line::Get().DrawCircle(worldPosition, Vector3::Up(), radius, 64, Vector4(0.980f, 0.804f, 0.0196f, 1.0f));
	WrapperRHI::Line::Get().DrawCircle(worldPosition, Vector3::Forward(), radius, 64, Vector4(0.980f, 0.804f, 0.0196f, 1.0f));
	WrapperRHI::Line::Get().DrawCircle(worldPosition, Vector3::Right(), radius, 64, Vector4(0.980f, 0.804f, 0.0196f, 1.0f));
}

std::ostream& Component::PointLight::operator<<(std::ostream& os)
{
	os << color << '\n';
	os << intensity << '\n';
	os << radius << '\n';

	return os;
}

void PointLight::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line))
		if (line != "end")
			color = Math::ParseVector3(line);

	if (getline(sceneFile, line))
		if (line != "end")
			intensity = std::stof(line);

	if (getline(sceneFile, line))
		if (line != "end")
			radius = std::stof(line);
	while (getline(sceneFile, line) && line != "end") {}
}

SpotLight::SpotLight()
{
	p_type = LightType::SPOT;
}

void Component::SpotLight::SetUniforms(Resources::Shader* shaderProgram, const int index)
{
	std::string indexStr = std::to_string(index);

	ShaderSendInt(shaderProgram->GetLocation("spotLights[" + indexStr + "].enable"), gameObject->GetScene() == Core::SceneManager::Get()->GetCurrentScene());
	ShaderSendFloat(shaderProgram->GetLocation("spotLights[" + indexStr + "].intensity"), intensity);
	ShaderSendVec3(shaderProgram->GetLocation("spotLights[" + indexStr + "].color"), color);
	ShaderSendVec3(shaderProgram->GetLocation("spotLights[" + indexStr + "].direction"), gameObject->transform->GetForwardVector());
	ShaderSendVec3(shaderProgram->GetLocation("spotLights[" + indexStr + "].position"), gameObject->transform->GetWorldPosition());
	ShaderSendFloat(shaderProgram->GetLocation("spotLights[" + indexStr + "].cutOff"), cosf(innerAngle * DEG2RAD));
	ShaderSendFloat(shaderProgram->GetLocation("spotLights[" + indexStr + "].outerCutOff"), cosf(outerAngle * DEG2RAD));
}

void SpotLight::ShowInInspector()
{
	LightComponent::ShowInInspector();
	if (WrapperUI::DragFloat("Inner Angle", &innerAngle, 0.1f, 0.0f, 90.0f) && innerAngle > outerAngle)
		outerAngle = innerAngle;
	WrapperUI::DragFloat("Outer Angle", &outerAngle, 0.1f, innerAngle, 120.0f);
}

void SpotLight::EditorDraw()
{
	LightComponent::EditorDraw();
	if (!gameObject->IsSelected())
		return;
	auto worldPosition = gameObject->transform->GetWorldPosition();
	WrapperRHI::Line::Get().DrawCone(worldPosition, gameObject->transform->GetWorldRotation(), 0.01f, outerAngle, 25.f, Vector4(0.980f, 0.804f, 0.0196f, 1.0f));
}

std::ostream& Component::SpotLight::operator<<(std::ostream& os)
{
	os << color << '\n';
	os << intensity << '\n';
	os << innerAngle << '\n';
	os << outerAngle << '\n';

	return os;
}

void SpotLight::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line))
		if (line != "end")
			color = Math::ParseVector3(line);

	if (getline(sceneFile, line))
		if (line != "end")
			intensity = std::stof(line);

	if (getline(sceneFile, line))
		if (line != "end")
			innerAngle = std::stof(line);

	if (getline(sceneFile, line))
		if (line != "end")
			outerAngle = std::stof(line);

	while (getline(sceneFile, line) && line != "end") {}
}

std::string Component::EnumToString(LightType type)
{
	switch (type)
	{
	case LightType::DIRECTIONAL:
		return "Directional";
	case LightType::POINT:
		return "Point";
	case LightType::SPOT:
		return "Spot";
	default:
		return "LightComp";
	}
}
