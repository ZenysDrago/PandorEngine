#pragma once
#include "PandorAPI.h"

#include "UIElement.h"
#include <Math/Maths.h>
namespace Resources
{
	class Texture;
}

namespace Component::UI
{
	class PANDOR_API UIImage : public UIElement
	{
	public:
		~UIImage();

		void Initialize() override;

		void Update() override;

		void ShowInInspector() override;

		std::string GetComponentName() { return "Image"; }

		Component::BaseComponent* Clone() const { return new UIImage(*this); };
		Resources::Texture* GetImage() const { return m_texture; }
		void SetImage(Resources::Texture* tex);

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;
	private:
		Resources::Texture* m_texture;
	};
}