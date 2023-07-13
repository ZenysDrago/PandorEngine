#pragma once
#include "PandorAPI.h"
#include <Resources/IResources.h>
#include <Core/GameObject.h>

namespace Utils::Loader
{
	class OBJ;
	class FBX;
}
namespace Core
{
	class Scene;
}

namespace Component
{
	class SkeletalMeshComponent;
}
namespace Resources
{
	class Bone : public Core::GameObject
	{
	public:
		Bone();
		~Bone();

		void ShowInInspector();
		void UpdateBone(class Animation*, float);
		void CrossUpdate(float CrossFadeDuration, float Time, class Animation* currentAnimation,  class Animation* nextAnimation);
		void SetDefault();

		std::vector<Bone*> GetAllBones();

		Math::Matrix4 GetBoneMatrix();

		void DrawDebug();

		Bone* Clone();

		GameObject* Instantiate(GameObject* parent) override;

		void RemoveFromParent();

		void ReadGameObject(std::fstream& sceneFile, Core::Scene* scene) override;

		std::ostream& operator<<(std::ostream& os) override;

		int Id = -1;
		Math::Vector3 DefaultPosition;
		Math::Quaternion DefaultRotation;
		Math::Matrix4 DefaultMatrix;

		class Skeleton* GetSkeleton() { return m_skeleton; }
	private:
		friend class Utils::Loader::FBX;
		friend class Skeleton;
		class Skeleton* m_skeleton;
	};

	class Skeleton : public IResources
	{
	public:
		Skeleton(std::string _name, ResourcesType _type) : IResources(_name, _type) {}
		~Skeleton();

		void Load() override {}
		void SendResource() override {}

		std::vector<Math::Matrix4> GetBonesMatrices();
		void SortBones();

		Bone* RootBone = nullptr;

		// sorted list by index
		std::vector<Bone*> Bones;

		size_t BoneCount = 0;

		size_t GetMaxBoneWeight() { return m_maxBoneWeight; }

		Resources::Skeleton* Clone() const;

		void RemoveMesh(Component::SkeletalMeshComponent* skelMesh);
		void AddMesh(Component::SkeletalMeshComponent* skelMesh);

		static ResourcesType GetResourceType() { return ResourcesType::Skeleton; };
	private:
		size_t m_maxBoneWeight = 0;
		std::vector<Component::SkeletalMeshComponent*> m_skeletalMeshes = {};
		

		friend Component::SkeletalMeshComponent;
		friend Bone;
		friend class Utils::Loader::FBX;
	};
}