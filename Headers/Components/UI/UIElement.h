#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>
#include <Math/Maths.h>
namespace Core::Wrapper::WrapperRHI {
	class Buffer;
}
namespace Resources
{
	class Material;
}
namespace Component::UI
{
	class PANDOR_API UIElement : public IComponent<UIElement>
	{
	public:
		~UIElement();
		void Initialize() override;
		std::string GetComponentName() { return "UIElement"; }
		virtual void Update() override;
		virtual void GameDraw() override;
		class RectTransform* GetRectTransform();
	protected:
		WrapperRHI::Buffer* m_buffer;
		Resources::Material* m_material;
		class RectTransform* m_rectTransform;
	};
}

