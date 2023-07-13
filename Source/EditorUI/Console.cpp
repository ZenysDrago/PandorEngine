#include "pch.h"

#include <EditorUI\Console.h>

#include <Core/Wrappers/WrapperUI.h>

#include <Utils/Utils.h>

#include <Core/App.h>

using namespace Core::Wrapper;
EditorUI::Console::Console()
{

}

EditorUI::Console::~Console()
{
}

void EditorUI::Console::Draw()
{
	if (!p_open)
		return;
	if (WrapperUI::Begin("Console", &p_open))
	{
		WrapperUI::PushID(0);
		// ----------- LogType Buttons ----------- //
		WrapperUI::Checkbox(m_checkBoxTexts[0].c_str(), &m_showInfo);
		WrapperUI::SameLine();

		WrapperUI::PushStyleColor((Col)0, Math::Vector4(1, 0.5, 0, 1));
		WrapperUI::Checkbox(m_checkBoxTexts[1].c_str(), &m_showWarning);
		WrapperUI::PopStyleColor();
		WrapperUI::SameLine();

		WrapperUI::PushStyleColor((Col)0, Math::Vector4(1, 0, 0, 1));
		WrapperUI::Checkbox(m_checkBoxTexts[2].c_str(), &m_showError);
		WrapperUI::PopStyleColor();
		WrapperUI::SameLine();
		WrapperUI::PopID();

		if (WrapperUI::Button("Clear"))
		{
			Clear();
		}
		WrapperUI::Separator();

		const float footer_height_to_reserve = WrapperUI::GetItemSpacing().y + WrapperUI::GetFrameHeightWithSpacing();
		if (WrapperUI::BeginChild("ScrollingRegion", Math::Vector2(0, -footer_height_to_reserve), false, WindowFlags::HorizontalScrollbar)) {
			for (auto t : m_consoleText)
			{
				switch (t.type)
				{
				case Debug::LogType::L_INFO:
					if (m_showInfo)
						WrapperUI::TextUnformatted(t.text.c_str());
					break;
				case Debug::LogType::L_WARNING:
					if (m_showWarning)
						WrapperUI::TextColored(Math::Vector4(255, 128, 0, 255), t.text.c_str());
					break;
				case Debug::LogType::L_ERROR:
					if (m_showError)
						WrapperUI::TextColored(Math::Vector4(255, 0, 0, 255), t.text.c_str());
					break;
				default:
					break;
				}
			}
			if (WrapperUI::GetScrollY() >= WrapperUI::GetScrollMaxY())
				WrapperUI::SetScrollHereY(1.0f);
		}
		WrapperUI::EndChild();
		WrapperUI::Separator();
		WrapperUI::InputText("Input", m_inputField, 64, InputTextFlags::EnterReturnsTrue);
	}
	WrapperUI::End();
}

void EditorUI::Console::AddLine(Debug::LogType t, std::string s)
{
	if (consoleMutex.try_lock())
	{
		ConsoleText Text(t, s);

		switch (t)
		{
		case Debug::LogType::L_INFO:
			m_numberOfInfo++;
			break;
		case Debug::LogType::L_WARNING:
			m_numberOfWarn++;
			break;
		case Debug::LogType::L_ERROR:
			m_numberOfErro++;
			break;
		}
		if (m_consoleText.size() >= m_maxLog)
		{
			auto it = m_consoleText.begin() + (m_consoleText.size() + 1 - m_maxLog);
			std::vector<ConsoleText> destroyedText(m_consoleText.begin(), it);
			for (auto& text : destroyedText)
			{
				switch (text.type)
				{
				case Debug::LogType::L_INFO:
					--m_numberOfInfo;
					break;
				case Debug::LogType::L_WARNING:
					--m_numberOfWarn;
					break;
				case Debug::LogType::L_ERROR:
					--m_numberOfErro;
					break;
				default:
					--m_numberOfInfo;
					break;
				}
			}
			m_consoleText.erase(m_consoleText.begin(), it);
		}
		m_checkBoxTexts[0] = Utils::StringFormat("%d Info", GetNumberOfLogOfType(Debug::LogType::L_INFO));
		m_checkBoxTexts[1] = Utils::StringFormat("%d Warning", GetNumberOfLogOfType(Debug::LogType::L_WARNING));
		m_checkBoxTexts[2] = Utils::StringFormat("%d Error", GetNumberOfLogOfType(Debug::LogType::L_ERROR));

		m_consoleText.push_back(Text);
		consoleMutex.unlock();
	}
}

void EditorUI::Console::Clear()
{
	m_consoleText.clear();
	m_numberOfInfo = 0;
	m_numberOfWarn = 0;
	m_numberOfErro = 0;
	m_checkBoxTexts[0] = Utils::StringFormat("%d Info", GetNumberOfLogOfType(Debug::LogType::L_INFO));
	m_checkBoxTexts[1] = Utils::StringFormat("%d Warning", GetNumberOfLogOfType(Debug::LogType::L_WARNING));
	m_checkBoxTexts[2] = Utils::StringFormat("%d Error", GetNumberOfLogOfType(Debug::LogType::L_ERROR));
}

int EditorUI::Console::GetNumberOfLogOfType(Debug::LogType t)
{
	switch (t)
	{
	case Debug::LogType::L_INFO:
		return m_numberOfInfo;
	case Debug::LogType::L_WARNING:
		return m_numberOfWarn;
	case Debug::LogType::L_ERROR:
		return m_numberOfErro;
	default:
		return m_numberOfInfo;
	}
}

