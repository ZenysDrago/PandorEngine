#pragma once
#include "PandorAPI.h"

#include <vector>
#include <stdexcept>

namespace Component
{
    class LightComponent;
}

namespace Resources
{
    class Shader;
}

namespace LowRenderer
{
    class PANDOR_API LightManager
    {

    private:
        unsigned int lightCount = 0;
        std::vector<Component::LightComponent*> currentLights;

    public:
        const unsigned int maxLights = 3;

    private:
        LightManager() {} // private constructor to prevent external instantiation

    public:
        static LightManager& getInstance();

        bool Create(Component::LightComponent* light);

        void Delete(Component::LightComponent* light);

        void SendLightToAllShaders();
      
        void SetLightsUniforms(Resources::Shader* shaderProgram);

        LightManager(LightManager const&) = delete;
        void operator=(LightManager const&) = delete;

        int GetLightCount() const { return lightCount;  }

        std::vector<Component::LightComponent*> GetAllLight();
    };
}