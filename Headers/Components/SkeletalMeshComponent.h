#pragma once
#include "PandorAPI.h"
#include <Components/BaseComponent.h>
#include <Components/MeshComponent.h>
namespace Resources
{
	class SkeletalMesh;
	class Skeleton;
	class Bone;
}
namespace Component
{
	class PANDOR_API SkeletalMeshComponent : public MeshComponent
	{
	public:
		SkeletalMeshComponent();
		~SkeletalMeshComponent();

		std::string GetComponentName() override { return "Skeletal Mesh Component"; }
		
		void ShowInInspector() override;

		void EditorDraw() override;
		void Draw() override;
		void DrawPicking(int ID) override;

		void Update() override;

		void SetMesh(Resources::SkeletalMesh* mesh);
		void SetSkeleton(Resources::Skeleton* _skeleton, bool addSkeletonToScene = true);

		Resources::SkeletalMesh* GetMesh() { return m_skeletalMesh; }
		Resources::Skeleton* GetSkeleton() { return m_skeleton; }

		BaseComponent* Clone() const override { return new SkeletalMeshComponent(*this); };

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;

	private:
		friend class Animator;
		friend Resources::Bone;

		Resources::SkeletalMesh* m_skeletalMesh;
		Resources::Skeleton* m_skeleton;

		std::string m_meshToLoad;
		int m_boneIndex = -1;
	};
}