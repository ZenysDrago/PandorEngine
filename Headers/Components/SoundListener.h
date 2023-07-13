#pragma once

#include <Components/BaseComponent.h>
#include <Core/Wrappers/WrapperAudio.h>

namespace Component
{
    class Rigidbody;

    
    class SoundListener : public IComponent<SoundListener> 
    {
    private:
        Component::Rigidbody* m_rbody = nullptr;
        Core::Wrapper::WrapperAudio::Listener* m_listener = nullptr;

    public :
        
    private:
    public :
        SoundListener();
        void Start() override;
        void Update() override;
        void EditorUpdate() override;
        void ShowInInspector() override;
        
        std::string GetComponentName() override { return "SoundListener"; }
        BaseComponent* Clone() const override;

		void ReadComponent(std::fstream& sceneFile) override;

		std::ostream& operator<<(std::ostream& os) override;
    };
}
