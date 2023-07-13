#pragma once
#include "PandorAPI.h"

namespace EditorUI
{
	class PANDOR_API EditorWindow
	{
	protected:
		bool p_open = true;
	public:
		EditorWindow() {}
		~EditorWindow() {}

		virtual void Draw() {}

		virtual void SetOpen(bool value) { p_open = value; }
		virtual bool* IsOpen() { return &p_open; }
	};
}