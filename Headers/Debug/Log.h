#pragma once
#include "PandorAPI.h"
#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include <string>
#include <chrono>
#include <time.h>

#include <EditorUI/Console.h>

#define MAX_LOG_SIZE 1024

namespace Debug {

	class PANDOR_API Log
	{
	public:
		Log();
		~Log();
		template <typename ...Args> static void Print(const char* file, int line, LogType type, const char* format, Args ... args)
		{
#pragma warning(push)
#pragma warning(disable:4996)
			const std::time_t now = std::time(nullptr); // get the current time point
			const std::tm calendar_time = *std::localtime(std::addressof(now));
#pragma warning(pop)  

			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			char buf[MAX_LOG_SIZE];
			char buf2[MAX_LOG_SIZE];
			sprintf_s(buf2, format, args ...);
			sprintf_s(buf, "[%02d:%02d:%02d] %s (l:%d): %s", calendar_time.tm_hour, calendar_time.tm_min, calendar_time.tm_sec, file, line, buf2);
			switch (type)
			{
			case Debug::LogType::L_INFO:
				SetConsoleTextAttribute(hConsole, 15);
				break;
			case Debug::LogType::L_WARNING:
				SetConsoleTextAttribute(hConsole, 14);
				break;
			case Debug::LogType::L_ERROR:
				SetConsoleTextAttribute(hConsole, 4);
				break;
			default:
				break;
			}
#ifndef PANDOR_GAME
			m_console.AddLine(type, buf);
#endif
			printf(((std::string)buf + "\n").c_str());
			SetConsoleTextAttribute(hConsole, 15);
		}

#ifndef PANDOR_GAME
		static EditorUI::Console* GetConsole() { return &m_console; }
	private:
		static EditorUI::Console m_console;
#endif
	};


#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define LOG(t, x, ...) Debug::Log::Print(__FILENAME__, __LINE__, t, x, __VA_ARGS__);
#define PrintLog(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_INFO, x, __VA_ARGS__);
#define PrintWarning(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_WARNING, x, __VA_ARGS__);
#define PrintError(x, ...) Debug::Log::Print(__FILENAME__, __LINE__, Debug::LogType::L_ERROR, x, __VA_ARGS__);
#define Assert(condition) if(condition){ abort(); }
	}