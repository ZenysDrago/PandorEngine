#pragma once
#include "PandorAPI.h"
namespace Component
{
	class Transform;
}
namespace Resources
{
	class Mesh;
	class Texture;
	class Material;
}
namespace Render
{
	class PANDOR_API EditorIcon
	{
	public:
		EditorIcon();
		~EditorIcon();

		void Initialize();

		void SetIcon(Resources::Texture* tex);

		void Draw(const Math::Vector3& position, bool outline);

		void DrawPicking(const Math::Vector3& position, int ID);

	private:
		Resources::Mesh* m_plane = nullptr;
		Resources::Material* m_material = nullptr;

		Math::Matrix4 GetMVP(const Math::Matrix4& VP, const Math::Vector3& position);
	};
}