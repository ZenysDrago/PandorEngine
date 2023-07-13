#pragma once
#include "PandorAPI.h"

#include "../Debug/Log.h"
#include <string>
#include <mutex>
#include <Core/Wrappers/WrapperRHI.h>

namespace EditorUI
{
	class FileExplorer;
}
namespace Resources
{
    enum class PANDOR_API ResourcesType
    {
        None,
        Texture,
        Shader,
        FragmentShader,
        VertexShader,
        ComputeShader,
        Material,
        PhysicMaterial,
        Model,
        Mesh,
        SkeletalMesh,
        Skeleton,
        Animation,
        AnimationController,
        Font,
        Skybox,
        Sound,
        Prefab,
    };


    class PANDOR_API IResources
    {
    protected:
        std::string p_name;
		std::string p_path;
		std::string p_fullPath;
        ResourcesType type = ResourcesType::None;

        std::atomic_bool p_shouldBeLoaded = false;
        std::atomic_bool isLoaded = false;
        std::atomic_bool hasBeenSent = false;

        bool m_displayOnResourcesManager = true;

    public:
        IResources(std::string _path, ResourcesType _type);
        virtual ~IResources();
        virtual void Load() = 0;
        virtual void SendResource() = 0;

        ResourcesType GetType() { return type; }
        void SetName(std::string _name) { p_name = _name; }
		std::string GetName()   { return p_name; }
		std::string GetPath() { return p_path; }
		std::string GetFullPath() { return p_fullPath; }
        bool ShouldBeLoaded() { return p_shouldBeLoaded.load(); }
        bool IsLoaded() { return isLoaded.load(); }
        virtual bool HasBeenSent() { return hasBeenSent.load(); }
        bool MustBeDisplay() { return m_displayOnResourcesManager; }
        virtual void Display(bool value) { m_displayOnResourcesManager = value; }

        virtual void ShowInInspector() {}

        friend class ResourcesManager;
	    friend EditorUI::FileExplorer;

    };
}