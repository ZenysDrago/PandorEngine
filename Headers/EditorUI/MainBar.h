#pragma once
#include "PandorAPI.h"

namespace Core::Wrapper
{
	class WrapperWindow;
}
namespace EditorUI
{
	class PANDOR_API MainBar
	{
	private:
		Vector3 m_outlineColor = { 1.0, 1.0, 1.0 };
	public:
		MainBar();
		~MainBar();

		void Draw();

		Vector3 GetOutlineColor();
		void SetOutlineColor(Vector3 outline) { m_outlineColor = outline; }
	};
}
