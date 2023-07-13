#pragma once
#include "PandorAPI.h"

#include <vector>
#include <EditorUI/EditorWindow.h>

namespace EditorUI
{
	class PANDOR_API PerformanceWindow : public EditorWindow
	{
	private:
	public:
		PerformanceWindow();
		~PerformanceWindow();

		void Draw() override;
	};
}