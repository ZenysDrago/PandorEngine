#pragma once
#include "PandorAPI.h"

#include <Resources/IResources.h>
#include <Math/Maths.h>
#include <Core/Wrappers/WrapperRHI.h>
#include <Resources/ResourcesManager.h>

//Layouts:
// 0 pos
// 1 tex
// 2 norm
// 3 tangent
// 4 boneIndices
// 5 boneWeights
// 8 

namespace Utils
{
	struct AABB;
	namespace Loader {
		class OBJ;
		class FBX;
	}
}
namespace Component
{
	class ParticleSystem;
	class Transform;
	class DirectionalLight;
}
namespace Core::Wrapper::WrapperPhysic
{
	class PhysicManager;
}
namespace Resources
{
	struct SubMesh
	{
		size_t StartIndex;
		size_t Count;
	};

	class PANDOR_API Mesh : public IResources
	{
	public:

		Mesh(std::string _name, ResourcesType _type) : IResources(_name, _type) {}
		~Mesh();

		void Load() override;
		virtual void SendResource() override;
		virtual void PickingResource(const Math::Matrix4& MVP, std::vector<class Material*> materials, int ID);

		Matrix4 ShadowVP();

		void Render(const Math::Matrix4& MVP, const Math::Matrix4& model, const std::vector<class Material*>& materials, bool wireframe = false, bool cullface = true, bool drawOutline = false, bool drawShadow = false);

		void RenderInstancing(class Material* material, class Shader* shader, size_t count, WrapperRHI::Buffer* buffer = nullptr);
		void RenderInstancingPicking(class Shader* shader, size_t count, int ID);

		void RenderUI(const Math::Vector2& Position, const Math::Vector2& Size, Resources::Material* material, float depth);

		static ResourcesType GetResourceType() { return ResourcesType::Mesh; };

		std::vector<SubMesh> GetSubMeshes() { return m_subMeshes; }
		class Model* GetModel() { return m_fromModel; }

		void BindBuffer();
		void UnBindBuffer();

		Texture* GetThumbnail();

		std::vector<Vector3> GetPositionVertices() { return m_positions; }
		Math::Vector3 MinAABB() const { return m_minAABB; }
		Math::Vector3 MaxAABB() const { return m_maxAABB; }
		Utils::AABB* GetBoudingBox() const { return m_boudingBox; }

		bool IsVisible(Render::Camera* camera, Component::Transform* transform);
	protected:
		class Model* m_fromModel;
		std::vector<float> m_vertices;
		std::vector<float> m_defaultVertices;
		std::vector<Vector3> m_positions;
		std::vector<uint32_t> m_indicesPositions;
		std::vector<uint32_t> m_indices;
		std::vector<SubMesh> m_subMeshes;
		class Texture* m_thumbnail;

		Math::Vector3 m_minAABB;
		Math::Vector3 m_maxAABB;
		Math::Vector3 m_translation;
		Math::Quaternion m_rotation;

		Utils::AABB* m_boudingBox;

		Core::Wrapper::WrapperRHI::Buffer* m_buffer;

		friend class Model;
		friend Utils::Loader::OBJ;
		friend Utils::Loader::FBX;
		friend Component::ParticleSystem;
		friend Core::Wrapper::WrapperPhysic::PhysicManager;
	};
}
