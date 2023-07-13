#pragma once
#include "PandorAPI.h"
#include <string>
#include <unordered_map>
namespace Resources
{
	struct Character;
	class Font;
}
namespace Core::Wrapper
{
	class PANDOR_API WrapperFont
	{
	public:
		WrapperFont();
		~WrapperFont();

		static void Initialize();
		static void LoadFont(Resources::Font* font);
		static void Delete();

	private:
		static void* ft;

	};
}