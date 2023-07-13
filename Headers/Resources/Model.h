#pragma once
#include "PandorAPI.h"

#include <Resources/IResources.h>
namespace Core
{
	class GameObject;
}
namespace Utils::Loader
{
	class OBJ;
	class FBX;
}

namespace Component
{
	class MeshComponent;
}

namespace Resources
{
	class PANDOR_API Model : public IResources
	{
	public:
		Model(std::string _name, ResourcesType _type) : IResources(_name, _type) {}
		~Model();
		Model& operator=(const Model&);

		void Load() override;
		void SendResource() override;

		Core::GameObject* ToGameObject(bool isRuntimeObject = true);

		void AddMaterial(class Material* mat) { m_materials.push_back(mat); }
		void AddMesh(class Mesh* mesh);
		std::vector<class Mesh*> GetMeshes() { return m_meshes; }
		std::vector<class Material*> GetMaterials() { return m_materials; }

		bool HasBeenSent() override;
		void SetupMeshComp(std::string path, Component::MeshComponent* meshComp);

		float GetMaxVerticeOnXYZ();

		std::vector<class Material*> GetMaterialsForMesh(Resources::Mesh* mesh);

		class Texture* GetThumbnail();

		void Display(bool value) override;

		static ResourcesType GetResourceType() { return ResourcesType::Model; };
	private:
		std::vector<class Mesh*> m_meshes;
		std::vector<class Skeleton*> m_skeletons;
		std::vector<class Material*> m_materials;
		Math::Vector3  m_maxXYZ = (0.f, 0.f, 0.f);
		class Texture* m_thumbnail;

		friend Utils::Loader::OBJ;
		friend Utils::Loader::FBX;
	};
}