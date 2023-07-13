#pragma once
#include "PandorAPI.h"

#include "../BaseComponent.h"

namespace Component::UI
{
	class PANDOR_API Canvas : public Component::IComponent<Canvas>
	{
		std::string GetComponentName() { return "Canvas"; }

		void ShowInInspector() override;
	};
}

