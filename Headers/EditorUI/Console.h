#pragma once
#include "PandorAPI.h"

#ifndef PANDOR_GAME
#include "EditorWindow.h"
#endif#include <vector>
#include <string>
#include <mutex>
#include <deque>

namespace Debug
{
	enum class LogType
	{
		L_INFO,
		L_WARNING,
		L_ERROR
	};
}
#ifndef PANDOR_GAME
namespace EditorUI
{
	struct PANDOR_API ConsoleText
	{
		std::string text;
		Debug::LogType type;
		ConsoleText(Debug::LogType c, std::string t)
		{
			type = c;
			text = t;
		}
	};

	class PANDOR_API Console : public EditorWindow
	{
	private:
		std::mutex consoleMutex;
		char m_inputField[64];
		std::deque<ConsoleText> m_consoleText;
		std::string m_checkBoxTexts[3];
		int m_numberOfInfo = 0;
		int m_numberOfWarn = 0;
		int m_numberOfErro = 0;
		bool m_showInfo = true;
		bool m_showWarning = true;
		bool m_showError = true;
		int m_maxLog = 200;
	private:
		int GetNumberOfLogOfType(Debug::LogType);
	public:
		Console();
		~Console();

		void Draw() override;
		void AddLine(Debug::LogType, std::string);
		void Clear();

	};
}
#endif