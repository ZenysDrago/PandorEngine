#pragma once
#include <PandorAPI.h>
#include <Math/Maths.h>
#include <Resources/IResources.h>
#include <Resources/Mesh.h>

namespace Resources
{
	class SkeletalMesh : public Mesh
	{
	public:
		SkeletalMesh(std::string _name, ResourcesType _type) : Mesh(_name, _type) {}
		~SkeletalMesh();

		void SendResource() override;
		void Render(const Math::Matrix4& model, const std::vector<Resources::Material*>& materials, class Skeleton* skel, bool outline);
		void PickingResource(const Math::Matrix4& model, std::vector<class Material*> materials, class Skeleton* skel, int ID);

		static ResourcesType GetResourceType() { return ResourcesType::SkeletalMesh; };
	private:

	};

}