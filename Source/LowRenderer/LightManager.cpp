#include "pch.h"

#include <LowRenderer/LightManager.h>
#include <Components/LightComponent.h>
#include <Components/Transform.h>
#include <Resources/Shader.h>
#include <Resources/ResourcesManager.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/GameObject.h>
#include <Core/SceneManager.h>
#include <Render/Camera.h>

using namespace LowRenderer;

LightManager& LightManager::getInstance()
{
    static LightManager instance;
     return instance;
}

bool LowRenderer::LightManager::Create(Component::LightComponent* light)
{
    /*if (lightCount >= maxLights)
    {
        std::cout << ">> Max light count reached, could not add a new one <<" << std::endl;
        return false;
    }*/
    currentLights.push_back(light);
    lightCount++;
    return true;
}

void LightManager::Delete(Component::LightComponent* light)
{
    for (int i = 0; i < currentLights.size(); i++)
    {
        if (currentLights[i] == light)
        {
            currentLights.erase(currentLights.begin() + i);
        }
    }
}

void LowRenderer::LightManager::SendLightToAllShaders()
{
    for (auto&& shader : Resources::ResourcesManager::Get()->GetAllResourcesOfType<Resources::Shader>()) {
        if (!shader || !shader->HasBeenSent() || shader->GetLocation("lightEnable") != -1)
            continue;
        SetLightsUniforms(shader);
        Core::Wrapper::WrapperRHI::ShaderSendVec3(shader->GetLocation("viewPos"), Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetTransform()->GetWorldPosition());
    }
}

void LowRenderer::LightManager::SetLightsUniforms(Resources::Shader* shaderProgram)
{
    if (!shaderProgram)
        return;
    using namespace Core::Wrapper::WrapperRHI;
    using namespace Component;
    shaderProgram->Use();
    int sizeDirLights = 0, sizePointLights = 0, sizeSpotLights = 0;

    for (int i = 0; i < currentLights.size(); i++)
    {
        if (!currentLights[i]->IsEnable())
            continue;
            
        if (currentLights[i]->GetType() == LightType::DIRECTIONAL)
        {
            currentLights[i]->SetUniforms(shaderProgram, sizeDirLights);
            sizeDirLights++;
        }
        else if (currentLights[i]->GetType() == LightType::POINT)
        {
            currentLights[i]->SetUniforms(shaderProgram, sizePointLights);
            sizePointLights++;
        }
        else
        {
            currentLights[i]->SetUniforms(shaderProgram, sizeSpotLights);
            sizeSpotLights++;
        }
            
    }

	ShaderSendInt(shaderProgram->GetLocation("sizeDirLights"), sizeDirLights);
    ShaderSendInt(shaderProgram->GetLocation("sizePointLights"), sizePointLights);
    ShaderSendInt(shaderProgram->GetLocation("sizeSpotLights"), sizeSpotLights);
}

std::vector<Component::LightComponent*> LowRenderer::LightManager::GetAllLight()
{
    return currentLights;
}

