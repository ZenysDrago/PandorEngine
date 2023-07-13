#pragma once
#include "PandorAPI.h"

#include <vector>

namespace Component {
	class PANDOR_API ComponentsData
	{
	public:
		ComponentsData();
		~ComponentsData();
		void Destroy();
		std::vector<class BaseComponent*> Components;
		void NewComponent(class BaseComponent* comp);
		void Initialize();

		static void Delete();
		static void Create() { m_componentsDatas = new ComponentsData(); }
		static ComponentsData& Get() { return *m_componentsDatas; }
	private:
		static ComponentsData* m_componentsDatas;
	};
}