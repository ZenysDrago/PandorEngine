#pragma once
#include <Resources/IResources.h>

#include <unordered_map>

namespace Resources
{
	struct Character {
		unsigned int  TextureID; // ID handle of the glyph texture
		Math::Vector2 Size;      // Size of glyph
		Math::Vector2 Bearing;   // Offset from baseline to left/top of glyph
		unsigned int  Advance;   // Horizontal offset to advance to next glyph
	};

	class PANDOR_API Font : public IResources
	{
	public:
		Font(std::string _name, ResourcesType _type) : IResources(_name, _type) {}
		~Font();

		void Load() override;
		void SendResource() override;

		uint32_t GetFontSize() { return fontSize; }

		static ResourcesType GetResourceType() { return ResourcesType::Font; }

		std::unordered_map<char, Character>& GetCharacters() { return m_characters; }

		float maxHeight = 0.f;
	private:
		std::unordered_map<char, Character> m_characters;
		uint32_t fontSize = 128;

	};
}