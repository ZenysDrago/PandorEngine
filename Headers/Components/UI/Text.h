
#pragma once
#include <Components/UI/UIElement.h>
namespace Resources
{
	class Font;
	class Shader;
}
namespace Component::UI
{
	class PANDOR_API Text : public UIElement
	{
	public:
		Text();
		~Text();

		void Initialize() override;

		void ShowInInspector() override;

		void GameDraw() override;

		Component::BaseComponent* Clone() const { return new Text(*this); };

		std::string GetComponentName() override { return "Text"; }

		std::string GetText() { return m_text; }
		void SetText(std::string val) { m_text = val; }

		Resources::Font* GetFont() { return m_font; }

		float GetScale() { return m_scale; }
		void SetScale(float val) { m_scale = val; }

		Math::Vector4 GetColor() const { return m_color; }
		void SetColor(const Math::Vector4& _color) { m_color = _color; }

		void ReadComponent(std::fstream& sceneFile) override;

		std::ostream& operator<<(std::ostream& os) override;

	private:
		Resources::Font* m_font;
		std::string m_text = "Test";
		float m_scale = 1.f;
		Math::Vector4 m_color = { 1, 1, 1, 1 };
		bool m_sizeToContent = true;
	};
}