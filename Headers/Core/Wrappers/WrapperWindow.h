#pragma once
#include "PandorAPI.h"
#include <windows.h>


#include <Math/Maths.h>

typedef void(*glproc);
struct GLFWwindow;
struct GLFWvidmode;
namespace Core {
	struct AppInit;
	namespace Wrapper
	{
		class PANDOR_API WrapperWindow
		{
		private:
			static GLFWwindow* m_window;

			static WrapperWindow* m_instance;

			int m_width;
			int m_height;

			bool m_shouldBeFullscreen = false;

			const GLFWvidmode* m_videoMode = nullptr;

			//Callback function called when a GLFW window resizing event occurs
			static void FramebufferSizeCallback(GLFWwindow* window, int width, int height); 
			
			static void DropCallback(GLFWwindow* window, int count, const char** paths);

			Vector2 prevSize;
		public:
			GLFWwindow* GetWindow() { return m_window; }

			HWND GetWin32Window();

			static WrapperWindow& Get() { return *m_instance; }

			static WrapperWindow* CreateInstance() {	return m_instance = new Core::Wrapper::WrapperWindow();}

			void SetCloseCallback(void (*func)(GLFWwindow* window));

			void Initialize(const AppInit& init);

			void Initialize();

			void SetResizable(bool value);

			void SetFullscreen(bool value);

			void ToggleFullscreen();

			Math::Vector2 GetSize();

			bool ShouldClose();

			void CloseWindow(bool shouldClose = true);

			void Terminate();

			static int GetKey(int key);

			void SwapBuffers();

			void PollEvents();

			static double GetTime();

			static Math::Vector2 GetWindowPos();

			static glproc GetProcAddress(const char* procname);

			Math::Vector2 GetMonitorSize();
			bool ShouldBeFullscreen() const { return m_shouldBeFullscreen; }
		};
	}
}