#pragma once
#include "PandorAPI.h"

#include <Math/Maths.h>
namespace Resources
{
	class Shader;
	class Texture;
	class Mesh;
	class Material;
}

namespace Core::Wrapper::WrapperRHI
{
	class Buffer;
	class RenderBuffer;
}

namespace Render
{
	class Camera;
}

namespace Component
{
	class CameraComponent;
}

namespace Render
{
	class PANDOR_API Framebuffer
	{
	public:
		Framebuffer();
		~Framebuffer();

		void Initialize(const Math::Vector2& size);
		void PreUpdate();
		void Draw();

		void SaveFrameBufferToTexture(std::string name, const Vector2& size);

		unsigned int GetFrameBufferTexture();
		Resources::Texture* GetTexture();

		void SetShader(Resources::Shader* shader);
		Resources::Shader* GetShader();
		std::ostream& operator<<(std::ostream& os);
		static void ReadFrameBufferFromFile(Render::Framebuffer* framebuffer, std::fstream& sceneFile);

		bool* EnablePostProcessing() { return &m_enablePostProcessing; }
	private:
		std::string shaderName = "null" ; // Variables only use to save the shader to use from scene saving
		friend Render::Camera;
		friend Component::CameraComponent;
		Render::Framebuffer* m_postProcess;
		Resources::Material* m_material;
		Resources::Mesh* m_plane = nullptr;
		Core::Wrapper::WrapperRHI::RenderBuffer* m_renderBuffer;
		Core::Wrapper::WrapperRHI::Buffer* m_buffer;

		Math::Vector2 m_size;

		size_t m_index;

		bool m_enablePostProcessing = false;

	};
}