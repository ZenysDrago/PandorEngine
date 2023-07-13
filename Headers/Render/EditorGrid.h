#pragma once
#include "PandorAPI.h"
#include <Core/Wrappers/WrapperRHI.h>
namespace Resources
{
	class Mesh;
	class Material;
}
namespace Render
{
	class PANDOR_API EditorGrid
	{
	public:
		EditorGrid();
		~EditorGrid();
		void Initialize();
		void Draw();

	private:
		WrapperRHI::Buffer* m_buffer;
		Resources::Material* m_material;
	};
}