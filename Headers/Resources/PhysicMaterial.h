#pragma once
#include "PandorAPI.h"

#include <Resources/IResources.h>

namespace Resources {
	class PhysicMaterial : public IResources
	{
	public:
		PhysicMaterial(std::string _path, ResourcesType _type) : IResources(_path,  _type) {}
		~PhysicMaterial() {}

		void Load() override;
		void SendResource() override;
		void Save();

		void ShowInInspector() override;

		void CreateMaterial();

		static ResourcesType GetResourceType() { return ResourcesType::PhysicMaterial; };
	private:
		float m_staticFriction = 0.5f;
		float m_dynamicFriction = 0.5f;
		float m_restitution = 0.5f;
	};
}