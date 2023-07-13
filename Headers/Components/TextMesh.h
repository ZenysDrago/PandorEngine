#pragma once
#include <Components/BaseComponent.h>
namespace Resources {
	class Shader;
	class Material;
	class Font;
}
namespace Core::Wrapper::WrapperRHI
{
	class Buffer;
}
namespace Component
{
	class PANDOR_API TextMesh : public IComponent<TextMesh>
	{
	public:
		TextMesh();
		~TextMesh();

		void Initialize() override;

		void Draw() override;

		void DrawPicking(int ID) override;

		std::string GetComponentName() override { return "Text Mesh"; }

		void ShowInInspector() override;

		void ReadComponent(std::fstream& sceneFile) override;

		std::ostream& operator<<(std::ostream& os) override;

	private:
		Core::Wrapper::WrapperRHI::Buffer* m_buffer = nullptr;
		Resources::Material* m_material = nullptr;
		std::string m_text = "text";
		Resources::Font* m_font;
	};
}