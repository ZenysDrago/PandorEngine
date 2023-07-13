#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>
namespace Resources
{
	class Mesh;
	class Material;
}

namespace Utils
{
	struct Sphere;
	struct AABB;
}

namespace Component
{

	class PANDOR_API MeshComponent : public IComponent<MeshComponent>
	{
	private:
		Resources::Mesh* m_mesh = nullptr;
	protected:
		std::vector<Resources::Material*> m_materials;
		std::string m_MeshPath;
		std::string m_ModelPath;

	public:
		MeshComponent();
		~MeshComponent();

		void Update() override;
		void Draw() override;
		void GameDraw() override;
		void EditorDraw() override;
		void DrawShadow();
		void DrawPicking(int ID) override;

		Resources::Mesh* GetMesh() { return m_mesh; }
		void SetMesh(Resources::Mesh* mesh);

		void AddMaterial(Resources::Material* material);

		void ShowInInspector() override;

		std::string GetComponentName() override { return "Mesh Component"; }
		std::vector<Resources::Material*>& GetMaterials() { return m_materials; }

		std::ostream& operator<<(std::ostream& os) override;

		void SetMeshToLoad(const std::string& path);

		void ReadComponent(std::fstream& sceneFile) override;

		void OnTriggerEnter(class Collider* collider) override;
		void OnTriggerStay(class Collider* collider) override;
		void OnTriggerExit(class Collider* collider) override;

		void OnCollisionEnter(class Collider* collider) override;
		void OnCollisionStay(class Collider* collider) override;
		void OnCollisionExit(class Collider* collider) override;
	};
}
