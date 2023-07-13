#pragma once
#include "PandorAPI.h"

#include <Resources/IResources.h>
#include <Math/Maths.h>
namespace Component {
	class MeshComponent;
}

namespace Utils::Loader
{
	class MAT;
	class OBJ;
	class FBX;
}

namespace Resources
{
	class PANDOR_API Material : public IResources
	{
	public:
		Material(std::string _name, ResourcesType _type);
		~Material();

		void Load() override;
		void Save();
		void SendResource() override;

		static ResourcesType GetResourceType() { return ResourcesType::Material; };

		// Getters
		class Shader* GetShader() { return m_shader; }
		class Shader* GetOutlineShader() { return m_outlineShader; }
		class Shader* GetPickingShader() { return m_pickingShader; }
		class Texture* GetTexture() { return m_texture; }
		class Texture* GetNormalMap() { return m_normalMap; }
		class Texture* GetRoughnessMap() { return m_roughnessMap; }
		class Texture* GetMetallicMap() { return m_metallicMap; }
		Math::Vector4 GetAmbient() { return m_ambient; }
		Math::Vector4 GetDiffuse() { return m_diffuse; }
		Math::Vector4 GetSpecular() { return m_specular; }
		float GetShiniess() { return m_shininess; }
		class Texture* GetThumbnail();

		// Setters
		void SetName(std::string _name) { p_name = _name; }
		void SetShader(Resources::Shader* shader, bool createOthers = true);
		void SetTexture(Resources::Texture* texture) { m_texture = texture; }
		void SetNormalMap(Resources::Texture* texture) { m_normalMap = texture; }
		void SetRoughnessMap(Resources::Texture* texture) { m_roughnessMap = texture; }
		void SetMetallicMap(Resources::Texture* texture) { m_metallicMap = texture; }
		void SetAmbient(Math::Vector4 ambient) { m_ambient = ambient; }
		void SetDiffuse(Math::Vector4 diffuse) { m_diffuse = diffuse; }
		void SetSpecular(Math::Vector4 specular) { m_specular = specular; }
		void SetShiniess(float shininess) { m_shininess = shininess; }
		void SetTransparency(float transparency) {
			m_ambient.w = transparency;
			m_diffuse.w = transparency;
			m_specular.w = transparency;
		}

		std::ostream& operator<<(std::ostream& os) const;

		void ShowInInspector() override;

	private:
		friend Component::MeshComponent;
		friend Utils::Loader::MAT;
		friend Utils::Loader::OBJ;
		friend Utils::Loader::FBX;
		class Shader* m_shader = nullptr;
		class Shader* m_outlineShader = nullptr;
		class Shader* m_pickingShader = nullptr;
		class Texture* m_texture = nullptr;
		class Texture* m_normalMap = nullptr;
		class Texture* m_roughnessMap = nullptr;
		class Texture* m_metallicMap = nullptr;
		Math::Vector4 m_ambient = Math::Vector4(1);
		Math::Vector4 m_diffuse = Math::Vector4(1);
		Math::Vector4 m_specular = Math::Vector4(1);
		float m_shininess = 32.f;

		class Texture* m_thumbnail = nullptr;
	public:
		float roughness = 0.5f;
		float metallic = 0.0f;
	};
}