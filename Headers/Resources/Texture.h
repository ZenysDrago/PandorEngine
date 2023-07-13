#pragma once
#include "PandorAPI.h"

#include <string>
#include "IResources.h"
#include <Core/Wrappers/WrapperRHI.h>
namespace Render
{
	class Framebuffer;
}
namespace Resources
{
	class PANDOR_API Texture : public IResources
	{
	private:

		unsigned int m_format = -1;
		unsigned int m_slot = 0;
		unsigned int m_pixelType = -1;

		unsigned int m_filter = PR_LINEAR;

		unsigned int m_wrap = PR_REPEAT;

		unsigned char* bytes = nullptr;
		
		int widthImg = -1, heightImg = -1, numColCh = -1;
	public:
		friend class Render::Framebuffer;
		unsigned int ID = 0;
		unsigned int m_texType = -1;
		
		Texture(std::string _name, ResourcesType _type,
			unsigned int _texType = PR_TEXTURE2D, unsigned int _slot = 0, unsigned int _pixelType = PR_UNSIGNED_BYTE);

		~Texture() override;
		void Load() override;
		void SendResource() override;

		void SetSent() { hasBeenSent = true; }

		void TexUni(class Shader*& shader, const char* uniform, unsigned int unit);
		void Bind();
		void UnBind();
		void Delete();

		void Active(int index);

		void ShowInInspector() override;

		
		static ResourcesType GetResourceType() { return ResourcesType::Texture; }
	};
}
