#include "pch.h"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <STB_Image/stb_image.h>

#include <Core/App.h>
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/FileExplorer.h>
#endif // !PANDOR_GAME
#include <Core/Wrappers/WrapperRHI.h>

GLFWwindow* Core::Wrapper::WrapperWindow::m_window = nullptr;
Core::Wrapper::WrapperWindow* Core::Wrapper::WrapperWindow::m_instance = nullptr;
HWND Core::Wrapper::WrapperWindow::GetWin32Window()
{
	return glfwGetWin32Window(m_window);
}

void Core::Wrapper::WrapperWindow::Initialize(const AppInit& init)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, init.major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, init.minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, init.resizable);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif // DEBUG


	m_window = glfwCreateWindow(init.width, init.height, init.name, NULL, NULL);
	if (m_window == NULL) {
		PrintError("Failed to create GLFW window");
		glfwTerminate();
		return;
	}
	m_width = init.width;
	m_height = init.height;
	glfwMakeContextCurrent(m_window);
	glfwSetFramebufferSizeCallback(m_window, &FramebufferSizeCallback);
	glfwSetDropCallback(m_window, &DropCallback);
	glfwSwapInterval(0);

	GLFWimage images[1];
	images[0].pixels = stbi_load((ENGINEPATH"Icon/LogoCouleur.png").c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(m_window, 1, images);
	stbi_image_free(images->pixels);
}

void Core::Wrapper::WrapperWindow::SetResizable(bool value)
{
	glfwWindowHint(GLFW_RESIZABLE, value);
}

void Core::Wrapper::WrapperWindow::SetFullscreen(bool value)
{
	if (value) {
		prevSize = GetSize();
		GLFWmonitor* primary = glfwGetPrimaryMonitor();

		const GLFWvidmode* mode = glfwGetVideoMode(primary);

		glfwSetWindowMonitor(m_window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else
	{
		GLFWmonitor* primary = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(primary);
		glfwSetWindowMonitor(m_window, NULL, 0, 0, (int)prevSize.x, (int)prevSize.y, 0);
		glfwSetWindowPos(m_window, 100, 100);
	}
}

void Core::Wrapper::WrapperWindow::ToggleFullscreen()
{
	m_shouldBeFullscreen = !m_shouldBeFullscreen;
	SetFullscreen(m_shouldBeFullscreen);
}

Math::Vector2 Core::Wrapper::WrapperWindow::GetSize()
{
	return { (float)m_width, (float)m_height };
}

void Core::Wrapper::WrapperWindow::SetCloseCallback(void (*func)(GLFWwindow* window))
{
	glfwSetWindowCloseCallback(m_window, func);
}

bool Core::Wrapper::WrapperWindow::ShouldClose()
{
	return glfwWindowShouldClose(m_window);
}

void Core::Wrapper::WrapperWindow::CloseWindow(bool shouldClose)
{
	glfwSetWindowShouldClose(m_window, shouldClose);
}

void Core::Wrapper::WrapperWindow::Terminate()
{
	glfwTerminate();
}

int Core::Wrapper::WrapperWindow::GetKey(int key)
{
	return glfwGetKey(m_window, key);
}

void Core::Wrapper::WrapperWindow::SwapBuffers()
{
	glfwSwapBuffers(m_window);
}

void Core::Wrapper::WrapperWindow::PollEvents()
{
	glfwPollEvents();
}

double Core::Wrapper::WrapperWindow::GetTime()
{
	return glfwGetTime();
}

Math::Vector2 Core::Wrapper::WrapperWindow::GetWindowPos()
{
	int x, y;
	glfwGetWindowPos(m_window, &x, &y);
	Vector2 pos{ (float)x, (float)y };

	return pos;
}

glproc Core::Wrapper::WrapperWindow::GetProcAddress(const char* procname)
{
	return glfwGetProcAddress(procname);
}

Math::Vector2 Core::Wrapper::WrapperWindow::GetMonitorSize()
{
	if (!m_videoMode)
		m_videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	return { (float)m_videoMode->width, (float)m_videoMode->height };
}

void Core::Wrapper::WrapperWindow::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	if (m_instance) {
		m_instance->m_width = width;
		m_instance->m_height = height;
	}
	if (Resources::ResourcesManager::Get())
		WrapperRHI::ViewPort(0, 0,width, height);
}

void Core::Wrapper::WrapperWindow::DropCallback(GLFWwindow* window, int count, const char** paths)
{
#ifndef PANDOR_GAME
	Core::App::Get().GetEditorUIManager().GetFileExplorer().Drop(paths, count);
#endif // !PANDOR_GAME
}
