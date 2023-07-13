#include "pch.h"
#include <Core/Wrappers/WrapperFont.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <Resources/Font.h>

void* Core::Wrapper::WrapperFont::ft = nullptr;

Core::Wrapper::WrapperFont::WrapperFont()
{
}

Core::Wrapper::WrapperFont::~WrapperFont()
{
}

void Core::Wrapper::WrapperFont::Initialize()
{
	ft = new FT_Library();
	// All functions return a value different than 0 whenever an error occurred
	if (FT_Init_FreeType((FT_Library*)ft))
	{
		PrintError("ERROR::FREETYPE: Could not init FreeType Library");
		return;
	}
}

void Core::Wrapper::WrapperFont::LoadFont(Resources::Font* font)
{
	// load font as face
	FT_Face face;
	if (FT_New_Face(*(FT_Library*)ft, font->GetFullPath().c_str(), 0, &face)) {
		PrintError("ERROR::FREETYPE: Failed to load font");
		return;
	}
	else {
		// set size to load glyphs as
		FT_Set_Pixel_Sizes(face, 0, font->GetFontSize());

		// disable byte-alignment restriction
		WrapperRHI::PixelStore(PR_UNPACK_ALIGNMENT, 1);

		// load first 128 characters of ASCII set
		for (unsigned char c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				PrintError("ERROR::FREETYTPE: Failed to load Glyph");
				continue;
			}
			// generate texture
			unsigned int texture;
			WrapperRHI::GenerateTexture(texture);
			WrapperRHI::TextureBind(texture);
			WrapperRHI::FontTexture(face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.buffer);
			// now store character for later use
			if (font->maxHeight < (float)face->glyph->bitmap.rows)
			{
				font->maxHeight = (float)face->glyph->bitmap.rows;
			}
			Resources::Character character = {
				texture,
				Math::Vector2((float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows),
				Math::Vector2((float)face->glyph->bitmap_left, (float)face->glyph->bitmap_top),
				static_cast<unsigned int>(face->glyph->advance.x)
			};
			font->GetCharacters().insert(std::pair<char, Resources::Character>(c, character));
		}
		WrapperRHI::TextureUnBind();
	}
	PrintLog("%s Font Loaded !", font->GetFullPath().c_str());
	FT_Done_Face(face);
}

void Core::Wrapper::WrapperFont::Delete()
{
	FT_Done_FreeType(*(FT_Library*)ft);
	delete ft;
	ft = nullptr;
}
