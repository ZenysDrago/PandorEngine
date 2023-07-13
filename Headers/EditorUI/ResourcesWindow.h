#pragma once
#include "PandorAPI.h"

#include "EditorWindow.h"

namespace EditorUI {
	class PANDOR_API ResourcesWindow : public EditorWindow
	{
	public:
		ResourcesWindow();
		~ResourcesWindow();

		void Draw() override;

	};
}