#include "pch.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <Resources/ResourcesManager.h>
#include <Resources/AnimationController.h>
#include <Core/GameObject.h>
#include <Core/App.h>
#include <EditorUI/EditorUIManager.h>

#include <GLFW/glfw3.h>

#include <Resources/Font.h>
#include <Components/UI/RectTransform.h>
#include <Components/UI/UIImage.h>
#include <Components/UI/Button.h>
#include <Components/UI/Text.h>

void Core::Wrapper::WrapperUI::Initialize(GLFWwindow* GlfwWindow)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer back ends
	ImGui_ImplGlfw_InitForOpenGL(GlfwWindow, true);
	ImGui_ImplOpenGL3_Init("#version 130");

	defaultFont = io.Fonts->AddFontDefault();

	SetupTheme(0);
}

Vector3 rgb(float ratio, float range, float offset = 0.f)
{
	//we want to normalize ratio so that it fits in to 6 regions
	//where each region is 256 units long
	float i = fmodf(ratio + offset * range, range) / range;
	int normalized = int(i * 256 * 6);

	//find the region for this position
	int region = normalized / 256;

	//find the distance to the start of the closest region
	int x = normalized % 256;

	uint8_t r = 0, g = 0, b = 0;
	switch (region)
	{
	case 0: r = 255; g = 0;   b = 0;   g += x; break;
	case 1: r = 255; g = 255; b = 0;   r -= x; break;
	case 2: r = 0;   g = 255; b = 0;   b += x; break;
	case 3: r = 0;   g = 255; b = 255; g -= x; break;
	case 4: r = 0;   g = 0;   b = 255; r += x; break;
	case 5: r = 255; g = 0;   b = 255; b -= x; break;
	}
	return Vector3(r, g, b);
}

void Core::Wrapper::WrapperUI::SetupTheme(int themeID)
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	switch (themeID)
	{
	case 0:
		ImGui::StyleColorsDark();
		break;
	case 1:
		ImGui::StyleColorsLight();
		break;
	case 2:
		ImGui::StyleColorsClassic();
		break;
	case 3:
		colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.24f, 0.29f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.04f, 0.13f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.24f, 0.19f, 0.28f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.12f, 0.14f, 0.67f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.09f, 0.10f, 0.87f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.21f, 0.10f, 0.28f, 0.87f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.08f, 0.29f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.52f, 0.52f, 0.54f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.30f, 0.30f, 0.31f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.52f, 0.52f, 0.54f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.20f, 0.12f, 0.32f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.55f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.52f, 0.31f, 0.78f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.29f, 0.17f, 0.44f, 0.64f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.33f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.52f, 0.31f, 0.78f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.52f, 0.31f, 0.78f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.52f, 0.31f, 0.78f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.52f, 0.31f, 0.78f, 0.95f);
		colors[ImGuiCol_Tab] = ImVec4(0.29f, 0.17f, 0.44f, 0.86f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.80f);
		colors[ImGuiCol_TabActive] = ImVec4(0.52f, 0.31f, 0.78f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.29f, 0.17f, 0.44f, 0.82f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.25f, 0.64f, 0.80f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.52f, 0.31f, 0.78f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.80f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.52f, 0.31f, 0.78f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.42f, 0.25f, 0.64f, 0.78f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.11f, 0.06f, 0.19f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.42f, 0.25f, 0.64f, 0.43f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.42f, 0.25f, 0.64f, 0.95f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.42f, 0.25f, 0.64f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.42f, 0.25f, 0.64f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.61f);
		break;
	case 4: {
		static float  i = 0;
		i += ImGui::GetIO().DeltaTime;
		float range = 10;
		Vector3 color = rgb(i, range) / 255.f;
		Vector3 color2 = rgb(i, range, .1f) / 255.f;
		//auto font = GetOrAddFont(ENGINEPATH"Fonts/SuperMario256.ttf", 12);
		//ImGui::PushFont(font);
		ImGui::StyleColorsDark();
		colors[ImGuiCol_TitleBg] = Vector4(color * 0.85f, 0.8f);
		colors[ImGuiCol_TitleBgActive] = Vector4(color, 0.8f);
		colors[ImGuiCol_TitleBgCollapsed] = Vector4(color * 0.85f, 0.5f);
		colors[ImGuiCol_FrameBg] = Vector4(color * 0.3f, 1.f);
		colors[ImGuiCol_FrameBgHovered] = Vector4(color * 0.5f, 1.f);
		colors[ImGuiCol_FrameBgActive] = Vector4(color * 0.8f, 1.f);
		colors[ImGuiCol_Border] = Vector4(color * 0.8f, 1.f);
		colors[ImGuiCol_Tab] = Vector4(color * 0.6f, 0.86f);
		colors[ImGuiCol_TabHovered] = Vector4(color * 0.9f, 1.f);
		colors[ImGuiCol_TabActive] = Vector4(color * 0.75f, 1.f);
		colors[ImGuiCol_TabUnfocused] = Vector4(color * 0.2f, 0.9f);
		colors[ImGuiCol_TabUnfocusedActive] = Vector4(color * 0.5f, 1.f);
		colors[ImGuiCol_PlotLines] = Vector4(color2 * 0.5f, 1.f);
		colors[ImGuiCol_PlotLinesHovered] = Vector4(color2 * 0.5f, 1.f);
		colors[ImGuiCol_PlotHistogram] = Vector4(color2 * 0.5f, 1.f);
		colors[ImGuiCol_PlotHistogramHovered] = Vector4(color2 * 0.5f, 1.f);
		colors[ImGuiCol_Button] = Vector4(color * 0.5f, 0.5f);
		colors[ImGuiCol_ButtonHovered] = Vector4(color * 0.8f, 0.8f);
		colors[ImGuiCol_ButtonActive] = Vector4(color * 1.f, 1.f);
		colors[ImGuiCol_Header] = Vector4(color * 0.85f, 0.5f);
		colors[ImGuiCol_HeaderHovered] = Vector4(color * 0.85f, 0.2f);
		colors[ImGuiCol_HeaderActive] = Vector4(color, 0.8f);
		colors[ImGuiCol_TableHeaderBg] = Vector4(color * 0.5f, 1.f);
		colors[ImGuiCol_CheckMark] = Vector4(color2, 1.f);
		//colors[ImGuiCol_Text] = Vector4(color2, 1.f);
		break;
	}
	default:
		break;
	}

}

void Core::Wrapper::WrapperUI::NewFrame()
{
	PopulateFonts();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Core::Wrapper::WrapperUI::EndFrame()
{
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void Core::Wrapper::WrapperUI::Destroy()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Core::Wrapper::WrapperUI::ShowDemoWindow()
{
	ImGui::ShowDemoWindow();
}

float Core::Wrapper::WrapperUI::GetDeltaTime()
{
	return ImGui::GetIO().DeltaTime * Core::App::Get().timeScale;
}

bool Core::Wrapper::WrapperUI::Button(const char* label, const Math::Vector2& size_arg)
{
	return ImGui::Button(label, size_arg);
}

bool Core::Wrapper::WrapperUI::Begin(const char* name, bool* p_open, WindowFlags flags)
{
	return ImGui::Begin(name, p_open, (ImGuiWindowFlags)flags);
}

void Core::Wrapper::WrapperUI::End()
{
	ImGui::End();
}

void Core::Wrapper::WrapperUI::Image(unsigned int user_texture_id, const Math::Vector2& size, const Math::Vector2& uv0 /*= Math::Vector2(0, 0)*/, const Math::Vector2& uv1 /*= Math::Vector2(1, 1)*/, const Math::Vector4& tint_col /*= Math::Vector4(1, 1, 1, 1)*/, const Math::Vector4& border_col /*= Math::Vector4(0, 0, 0, 0)*/)
{
	if (user_texture_id > 0)
		ImGui::Image(reinterpret_cast<void*>(static_cast<uintptr_t>(user_texture_id)), size, uv0, uv1, tint_col, border_col);
}

bool Core::Wrapper::WrapperUI::IsWindowAppearing()
{
	return ImGui::IsWindowAppearing();
}

bool Core::Wrapper::WrapperUI::IsWindowCollapsed()
{
	return ImGui::IsWindowCollapsed();
}

bool Core::Wrapper::WrapperUI::IsWindowFocused(FocusedFlags flags /*= 0*/)
{
	return ImGui::IsWindowFocused((ImGuiFocusedFlags)flags);
}

bool Core::Wrapper::WrapperUI::IsWindowHovered(HoveredFlags flags /*= HoveredFlags::None*/)
{
	return ImGui::IsWindowHovered((ImGuiHoveredFlags)flags);
}

float Core::Wrapper::WrapperUI::GetWindowDpiScale()
{
	return ImGui::GetWindowDpiScale();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetWindowPos()
{
	return ImGui::GetWindowPos();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetWindowSize()
{
	return ImGui::GetWindowSize();
}

float Core::Wrapper::WrapperUI::GetWindowWidth()
{
	return ImGui::GetWindowWidth();
}

float Core::Wrapper::WrapperUI::GetWindowHeight()
{
	return ImGui::GetWindowHeight();
}

bool Core::Wrapper::WrapperUI::BeginChild(const char* str_id, const Math::Vector2& size /*= Math::Vector2(0, 0)*/, bool border /*= false*/, WindowFlags flags /*= WindowFlags::None*/)
{
	return ImGui::BeginChild(str_id, size, border, (ImGuiWindowFlags)flags);
}

bool Core::Wrapper::WrapperUI::BeginChild(uint32_t id, const Math::Vector2& size /*= Math::Vector2(0, 0)*/, bool border /*= false*/, WindowFlags flags /*= WindowFlags::None*/)
{
	return ImGui::BeginChild(id, size, border, (ImGuiWindowFlags)flags);
}

void Core::Wrapper::WrapperUI::EndChild()
{
	ImGui::EndChild();
}

void Core::Wrapper::WrapperUI::SetNextWindowPos(const Math::Vector2& pos, Cond cond /*= Cond::None*/, const Math::Vector2& pivot /*= Math::Vector2(0, 0)*/)
{
	ImGui::SetNextWindowPos(pos, (ImGuiCond)cond, pivot);
}

void Core::Wrapper::WrapperUI::SetNextWindowSize(const Math::Vector2& size, Cond cond /*= Cond::None*/)
{
	ImGui::SetNextWindowSize(size, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetNextWindowSizeConstraints(const Math::Vector2& size_min, const Math::Vector2& size_max, SizeCallback custom_callback /*= NULL*/, void* custom_callback_data /*= NULL*/)
{
	ImGui::SetNextWindowSizeConstraints(size_min, size_max, (ImGuiSizeCallback)custom_callback, custom_callback_data);
}

void Core::Wrapper::WrapperUI::SetNextWindowContentSize(const Math::Vector2& size)
{
	ImGui::SetNextWindowContentSize(size);
}

void Core::Wrapper::WrapperUI::SetNextWindowCollapsed(bool collapsed, Cond cond /*= Cond::None*/)
{
	ImGui::SetNextWindowCollapsed(collapsed, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetNextWindowFocus()
{
	ImGui::SetNextWindowFocus();
}

void Core::Wrapper::WrapperUI::SetNextWindowScroll(const Math::Vector2& scroll)
{
	ImGui::SetNextWindowScroll(scroll);
}

void Core::Wrapper::WrapperUI::SetNextWindowBgAlpha(float alpha)
{
	ImGui::SetNextWindowBgAlpha(alpha);
}

void Core::Wrapper::WrapperUI::SetNextWindowViewport(uint32_t viewport_id)
{
	ImGui::SetNextWindowViewport(viewport_id);
}

void Core::Wrapper::WrapperUI::SetWindowPos(const Math::Vector2& pos, Cond cond /*= Cond::None*/)
{
	ImGui::SetWindowPos(pos, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetWindowPos(const char* name, const Math::Vector2& pos, Cond cond /*= Cond::None*/)
{
	ImGui::SetWindowPos(name, pos, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetWindowSize(const Math::Vector2& size, Cond cond /*= Cond::None*/)
{
	ImGui::SetWindowSize(size, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetWindowSize(const char* name, const Math::Vector2& size, Cond cond /*= Cond::None*/)
{
	ImGui::SetWindowSize(name, size, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetWindowCollapsed(bool collapsed, Cond cond /*= Cond::None*/)
{
	ImGui::SetWindowCollapsed(collapsed, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetWindowCollapsed(const char* name, bool collapsed, Cond cond /*= Cond::None*/)
{
	ImGui::SetWindowCollapsed(name, collapsed, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetWindowFocus()
{
	ImGui::SetWindowFocus();
}

void Core::Wrapper::WrapperUI::SetWindowFocus(const char* name)
{
	ImGui::SetWindowFocus(name);
}

void Core::Wrapper::WrapperUI::SetWindowFontScale(float scale)
{
	ImGui::SetWindowFontScale(scale);
}

Math::Vector2 Core::Wrapper::WrapperUI::GetContentRegionAvail()
{
	return ImGui::GetContentRegionAvail();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetContentRegionMax()
{
	return ImGui::GetContentRegionMax();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetWindowContentRegionMin()
{
	return ImGui::GetWindowContentRegionMin();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetWindowContentRegionMax()
{
	return ImGui::GetWindowContentRegionMax();
}

float Core::Wrapper::WrapperUI::GetScrollX()
{
	return ImGui::GetScrollX();
}

float Core::Wrapper::WrapperUI::GetScrollY()
{
	return ImGui::GetScrollY();
}

float Core::Wrapper::WrapperUI::GetScrollMaxX()
{
	return ImGui::GetScrollMaxX();
}

float Core::Wrapper::WrapperUI::GetScrollMaxY()
{
	return ImGui::GetScrollMaxY();
}

void Core::Wrapper::WrapperUI::SetScrollHereX(float center_x_ratio /*= 0.5f*/)
{
	ImGui::SetScrollHereX(center_x_ratio);
}

void Core::Wrapper::WrapperUI::SetScrollHereY(float center_y_ratio /*= 0.5f*/)
{
	ImGui::SetScrollHereY(center_y_ratio);
}

void Core::Wrapper::WrapperUI::SetScrollFromPosX(float local_x, float center_x_ratio /*= 0.5f*/)
{
	ImGui::SetScrollFromPosX(local_x, center_x_ratio);
}

void Core::Wrapper::WrapperUI::SetScrollFromPosY(float local_y, float center_y_ratio /*= 0.5f*/)
{
	ImGui::SetScrollFromPosY(local_y, center_y_ratio);
}

void Core::Wrapper::WrapperUI::SetScrollX(float scroll_x)
{
	ImGui::SetScrollX(scroll_x);
}

void Core::Wrapper::WrapperUI::SetScrollY(float scroll_y)
{
	ImGui::SetScrollY(scroll_y);
}

void Core::Wrapper::WrapperUI::PushStyleColor(Col idx, uint32_t col)
{
	ImGui::PushStyleColor((ImGuiCol)idx, col);
}

void Core::Wrapper::WrapperUI::PushStyleColor(Col idx, const Math::Vector4& col)
{
	ImGui::PushStyleColor((ImGuiCol)idx, col);
}

void Core::Wrapper::WrapperUI::PopStyleColor(int count /*= 1*/)
{
	ImGui::PopStyleColor(count);
}

void Core::Wrapper::WrapperUI::PushStyleVar(StyleVar idx, float val)
{
	ImGui::PushStyleVar((ImGuiStyleVar)idx, val);
}

void Core::Wrapper::WrapperUI::PushStyleVar(StyleVar idx, const Math::Vector2& val)
{
	ImGui::PushStyleVar((ImGuiStyleVar)idx, val);
}

void Core::Wrapper::WrapperUI::PopStyleVar(int count /*= 1*/)
{
	ImGui::PopStyleVar(count);
}

void Core::Wrapper::WrapperUI::PushAllowKeyboardFocus(bool allow_keyboard_focus)
{
	ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
}

void Core::Wrapper::WrapperUI::PopAllowKeyboardFocus()
{
	ImGui::PopAllowKeyboardFocus();
}

void Core::Wrapper::WrapperUI::PushButtonRepeat(bool repeat)
{
	ImGui::PushButtonRepeat(repeat);
}

void Core::Wrapper::WrapperUI::PopButtonRepeat()
{
	ImGui::PopButtonRepeat();
}

void Core::Wrapper::WrapperUI::PushItemWidth(float item_width)
{
	ImGui::PushItemWidth(item_width);
}

void Core::Wrapper::WrapperUI::PopItemWidth()
{
	ImGui::PopItemWidth();
}

void Core::Wrapper::WrapperUI::SetNextItemWidth(float item_width)
{
	ImGui::SetNextItemWidth(item_width);
}

float Core::Wrapper::WrapperUI::CalcItemWidth()
{
	return ImGui::CalcItemWidth();
}

void Core::Wrapper::WrapperUI::PushTextWrapPos(float wrap_local_pos_x /*= 0.0f*/)
{
	ImGui::PushTextWrapPos(wrap_local_pos_x);
}

void Core::Wrapper::WrapperUI::PopTextWrapPos()
{
	ImGui::PopTextWrapPos();
}

float Core::Wrapper::WrapperUI::GetFontSize()
{
	return ImGui::GetFontSize();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetFontTexUvWhitePixel()
{
	return ImGui::GetFontTexUvWhitePixel();
}

uint32_t Core::Wrapper::WrapperUI::GetColorU32(Col idx, float alpha_mul /*= 1.0f*/)
{
	return ImGui::GetColorU32((ImGuiCol)idx, alpha_mul);
}

uint32_t Core::Wrapper::WrapperUI::GetColorU32(const Math::Vector4& col)
{
	return ImGui::GetColorU32(col);
}

uint32_t Core::Wrapper::WrapperUI::GetColorU32(uint32_t col)
{
	return ImGui::GetColorU32(col);
}

const Math::Vector4 Core::Wrapper::WrapperUI::GetStyleColorVec4(Col idx)
{
	return ImGui::GetStyleColorVec4((ImGuiCol)idx);
}

void Core::Wrapper::WrapperUI::Separator()
{
	ImGui::Separator();
}

void Core::Wrapper::WrapperUI::SameLine(float offset_from_start_x /*= 0.0f*/, float spacing /*= -1.0f*/)
{
	ImGui::SameLine(offset_from_start_x, spacing);
}

void Core::Wrapper::WrapperUI::NewLine()
{
	ImGui::NewLine();
}

void Core::Wrapper::WrapperUI::Spacing()
{
	ImGui::Spacing();
}

void Core::Wrapper::WrapperUI::Dummy(const Math::Vector2& size)
{
	ImGui::Dummy(size);
}

void Core::Wrapper::WrapperUI::Indent(float indent_w /*= 0.0f*/)
{
	ImGui::Indent(indent_w);
}

void Core::Wrapper::WrapperUI::Unindent(float indent_w /*= 0.0f*/)
{
	ImGui::Unindent(indent_w);
}

void Core::Wrapper::WrapperUI::BeginGroup()
{
	ImGui::BeginGroup();
}

void Core::Wrapper::WrapperUI::EndGroup()
{
	ImGui::EndGroup();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetCursorPos()
{
	return ImGui::GetCursorPos();
}

float Core::Wrapper::WrapperUI::GetCursorPosX()
{
	return ImGui::GetCursorPosX();
}

float Core::Wrapper::WrapperUI::GetCursorPosY()
{
	return ImGui::GetCursorPosY();
}

void Core::Wrapper::WrapperUI::SetCursorPos(const Math::Vector2& local_pos)
{
	ImGui::SetCursorPos(local_pos);
}

void Core::Wrapper::WrapperUI::SetCursorPosX(float local_x)
{
	ImGui::SetCursorPosX(local_x);
}

void Core::Wrapper::WrapperUI::SetCursorPosY(float local_y)
{
	ImGui::SetCursorPosY(local_y);
}

Math::Vector2 Core::Wrapper::WrapperUI::GetCursorStartPos()
{
	return ImGui::GetCursorStartPos();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetCursorScreenPos()
{
	return ImGui::GetCursorScreenPos();
}

void Core::Wrapper::WrapperUI::SetCursorScreenPos(const Math::Vector2& pos)
{
	ImGui::SetCursorScreenPos(pos);
}

void Core::Wrapper::WrapperUI::AlignTextToFramePadding()
{
	ImGui::AlignTextToFramePadding();
}

float Core::Wrapper::WrapperUI::GetTextLineHeight()
{
	return ImGui::GetTextLineHeight();
}

float Core::Wrapper::WrapperUI::GetTextLineHeightWithSpacing()
{
	return ImGui::GetTextLineHeightWithSpacing();
}

float Core::Wrapper::WrapperUI::GetFrameHeight()
{
	return ImGui::GetFrameHeight();
}

float Core::Wrapper::WrapperUI::GetFrameHeightWithSpacing()
{
	return ImGui::GetFrameHeightWithSpacing();
}

void Core::Wrapper::WrapperUI::PushID(const char* str_id)
{
	ImGui::PushID(str_id);
}

void Core::Wrapper::WrapperUI::PushID(const char* str_id_begin, const char* str_id_end)
{
	ImGui::PushID(str_id_begin, str_id_end);
}

void Core::Wrapper::WrapperUI::PushID(const void* ptr_id)
{
	ImGui::PushID(ptr_id);
}

void Core::Wrapper::WrapperUI::PushID(int int_id)
{
	ImGui::PushID(int_id);
}

void Core::Wrapper::WrapperUI::PopID()
{
	ImGui::PopID();
}

uint32_t Core::Wrapper::WrapperUI::GetID(const char* str_id)
{
	return ImGui::GetID(str_id);
}

uint32_t Core::Wrapper::WrapperUI::GetID(const char* str_id_begin, const char* str_id_end)
{
	return ImGui::GetID(str_id_begin, str_id_end);
}

uint32_t Core::Wrapper::WrapperUI::GetID(const void* ptr_id)
{
	return ImGui::GetID(ptr_id);
}

void Core::Wrapper::WrapperUI::Text(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt, args);
	va_end(args);
}

void Core::Wrapper::WrapperUI::TextColored(const Math::Vector4& col, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextColoredV(col, fmt, args);
	va_end(args);
}

void Core::Wrapper::WrapperUI::TextUnformatted(const char* text, const char* text_end /*= NULL*/)
{
	ImGui::TextUnformatted(text, text_end);
}
void Core::Wrapper::WrapperUI::TextDisabled(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextDisabledV(fmt, args);
	va_end(args);
}

void Core::Wrapper::WrapperUI::TextWrapped(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextWrappedV(fmt, args);
	va_end(args);
}

void Core::Wrapper::WrapperUI::LabelText(const char* label, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::LabelTextV(label, fmt, args);
	va_end(args);
}

void Core::Wrapper::WrapperUI::BulletText(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::BulletTextV(fmt, args);
	va_end(args);
}

void Core::Wrapper::WrapperUI::SeparatorText(const char* label)
{
	ImGui::SeparatorText(label);
}

bool Core::Wrapper::WrapperUI::SmallButton(const char* label)
{
	return ImGui::SmallButton(label);
}

bool Core::Wrapper::WrapperUI::InvisibleButton(const char* str_id, const Math::Vector2& size, ButtonFlags flags /*= ButtonFlags::None*/)
{
	return ImGui::InvisibleButton(str_id, size, (ImGuiButtonFlags)flags);
}

bool Core::Wrapper::WrapperUI::ArrowButton(const char* str_id, Dir dir)
{
	return ImGui::ArrowButton(str_id, (ImGuiDir)dir);
}

bool Core::Wrapper::WrapperUI::Checkbox(const char* label, bool* v)
{
	return ImGui::Checkbox(label, v);
}

bool Core::Wrapper::WrapperUI::CheckboxFlags(const char* label, int* flags, int flags_value)
{
	return ImGui::CheckboxFlags(label, flags, flags_value);
}

bool Core::Wrapper::WrapperUI::CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value)
{
	return ImGui::CheckboxFlags(label, flags, flags_value);
}

bool Core::Wrapper::WrapperUI::RadioButton(const char* label, bool active)
{
	return ImGui::RadioButton(label, active);
}

bool Core::Wrapper::WrapperUI::RadioButton(const char* label, int* v, int v_button)
{
	return ImGui::RadioButton(label, v, v_button);
}

void Core::Wrapper::WrapperUI::ProgressBar(float fraction, const Math::Vector2& size_arg /*= Math::Vector2(-FLT_MIN, 0)*/, const char* overlay /*= NULL*/)
{
	ImGui::ProgressBar(fraction, size_arg, overlay);
}

void Core::Wrapper::WrapperUI::Bullet()
{
	ImGui::Bullet();
}

bool Core::Wrapper::WrapperUI::ImageButton(const char* str_id, void* user_texture_id, const Math::Vector2& size, const Math::Vector2& uv0 /*= Math::Vector2(0, 0)*/, const Math::Vector2& uv1 /*= Math::Vector2(1, 1)*/, const Math::Vector4& bg_col /*= Math::Vector4(0, 0, 0, 0)*/, const Math::Vector4& tint_col /*= Math::Vector4(1, 1, 1, 1)*/)
{
	return ImGui::ImageButton(str_id, user_texture_id, size, uv0, uv1, bg_col, tint_col);
}

bool Core::Wrapper::WrapperUI::ImageButton(unsigned int user_texture_id, const Math::Vector2& size, const Math::Vector2& uv0 /*= Math::Vector2(0, 0)*/, const Math::Vector2& uv1 /*= Math::Vector2(1, 1)*/, int frame_padding /*= -1*/, const Math::Vector4& bg_col /*= Math::Vector4(0, 0, 0, 0)*/, const Math::Vector4& tint_col /*= Math::Vector4(1, 1, 1, 1)*/)
{
	return ImGui::ImageButton(reinterpret_cast<void*>(static_cast<uintptr_t>(user_texture_id)), size, uv0, uv1, frame_padding, bg_col, tint_col);
}

bool Core::Wrapper::WrapperUI::BeginCombo(const char* label, const char* preview_value, ComboFlags flags /*= ComboFlags::None*/)
{
	return ImGui::BeginCombo(label, preview_value, (ImGuiComboFlags)flags);
}

void Core::Wrapper::WrapperUI::EndCombo()
{
	ImGui::EndCombo();
}

bool Core::Wrapper::WrapperUI::Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items /*= -1*/)
{
	return ImGui::Combo(label, current_item, items, items_count, popup_max_height_in_items);
}

bool Core::Wrapper::WrapperUI::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items /*= -1*/)
{
	return ImGui::Combo(label, current_item, items_separated_by_zeros, popup_max_height_in_items);
}

bool Core::Wrapper::WrapperUI::Combo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items /*= -1*/)
{
	return ImGui::Combo(label, current_item, items_getter, data, items_count, popup_max_height_in_items);
}

bool Core::Wrapper::WrapperUI::DragFloat(const char* label, float* v, float v_speed /*= 1.0f*/, float v_min /*= 0.0f*/, float v_max /*= 0.0f*/, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragFloat(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragFloat2(const char* label, float v[2], float v_speed /*= 1.0f*/, float v_min /*= 0.0f*/, float v_max /*= 0.0f*/, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragFloat2(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragFloat3(const char* label, float v[3], float v_speed /*= 1.0f*/, float v_min /*= 0.0f*/, float v_max /*= 0.0f*/, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragFloat3(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragFloat4(const char* label, float v[4], float v_speed /*= 1.0f*/, float v_min /*= 0.0f*/, float v_max /*= 0.0f*/, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragFloat4(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed /*= 1.0f*/, float v_min /*= 0.0f*/, float v_max /*= 0.0f*/, const char* format /*= "%.3f"*/, const char* format_max /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragFloatRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragInt(const char* label, int* v, float v_speed /*= 1.0f*/, int v_min /*= 0*/, int v_max /*= 0*/, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragInt(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragInt2(const char* label, int v[2], float v_speed /*= 1.0f*/, int v_min /*= 0*/, int v_max /*= 0*/, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragInt2(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragInt3(const char* label, int v[3], float v_speed /*= 1.0f*/, int v_min /*= 0*/, int v_max /*= 0*/, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragInt3(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragInt4(const char* label, int v[4], float v_speed /*= 1.0f*/, int v_min /*= 0*/, int v_max /*= 0*/, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragInt4(label, v, v_speed, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed /*= 1.0f*/, int v_min /*= 0*/, int v_max /*= 0*/, const char* format /*= "%d"*/, const char* format_max /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragIntRange2(label, v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragScalar(const char* label, DataType data_type, void* p_data, float v_speed /*= 1.0f*/, const void* p_min /*= NULL*/, const void* p_max /*= NULL*/, const char* format /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragScalar(label, (ImGuiDataType)data_type, p_data, v_speed, p_min, p_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::DragScalarN(const char* label, DataType data_type, void* p_data, int components, float v_speed /*= 1.0f*/, const void* p_min /*= NULL*/, const void* p_max /*= NULL*/, const char* format /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::DragScalarN(label, (ImGuiDataType)data_type, p_data, components, v_speed, p_min, p_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderFloat(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderFloat2(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderFloat3(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderFloat4(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderInt(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderAngle(const char* label, float* v_rad, float v_degrees_min /*= -360.0f*/, float v_degrees_max /*= +360.0f*/, const char* format /*= "%.0f deg"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderAngle(label, v_rad, v_degrees_min, v_degrees_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderInt(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderInt3(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderInt4(label, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderScalar(const char* label, DataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderScalar(label, (ImGuiDataType)data_type, p_data, p_min, p_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderScalarN(const char* label, DataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::SliderScalarN(label, (ImGuiDataType)data_type, p_data, components, p_min, p_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::VSliderFloat(const char* label, const Math::Vector2& size, float* v, float v_min, float v_max, const char* format /*= "%.3f"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::VSliderFloat(label, size, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::VSliderInt(const char* label, const Math::Vector2& size, int* v, int v_min, int v_max, const char* format /*= "%d"*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::VSliderInt(label, size, v, v_min, v_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::VSliderScalar(const char* label, const Math::Vector2& size, DataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format /*= NULL*/, SliderFlags flags /*= SliderFlags::None*/)
{
	return ImGui::VSliderScalar(label, size, (ImGuiDataType)data_type, p_data, p_min, p_max, format, (ImGuiSliderFlags)flags);
}

bool Core::Wrapper::WrapperUI::SliderMat4(const char* label, float v[16], float v_min, float v_max, const char* format, SliderFlags flags)
{
	bool isModified = false;
	ImGui::Text(label);
	if (SliderFloat4("##00", &v[0], v_min, v_max, format, flags))
		isModified = true;
	if (SliderFloat4("##01", &v[4], v_min, v_max, format, flags))
		isModified = true;
	if (SliderFloat4("##02", &v[8], v_min, v_max, format, flags))
		isModified = true;
	if (SliderFloat4("##03", &v[12], v_min, v_max, format, flags))
		isModified = true;
	return isModified;
}

bool Core::Wrapper::WrapperUI::InputText(const char* label, char* buf, size_t buf_size, InputTextFlags flags /*= InputTextFlags::None*/, InputTextCallback callback /*= NULL*/, void* user_data /*= NULL*/)
{
	return ImGui::InputText(label, buf, buf_size, (ImGuiInputTextFlags)flags, (ImGuiInputTextCallback)callback, user_data);
}

bool Core::Wrapper::WrapperUI::InputTextMultiline(const char* label, char* buf, size_t buf_size, const Math::Vector2& size /*= Math::Vector2(0, 0)*/, InputTextFlags flags /*= InputTextFlags::None*/, InputTextCallback callback /*= NULL*/, void* user_data /*= NULL*/)
{
	return ImGui::InputTextMultiline(label, buf, buf_size, size, (ImGuiInputTextFlags)flags, (ImGuiInputTextCallback)callback, user_data);
}

bool Core::Wrapper::WrapperUI::InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, InputTextFlags flags /*= InputTextFlags::None*/, InputTextCallback callback /*= NULL*/, void* user_data /*= NULL*/)
{
	return ImGui::InputTextWithHint(label, hint, buf, buf_size, (ImGuiInputTextFlags)flags, (ImGuiInputTextCallback)callback, user_data);
}

bool Core::Wrapper::WrapperUI::InputFloat(const char* label, float* v, float step /*= 0.0f*/, float step_fast /*= 0.0f*/, const char* format /*= "%.3f"*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputFloat(label, v, step, step_fast, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputFloat2(const char* label, float v[2], const char* format /*= "%.3f"*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputFloat2(label, v, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputFloat3(const char* label, float v[3], const char* format /*= "%.3f"*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputFloat3(label, v, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputFloat4(const char* label, float v[4], const char* format /*= "%.3f"*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputFloat4(label, v, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputInt(const char* label, int* v, int step /*= 1*/, int step_fast /*= 100*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputInt(label, v, step, step_fast, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputInt2(const char* label, int v[2], InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputInt2(label, v, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputInt3(const char* label, int v[3], InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputInt3(label, v, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputInt4(const char* label, int v[4], InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputInt4(label, v, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputDouble(const char* label, double* v, double step /*= 0.0*/, double step_fast /*= 0.0*/, const char* format /*= "%.6f"*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputDouble(label, v, step, step_fast, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputScalar(const char* label, DataType data_type, void* p_data, const void* p_step /*= NULL*/, const void* p_step_fast /*= NULL*/, const char* format /*= NULL*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputScalar(label, (ImGuiDataType)data_type, p_data, p_step, p_step_fast, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::InputScalarN(const char* label, DataType data_type, void* p_data, int components, const void* p_step /*= NULL*/, const void* p_step_fast /*= NULL*/, const char* format /*= NULL*/, InputTextFlags flags /*= InputTextFlags::None*/)
{
	return ImGui::InputScalarN(label, (ImGuiDataType)data_type, p_data, components, p_step, p_step_fast, format, (ImGuiInputTextFlags)flags);
}

bool Core::Wrapper::WrapperUI::ColorEdit3(const char* label, float col[3], ColorEditFlags flags /*= ColorEditFlags::None*/)
{
	return ImGui::ColorEdit3(label, col, (ImGuiColorEditFlags)flags);
}

bool Core::Wrapper::WrapperUI::ColorEdit4(const char* label, float col[4], ColorEditFlags flags /*= ColorEditFlags::None*/)
{
	return ImGui::ColorEdit4(label, col, (ImGuiColorEditFlags)flags);
}

bool Core::Wrapper::WrapperUI::ColorPicker3(const char* label, float col[3], ColorEditFlags flags /*= ColorEditFlags::None*/)
{
	return ImGui::ColorPicker3(label, col, (ImGuiColorEditFlags)flags);
}

bool Core::Wrapper::WrapperUI::ColorPicker4(const char* label, float col[4], ColorEditFlags flags /*= ColorEditFlags::None*/, const float* ref_col /*= NULL*/)
{
	return ImGui::ColorPicker4(label, col, (ImGuiColorEditFlags)flags);
}

bool Core::Wrapper::WrapperUI::ColorButton(const char* desc_id, const Math::Vector4& col, ColorEditFlags flags /*= ColorEditFlags::None*/, const Math::Vector2& size /*= Math::Vector2(0, 0)*/)
{
	return ImGui::ColorButton(desc_id, col, (ImGuiColorEditFlags)flags, size);
}

void Core::Wrapper::WrapperUI::SetColorEditOptions(ColorEditFlags flags)
{
	ImGui::SetColorEditOptions((ImGuiColorEditFlags)flags);
}

bool Core::Wrapper::WrapperUI::TreeNode(const char* label)
{
	return ImGui::TreeNode(label);
}

bool Core::Wrapper::WrapperUI::TreeNode(const char* str_id, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = ImGui::TreeNodeExV(str_id, 0, fmt, args);
	va_end(args);
	return is_open;
}

bool Core::Wrapper::WrapperUI::TreeNode(const void* ptr_id, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = ImGui::TreeNodeExV(ptr_id, 0, fmt, args);
	va_end(args);
	return is_open;
}

bool Core::Wrapper::WrapperUI::TreeNodeEx(const char* label, TreeNodeFlags flags /*= TreeNodeFlags::None*/)
{
	return ImGui::TreeNodeEx(label, (ImGuiTreeNodeFlags)flags);
}

bool Core::Wrapper::WrapperUI::TreeNodeEx(const char* str_id, TreeNodeFlags flags, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = ImGui::TreeNodeExV(str_id, (ImGuiTreeNodeFlags)flags, fmt, args);
	va_end(args);
	return is_open;
}

bool Core::Wrapper::WrapperUI::TreeNodeEx(const void* ptr_id, TreeNodeFlags flags, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	bool is_open = ImGui::TreeNodeExV(ptr_id, (ImGuiTreeNodeFlags)flags, fmt, args);
	va_end(args);
	return is_open;
}

void Core::Wrapper::WrapperUI::TreePush(const char* str_id)
{
	ImGui::TreePush(str_id);
}

void Core::Wrapper::WrapperUI::TreePush(const void* ptr_id)
{
	ImGui::TreePush(ptr_id);
}

void Core::Wrapper::WrapperUI::TreePop()
{
	ImGui::TreePop();
}

float Core::Wrapper::WrapperUI::GetTreeNodeToLabelSpacing()
{
	return ImGui::GetTreeNodeToLabelSpacing();
}

bool Core::Wrapper::WrapperUI::CollapsingHeader(const char* label, TreeNodeFlags flags /*= TreeNodeFlags::None*/)
{
	return ImGui::CollapsingHeader(label, (ImGuiTreeNodeFlags)flags);
}

bool Core::Wrapper::WrapperUI::CollapsingHeader(const char* label, bool* p_visible, TreeNodeFlags flags /*= TreeNodeFlags::None*/)
{
	return ImGui::CollapsingHeader(label, p_visible, (ImGuiTreeNodeFlags)flags);
}

void Core::Wrapper::WrapperUI::SetNextItemOpen(bool is_open, Cond cond /*= Cond::None*/)
{
	ImGui::SetNextItemOpen(is_open, (ImGuiCond)cond);
}

bool Core::Wrapper::WrapperUI::Selectable(const char* label, bool selected /*= false*/, SelectableFlags flags /*= SelectableFlags::None*/, const Math::Vector2& size /*= Math::Vector2(0, 0)*/)
{
	return ImGui::Selectable(label, selected, (ImGuiSelectableFlags)flags);
}

bool Core::Wrapper::WrapperUI::Selectable(const char* label, bool* p_selected, SelectableFlags flags /*= SelectableFlags::None*/, const Math::Vector2& size /*= Math::Vector2(0, 0)*/)
{
	return ImGui::Selectable(label, p_selected, (ImGuiSelectableFlags)flags);
}

bool Core::Wrapper::WrapperUI::BeginListBox(const char* label, const Math::Vector2& size /*= Math::Vector2(0, 0)*/)
{
	return ImGui::BeginListBox(label, size);
}

void Core::Wrapper::WrapperUI::EndListBox()
{
	return ImGui::EndListBox();
}

bool Core::Wrapper::WrapperUI::ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items /*= -1*/)
{
	return ImGui::ListBox(label, current_item, items, items_count, height_in_items);
}

bool Core::Wrapper::WrapperUI::ListBox(const char* label, int* current_item, bool (*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items /*= -1*/)
{
	return ImGui::ListBox(label, current_item, items_getter, data, items_count, height_in_items);
}

void Core::Wrapper::WrapperUI::PlotLines(const char* label, const float* values, int values_count, int values_offset /*= 0*/, const char* overlay_text /*= NULL*/, float scale_min /*= FLT_MAX*/, float scale_max /*= FLT_MAX*/, Math::Vector2 graph_size /*= Math::Vector2(0, 0)*/, int stride /*= sizeof(float)*/)
{
	return ImGui::PlotLines(label, values, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Core::Wrapper::WrapperUI::PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset /*= 0*/, const char* overlay_text /*= NULL*/, float scale_min /*= FLT_MAX*/, float scale_max /*= FLT_MAX*/, Math::Vector2 graph_size /*= Math::Vector2(0, 0)*/)
{
	return ImGui::PlotLines(label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Core::Wrapper::WrapperUI::PlotHistogram(const char* label, const float* values, int values_count, int values_offset /*= 0*/, const char* overlay_text /*= NULL*/, float scale_min /*= FLT_MAX*/, float scale_max /*= FLT_MAX*/, Math::Vector2 graph_size /*= Math::Vector2(0, 0)*/, int stride /*= sizeof(float)*/)
{
	return ImGui::PlotHistogram(label, values, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void Core::Wrapper::WrapperUI::PlotHistogram(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset /*= 0*/, const char* overlay_text /*= NULL*/, float scale_min /*= FLT_MAX*/, float scale_max /*= FLT_MAX*/, Math::Vector2 graph_size /*= Math::Vector2(0, 0)*/)
{
	return ImGui::PlotHistogram(label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

bool Core::Wrapper::WrapperUI::BeginMenuBar()
{
	return ImGui::BeginMenuBar();
}

void Core::Wrapper::WrapperUI::EndMenuBar()
{
	ImGui::EndMenuBar();
}

bool Core::Wrapper::WrapperUI::BeginMainMenuBar()
{
	return ImGui::BeginMainMenuBar();
}

void Core::Wrapper::WrapperUI::EndMainMenuBar()
{
	ImGui::EndMainMenuBar();
}

bool Core::Wrapper::WrapperUI::BeginMenu(const char* label, bool enabled /*= true*/)
{
	return ImGui::BeginMenu(label, enabled);
}

void Core::Wrapper::WrapperUI::EndMenu()
{
	ImGui::EndMenu();
}

bool Core::Wrapper::WrapperUI::MenuItem(const char* label, const char* shortcut /*= NULL*/, bool selected /*= false*/, bool enabled /*= true*/)
{
	return ImGui::MenuItem(label, shortcut, selected, enabled);
}

bool Core::Wrapper::WrapperUI::MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled /*= true*/)
{
	return ImGui::MenuItem(label, shortcut, p_selected, enabled);
}

void Core::Wrapper::WrapperUI::BeginTooltip()
{
	ImGui::BeginTooltip();
}

void Core::Wrapper::WrapperUI::EndTooltip()
{
	ImGui::EndTooltip();
}

void Core::Wrapper::WrapperUI::SetTooltip(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::SetTooltipV(fmt, args);
	va_end(args);
}

bool Core::Wrapper::WrapperUI::BeginPopup(const char* str_id, WindowFlags flags /*= WindowFlags::None*/)
{
	return ImGui::BeginPopup(str_id, (ImGuiWindowFlags)flags);
}

bool Core::Wrapper::WrapperUI::BeginPopupModal(const char* name, bool* p_open /*= NULL*/, WindowFlags flags /*= WindowFlags::None*/)
{
	return ImGui::BeginPopupModal(name, p_open, (ImGuiWindowFlags)flags);
}

void Core::Wrapper::WrapperUI::EndPopup()
{
	ImGui::EndPopup();
}

void Core::Wrapper::WrapperUI::OpenPopup(const char* str_id, PopupFlags popup_flags /*= PopupFlags::None*/)
{
	ImGui::OpenPopup(str_id, (ImGuiPopupFlags)popup_flags);
}

void Core::Wrapper::WrapperUI::OpenPopup(uint32_t id, PopupFlags popup_flags /*= PopupFlags::None*/)
{
	ImGui::OpenPopup(id, (ImGuiPopupFlags)popup_flags);
}

void Core::Wrapper::WrapperUI::OpenPopupOnItemClick(const char* str_id /*= NULL*/, PopupFlags popup_flags /*= PopupFlags::MouseButtonRight*/)
{
	ImGui::OpenPopupOnItemClick(str_id, (ImGuiPopupFlags)popup_flags);
}

void Core::Wrapper::WrapperUI::CloseCurrentPopup()
{
	ImGui::CloseCurrentPopup();
}

bool Core::Wrapper::WrapperUI::BeginPopupContextItem(const char* str_id /*= NULL*/, PopupFlags popup_flags /*= PopupFlags::MouseButtonRight*/)
{
	return ImGui::BeginPopupContextItem(str_id, (ImGuiPopupFlags)popup_flags);
}

bool Core::Wrapper::WrapperUI::BeginPopupContextWindow(const char* str_id /*= NULL*/, PopupFlags popup_flags /*= PopupFlags::MouseButtonRight*/)
{
	return ImGui::BeginPopupContextWindow(str_id, (ImGuiPopupFlags)popup_flags);
}

bool Core::Wrapper::WrapperUI::BeginPopupContextVoid(const char* str_id /*= NULL*/, PopupFlags popup_flags /*= PopupFlags::MouseButtonRight*/)
{
	return ImGui::BeginPopupContextVoid(str_id, (ImGuiPopupFlags)popup_flags);
}

bool Core::Wrapper::WrapperUI::IsPopupOpen(const char* str_id, PopupFlags flags /*= PopupFlags::None*/)
{
	return ImGui::IsPopupOpen(str_id, (ImGuiPopupFlags)flags);
}

bool Core::Wrapper::WrapperUI::BeginTable(const char* str_id, int column, TableFlags flags /*= TableFlags::None*/, const Math::Vector2& outer_size /*= Math::Vector2(0.0f, 0.0f)*/, float inner_width /*= 0.0f*/)
{
	return ImGui::BeginTable(str_id, column, (ImGuiTableFlags)flags, outer_size, inner_width);
}

void Core::Wrapper::WrapperUI::EndTable()
{
	ImGui::EndTable();
}

void Core::Wrapper::WrapperUI::TableNextRow(TableFlags row_flags /*= TableFlags::None*/, float min_row_height /*= 0.0f*/)
{
	ImGui::TableNextRow((ImGuiTableFlags)row_flags, min_row_height);
}

bool Core::Wrapper::WrapperUI::TableNextColumn()
{
	return ImGui::TableNextColumn();
}

bool Core::Wrapper::WrapperUI::TableSetColumnIndex(int column_n)
{
	return ImGui::TableSetColumnIndex(column_n);
}

void Core::Wrapper::WrapperUI::TableSetupColumn(const char* label, TableColumnFlags flags /*= TableFlags::None*/, float init_width_or_weight /*= 0.0f*/, uint32_t user_id /*= 0*/)
{
	ImGui::TableSetupColumn(label, (ImGuiTableFlags)flags, init_width_or_weight, user_id);
}

void Core::Wrapper::WrapperUI::TableSetupScrollFreeze(int cols, int rows)
{
	ImGui::TableSetupScrollFreeze(cols, rows);
}

void Core::Wrapper::WrapperUI::TableHeadersRow()
{
	ImGui::TableHeadersRow();
}

void Core::Wrapper::WrapperUI::TableHeader(const char* label)
{
	ImGui::TableHeader(label);
}

TableSortSpecs* Core::Wrapper::WrapperUI::TableGetSortSpecs()
{
	return (TableSortSpecs*)ImGui::TableGetSortSpecs();
}

int Core::Wrapper::WrapperUI::TableGetColumnCount()
{
	return ImGui::TableGetColumnCount();
}

int Core::Wrapper::WrapperUI::TableGetColumnIndex()
{
	return ImGui::TableGetColumnIndex();
}

int Core::Wrapper::WrapperUI::TableGetRowIndex()
{
	return ImGui::TableGetRowIndex();
}

const char* Core::Wrapper::WrapperUI::TableGetColumnName(int column_n /*= -1*/)
{
	return ImGui::TableGetColumnName(column_n);
}

TableColumnFlags Core::Wrapper::WrapperUI::TableGetColumnFlags(int column_n /*= -1*/)
{
	return (TableColumnFlags)ImGui::TableGetColumnFlags(column_n);
}

void Core::Wrapper::WrapperUI::TableSetColumnEnabled(int column_n, bool v)
{
	ImGui::TableSetColumnEnabled(column_n, v);
}

void Core::Wrapper::WrapperUI::TableSetBgColor(TableBgTarget target, uint32_t color, int column_n /*= -1*/)
{
	ImGui::TableSetBgColor((ImGuiTableBgTarget)target, color, column_n);
}

void Core::Wrapper::WrapperUI::Columns(int count /*= 1*/, const char* id /*= NULL*/, bool border /*= true*/)
{
	ImGui::Columns(count, id, border);
}

void Core::Wrapper::WrapperUI::NextColumn()
{
	ImGui::NextColumn();
}

int Core::Wrapper::WrapperUI::GetColumnIndex()
{
	return ImGui::GetColumnIndex();
}

float Core::Wrapper::WrapperUI::GetColumnWidth(int column_index /*= -1*/)
{
	return ImGui::GetColumnWidth(column_index);
}

void Core::Wrapper::WrapperUI::SetColumnWidth(int column_index, float width)
{
	ImGui::SetColumnWidth(column_index, width);
}

float Core::Wrapper::WrapperUI::GetColumnOffset(int column_index /*= -1*/)
{
	return ImGui::GetColumnOffset(column_index);
}

void Core::Wrapper::WrapperUI::SetColumnOffset(int column_index, float offset_x)
{
	ImGui::SetColumnOffset(column_index, offset_x);
}

int Core::Wrapper::WrapperUI::GetColumnsCount()
{
	return ImGui::GetColumnsCount();
}

bool Core::Wrapper::WrapperUI::BeginTabBar(const char* str_id, TabBarFlags flags /*= TabBarFlags::None*/)
{
	return ImGui::BeginTabBar(str_id, (ImGuiTabBarFlags)flags);
}

void Core::Wrapper::WrapperUI::EndTabBar()
{
	ImGui::EndTabBar();
}

void Core::Wrapper::WrapperUI::EndTabItem()
{
	ImGui::EndTabItem();
}

bool Core::Wrapper::WrapperUI::BeginTabItem(const char* label, bool* p_open /*= NULL*/, TabItemFlags flags /*= TabItemFlags::None*/)
{
	return ImGui::BeginTabItem(label, p_open, (ImGuiTabItemFlags)flags);
}

bool Core::Wrapper::WrapperUI::TabItemButton(const char* label, TabItemFlags flags /*= TabItemFlags::None*/)
{
	return ImGui::TabItemButton(label, (ImGuiTabItemFlags)flags);
}

void Core::Wrapper::WrapperUI::SetTabItemClosed(const char* tab_or_docked_window_label)
{
	ImGui::SetTabItemClosed(tab_or_docked_window_label);
}

uint32_t Core::Wrapper::WrapperUI::DockSpace(uint32_t id, const Math::Vector2& size /*= Math::Vector2(0, 0)*/, DockNodeFlags flags /*= DockNodeFlags::None*/, const WindowClass* window_class /*= NULL*/)
{
	return ImGui::DockSpace(id, size, (ImGuiDockNodeFlags)flags, (ImGuiWindowClass*)window_class);
}

void Core::Wrapper::WrapperUI::SetNextWindowDockID(uint32_t dock_id, Cond cond /*= Cond::None*/)
{
	ImGui::SetNextWindowDockID(dock_id, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::SetNextWindowClass(const WindowClass* window_class)
{
	ImGui::SetNextWindowClass((ImGuiWindowClass*)window_class);
}

uint32_t Core::Wrapper::WrapperUI::GetWindowDockID()
{
	return ImGui::GetWindowDockID();
}

bool Core::Wrapper::WrapperUI::IsWindowDocked()
{
	return ImGui::IsWindowDocked();
}

bool Core::Wrapper::WrapperUI::BeginDragDropSource(DragDropFlags flags /*= DragDropFlags::None*/)
{
	return ImGui::BeginDragDropSource((ImGuiDragDropFlags)flags);
}

bool Core::Wrapper::WrapperUI::SetDragDropPayload(const char* type, const void* data, size_t sz, Cond cond /*= Cond::None*/)
{
	return ImGui::SetDragDropPayload(type, data, sz, (ImGuiCond)cond);
}

void Core::Wrapper::WrapperUI::EndDragDropSource()
{
	ImGui::EndDragDropSource();
}

bool Core::Wrapper::WrapperUI::BeginDragDropTarget()
{
	return ImGui::BeginDragDropTarget();
}

const Payload* Core::Wrapper::WrapperUI::AcceptDragDropPayload(const char* type, DragDropFlags flags /*= DragDropFlags::None*/)
{
	return (Payload*)ImGui::AcceptDragDropPayload(type, (ImGuiDragDropFlags)flags);
}

void Core::Wrapper::WrapperUI::EndDragDropTarget()
{
	return ImGui::EndDragDropTarget();
}

const Payload* Core::Wrapper::WrapperUI::GetDragDropPayload()
{
	return (Payload*)ImGui::GetDragDropPayload();
}

void Core::Wrapper::WrapperUI::BeginDisabled(bool disabled /*= true*/)
{
	return ImGui::BeginDisabled(disabled);
}

void Core::Wrapper::WrapperUI::EndDisabled()
{
	ImGui::EndDisabled();
}

void Core::Wrapper::WrapperUI::SetItemDefaultFocus()
{
	ImGui::SetItemDefaultFocus();
}

void Core::Wrapper::WrapperUI::SetKeyboardFocusHere(int offset /*= 0*/)
{
	ImGui::SetKeyboardFocusHere(offset);
}

bool Core::Wrapper::WrapperUI::IsItemHovered(HoveredFlags flags /*= HoveredFlags::None*/)
{
	return ImGui::IsItemHovered((ImGuiHoveredFlags)flags);
}

bool Core::Wrapper::WrapperUI::IsItemActive()
{
	return ImGui::IsItemActive();
}

bool Core::Wrapper::WrapperUI::IsItemFocused()
{
	return ImGui::IsItemFocused();
}

bool Core::Wrapper::WrapperUI::IsItemClicked(MouseButton mouse_button /*= MouseButton::Left*/)
{
	return ImGui::IsItemClicked((ImGuiMouseButton)mouse_button);
}

bool Core::Wrapper::WrapperUI::IsItemVisible()
{
	return ImGui::IsItemVisible();
}

bool Core::Wrapper::WrapperUI::IsItemEdited()
{
	return ImGui::IsItemEdited();
}

bool Core::Wrapper::WrapperUI::IsItemActivated()
{
	return ImGui::IsItemActivated();
}

bool Core::Wrapper::WrapperUI::IsItemDeactivated()
{
	return ImGui::IsItemDeactivated();
}

bool Core::Wrapper::WrapperUI::IsItemDeactivatedAfterEdit()
{
	return ImGui::IsItemDeactivatedAfterEdit();
}

bool Core::Wrapper::WrapperUI::IsItemToggledOpen()
{
	return ImGui::IsItemToggledOpen();
}

bool Core::Wrapper::WrapperUI::IsAnyItemHovered()
{
	return ImGui::IsAnyItemHovered();
}

bool Core::Wrapper::WrapperUI::IsAnyItemActive()
{
	return ImGui::IsAnyItemActive();
}

bool Core::Wrapper::WrapperUI::IsAnyItemFocused()
{
	return ImGui::IsAnyItemFocused();
}

uint32_t Core::Wrapper::WrapperUI::GetItemID()
{
	return ImGui::GetItemID();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetItemRectMin()
{
	return ImGui::GetItemRectMin();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetItemRectMax()
{
	return ImGui::GetItemRectMax();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetItemRectSize()
{
	return ImGui::GetItemRectSize();
}

void Core::Wrapper::WrapperUI::SetItemAllowOverlap()
{
	ImGui::SetItemAllowOverlap();
}

double Core::Wrapper::WrapperUI::GetTime()
{
	return ImGui::GetTime();
}

int Core::Wrapper::WrapperUI::GetFrameCount()
{
	return ImGui::GetFrameCount();
}

Math::Vector4 Core::Wrapper::WrapperUI::ColorConvertU32ToFloat4(uint32_t in)
{
	return ImGui::ColorConvertU32ToFloat4(in);
}

uint32_t Core::Wrapper::WrapperUI::ColorConvertFloat4ToU32(const Math::Vector4& in)
{
	return ImGui::ColorConvertFloat4ToU32(in);
}

void Core::Wrapper::WrapperUI::ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v)
{
	ImGui::ColorConvertRGBtoHSV(r, g, b, out_h, out_s, out_v);
}

void Core::Wrapper::WrapperUI::ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b)
{
	ImGui::ColorConvertHSVtoRGB(h, s, v, out_r, out_g, out_b);
}

bool Core::Wrapper::WrapperUI::IsKeyDown(Key key)
{
	return ImGui::IsKeyDown((ImGuiKey)key);
}

bool Core::Wrapper::WrapperUI::IsKeyPressed(Key key, bool repeat /*= true*/)
{
	return ImGui::IsKeyPressed((ImGuiKey)key, repeat);
}

bool Core::Wrapper::WrapperUI::IsKeyReleased(Key key)
{
	return ImGui::IsKeyReleased((ImGuiKey)key);
}

int Core::Wrapper::WrapperUI::GetKeyPressedAmount(Key key, float repeat_delay, float rate)
{
	return ImGui::GetKeyPressedAmount((ImGuiKey)key, repeat_delay, rate);
}

const char* Core::Wrapper::WrapperUI::GetKeyName(Key key)
{
	return ImGui::GetKeyName((ImGuiKey)key);
}

void Core::Wrapper::WrapperUI::SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard)
{
	ImGui::SetNextFrameWantCaptureKeyboard(want_capture_keyboard);
}

bool Core::Wrapper::WrapperUI::IsMouseDown(MouseButton button)
{
	return ImGui::IsMouseDown((ImGuiMouseButton)button);
}

bool Core::Wrapper::WrapperUI::IsMouseClicked(MouseButton button, bool repeat /*= false*/)
{
	return ImGui::IsMouseClicked((ImGuiMouseButton)button, repeat);
}

bool Core::Wrapper::WrapperUI::IsMouseReleased(MouseButton button)
{
	return ImGui::IsMouseReleased((ImGuiMouseButton)button);
}

bool Core::Wrapper::WrapperUI::IsMouseDoubleClicked(MouseButton button)
{
	return ImGui::IsMouseDoubleClicked((ImGuiMouseButton)button);
}

int Core::Wrapper::WrapperUI::GetMouseClickedCount(MouseButton button)
{
	return ImGui::GetMouseClickedCount((ImGuiMouseButton)button);
}

bool Core::Wrapper::WrapperUI::IsMouseHoveringRect(const Math::Vector2& r_min, const Math::Vector2& r_max, bool clip /*= true*/)
{
	return ImGui::IsMouseHoveringRect(r_min, r_max, clip);
}

bool Core::Wrapper::WrapperUI::IsMousePosValid(const Math::Vector2* mouse_pos /*= NULL*/)
{
	return ImGui::IsMousePosValid((const ImVec2*)mouse_pos);
}

bool Core::Wrapper::WrapperUI::IsAnyMouseDown()
{
	return ImGui::IsAnyMouseDown();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetMousePos()
{
	return ImGui::GetMousePos();
}

Math::Vector2 Core::Wrapper::WrapperUI::GetMousePosOnOpeningCurrentPopup()
{
	return ImGui::GetMousePosOnOpeningCurrentPopup();
}

bool Core::Wrapper::WrapperUI::IsMouseDragging(MouseButton button, float lock_threshold /*= -1.0f*/)
{
	return ImGui::IsMouseDragging((ImGuiMouseButton)button, lock_threshold);
}

Math::Vector2 Core::Wrapper::WrapperUI::GetMouseDragDelta(MouseButton button /*= 0*/, float lock_threshold /*= -1.0f*/)
{
	return ImGui::GetMouseDragDelta((ImGuiMouseButton)button, lock_threshold);
}

void Core::Wrapper::WrapperUI::ResetMouseDragDelta(MouseButton button /*= 0*/)
{
	ImGui::ResetMouseDragDelta((ImGuiMouseButton)button);
}

MouseCursor Core::Wrapper::WrapperUI::GetMouseCursor()
{
	return (MouseCursor)ImGui::GetMouseCursor();
}

void Core::Wrapper::WrapperUI::SetMouseCursor(MouseCursor cursor_type)
{
	ImGui::SetMouseCursor((ImGuiMouseCursor)cursor_type);
}

void Core::Wrapper::WrapperUI::SetNextFrameWantCaptureMouse(bool want_capture_mouse)
{
	ImGui::SetNextFrameWantCaptureMouse(want_capture_mouse);
}

const char* Core::Wrapper::WrapperUI::GetClipboardText()
{
	return ImGui::GetClipboardText();
}

void Core::Wrapper::WrapperUI::SetClipboardText(const char* text)
{

	return ImGui::SetClipboardText(text);
}

Math::Vector2 Core::Wrapper::WrapperUI::GetItemSpacing()
{
	return ImGui::GetStyle().ItemSpacing;
}

void Core::Wrapper::WrapperUI::ShowMainDocking()
{
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;
	ImGui::GetWindowDockID();

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f)); // Set window background to red
	ImGui::Begin("DockSpace", (bool*)true, window_flags);
	ImGui::PopStyleColor();
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	ImGui::End();
}

bool Core::Wrapper::WrapperUI::DrawVec3Control(const std::string& label, float* values, float resetValue /*= 0.0f*/, bool lockButton /*= false*/, float columnWidth /*= 100.0f*/)
{
	static bool _lock = false;
	bool stillEditing = false;
	if (lockButton && _lock)
	{
		float value = values[0];
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth() * 3 - 15.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		// X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values[0] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		// y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values[0] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		// Z
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values[0] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
			stillEditing = true;
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		if (value != values[0]) {
			values[1] = values[0];
			values[2] = values[0];
		}
		ImGui::Columns(1);
	}
	else
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::TextUnformatted(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values[0] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f"))
			stillEditing = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values[1] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f"))
			stillEditing = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values[2] = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f"))
			stillEditing = true;;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);
	}
	if (lockButton) {
		ImGui::SameLine();
		ImGui::Checkbox("Lock", &_lock);
	}

	ImGui::PopID();
	return stillEditing;
}

void Core::Wrapper::WrapperUI::SeparatorEx(SeparatorFlags flags)
{
	ImGui::SeparatorEx((ImGuiSeparatorFlags)flags);
}

bool Core::Wrapper::WrapperUI::ChangeTextureButton(Resources::Texture*& texture, const std::string& buttonName /*= "Texture"*/)
{
	WrapperUI::PushID(buttonName.c_str());
	if (WrapperUI::Button(buttonName.c_str()))
	{
		WrapperUI::OpenPopup("TexturePopup");
	}
	if (WrapperUI::IsItemHovered() && texture)
	{
		WrapperUI::BeginTooltip();
		WrapperUI::Image(texture->ID, Math::Vector2(32, 32));
		WrapperUI::EndTooltip();
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(texture ? texture->GetName().c_str() : "None");
	WrapperUI::SameLine();
	bool value = false;
	if (WrapperUI::Button("Reset")) {
		texture = nullptr;
		value = true;
	}
	if (auto tex = Resources::ResourcesManager::Get()->TexturePopup("TexturePopup"))
	{
		texture = tex;
		value = true;
	}
	WrapperUI::PopID();
	return value;
}

void Core::Wrapper::WrapperUI::DrawCanvas(const Math::Vector2 CanvasSize, Core::GameObject* Canvas)
{
	Math::Vector2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
	Math::Vector2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
	if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
	if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
	Math::Vector2 canvas_p1 = Math::Vector2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
	static Math::Vector2 scrolling(0.0f, 0.0f);
	static float zoom = 1.f;

	// Draw border and background color
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
	draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(0, 0, 0, 255));

	// This will catch our interactions
	ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	const bool is_hovered = ImGui::IsItemHovered(); // Hovered
	const bool is_active = ImGui::IsItemActive();   // Held
	const Math::Vector2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
	const Math::Vector2 mouse_pos_in_canvas(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

	// Pan (we use a zero mouse threshold when there's no context menu)
	// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
	if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f))
	{
		scrolling.x += io.MouseDelta.x;
		scrolling.y += io.MouseDelta.y;
	}
	if (ImGui::GetIO().MouseWheel != 0.f && WrapperUI::IsWindowHovered())
	{
		zoom += ImGui::GetIO().MouseWheel * 0.05f;
	}
	zoom = std::clamp(zoom, 0.1f, 10.f);

	// Draw grid
	draw_list->PushClipRect(canvas_p0, canvas_p1, true);
	float GRID_STEP = 64.0f;
	GRID_STEP *= zoom;
	for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
		draw_list->AddLine(Math::Vector2(canvas_p0.x + x, canvas_p0.y), Math::Vector2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
	for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
		draw_list->AddLine(Math::Vector2(canvas_p0.x, canvas_p0.y + y), Math::Vector2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));

	// Draw Objects
	if (Canvas)
		DrawGameObjectOnCanvas(Canvas, origin, zoom);

	// Draw Canvas Rectangle
	draw_list->AddRect(origin, (Math::Vector2(origin) + CanvasSize * zoom), IM_COL32_WHITE);
	draw_list->PopClipRect();
}

bool compareByDepth(const std::shared_ptr<Component::BaseComponent>& obj1, const std::shared_ptr<Component::BaseComponent>& obj2) {
	float depth1 = Arithmetics::Clamp(-(obj1->gameObject->transform->GetWorldPosition().z - (obj1->gameObject->index / 1000.f)) / 10000.f, -1.f, 1.f);
	float depth2 = Arithmetics::Clamp(-(obj2->gameObject->transform->GetWorldPosition().z - (obj2->gameObject->index / 1000.f)) / 10000.f, -1.f, 1.f);
	return depth1 < depth2;
}

void AddComponentsToList(std::vector<std::shared_ptr<Component::BaseComponent>>& components, Core::GameObject* gameObject)
{
	for (auto& component : gameObject->GetComponents())
	{
		if (auto button = dynamic_cast<Component::UI::Button*>(component.get()))
			components.push_back(component);
		if (auto uiImage = dynamic_cast<Component::UI::UIImage*>(component.get()))
			components.push_back(component);
		if (auto text = dynamic_cast<Component::UI::Text*>(component.get()))
			components.push_back(component);
	}
}

void Core::Wrapper::WrapperUI::DrawGameObjectOnCanvas(GameObject* gameObject, const Math::Vector2& origin, float zoom)
{
	std::vector<std::shared_ptr<Component::BaseComponent>> components;
	AddComponentsToList(components, gameObject);
	for (auto& child : gameObject->GetAllChildren())
	{
		AddComponentsToList(components, child);
	}
	// Sort the list by depth
	std::sort(components.begin(), components.end(), compareByDepth);

	for (auto& component : components)
	{
		if (auto button = dynamic_cast<Component::UI::Button*>(component.get()))
			DrawButtonOnCanvas(button, origin, zoom);
		if (auto uiImage = dynamic_cast<Component::UI::UIImage*>(component.get()))
			DrawImageOnCanvas(uiImage, origin, zoom);
		if (auto text = dynamic_cast<Component::UI::Text*>(component.get()))
			DrawTextOnCanvas(text, origin, zoom);
	}
}

void Core::Wrapper::WrapperUI::DrawImageOnCanvas(Component::UI::UIImage* image, const Math::Vector2& origin, float zoom)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto pos = origin + image->GetRectTransform()->GetRectangle().Min * zoom;
	auto size = origin + image->GetRectTransform()->GetRectangle().Max * zoom;
	if (image->GetImage())
		draw_list->AddImage(reinterpret_cast<void*>(static_cast<uintptr_t>(image->GetImage()->ID)), pos, size);
	else
		draw_list->AddRectFilled(pos, size, IM_COL32_WHITE);
	DrawAnchor(image->GetRectTransform(), origin, zoom);

}

void Core::Wrapper::WrapperUI::DrawButtonOnCanvas(Component::UI::Button* button, const Math::Vector2& origin, float zoom)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto pos = origin + button->GetRectTransform()->GetRectangle().Min * zoom;
	auto size = origin + button->GetRectTransform()->GetRectangle().Max * zoom;
	if (button->GetTextureNormal())
		draw_list->AddImage(reinterpret_cast<void*>(static_cast<uintptr_t>(button->GetTextureNormal()->ID)), pos, size);
	else {
		draw_list->AddRectFilled(pos, size, ImColor{ button->GetDefaultColor() * 255.f });
	}
	DrawAnchor(button->GetRectTransform(), origin, zoom);
}

void Core::Wrapper::WrapperUI::DrawTextOnCanvas(Component::UI::Text* text, const Math::Vector2& origin, float zoom)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto pos = text->GetRectTransform()->GetFinalPosition();
	pos.y -= -text->GetRectTransform()->GetRectangle().Min.y + text->GetRectTransform()->GetRectangle().Max.y;
	pos = origin + pos * zoom;
	auto font = GetOrAddFont(text->GetFont()->GetFullPath(), text->GetFont()->GetFontSize());
	if (font)
	{
		draw_list->AddText(font, (text->GetFont()->GetFontSize() + 4.f) * text->GetScale() * zoom, pos, ImColor(text->GetColor()), text->GetText().c_str());
	}
	else
	{
		draw_list->AddText(arialFont, (text->GetFont()->GetFontSize() + 4.f) * text->GetScale() * zoom, pos, ImColor(text->GetColor()), text->GetText().c_str());

	}
	/*
	//auto size = origin + text->GetRectTransform()->GetRectangle().Max * zoom;
	//auto characterList = text->GetFont()->GetCharacters();
	std::string::const_iterator c;
	auto content = text->GetText();
	for (c = content.begin(); c != content.end(); c++)
	{
		Character ch = characterList[*c];
		float xpos = pos.x + ch.Bearing.x * text->GetScale();
		float ypos = pos.y - (ch.Size.y - ch.Bearing.y) * text->GetScale();

		float w = ch.Size.x * text->GetScale();
		float h = ch.Size.y * text->GetScale();
		draw_list->AddImage(reinterpret_cast<void*>(static_cast<uintptr_t>(ch.TextureID)), Math::Vector2{ xpos,  ypos}, Math::Vector2{ xpos + w * zoom, ypos + h * zoom });

		pos.x += (ch.Advance >> 6) * text->GetScale() * zoom; // bit shift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
	}
	*/
	DrawAnchor(text->GetRectTransform(), origin, zoom);
}

void Core::Wrapper::WrapperUI::DrawAnchor(Component::UI::RectTransform* transform, const Math::Vector2& origin, float zoom)
{
	if (transform->gameObject->IsSelected() && transform->ShouldDrawAnchor())
	{
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		Math::Vector2 size = transform->GetSize();
		Math::Vector2 center = transform->GetAnchorPosition();
		auto rect = transform->GetRectangle();
		float lenght = 15.f;
		draw_list->AddLine(origin + Math::Vector2{ center.x - lenght, center.y } *zoom, origin + Math::Vector2{ center.x + lenght, center.y } *zoom, IM_COL32_BLACK, 2.5f * zoom);
		draw_list->AddLine(origin + Math::Vector2{ center.x, center.y - lenght } *zoom, origin + Math::Vector2{ center.x, center.y + lenght } *zoom, IM_COL32_BLACK, 2.5f * zoom);
		draw_list->AddRect(origin + rect.Min * zoom, origin + rect.Max * zoom, ImColor(0, 255, 0));;
	}
}

ImFont* Core::Wrapper::WrapperUI::GetOrAddFont(std::string path, int size)
{
	if (fonts.count(path))
	{
		return fonts[path];
	}
	waitingFonts.push_back({ path, size });
	return nullptr;
}

void Core::Wrapper::WrapperUI::PopulateFonts()
{
	for (auto font : waitingFonts)
	{
		auto imFont = ImGui::GetIO().Fonts->AddFontFromFileTTF(font.first.c_str(), (float)font.second);
		fonts[font.first] = imFont;
		ImGui_ImplOpenGL3_CreateFontsTexture();
	}
	waitingFonts.clear();
}

bool Core::Wrapper::WrapperUI::IsWindowVisible()
{
	return ImGui::GetCurrentWindow()->DockTabIsVisible;
}

PANDOR_API void Core::Wrapper::WrapperUI::DrawAnimatorBackGround(Resources::AnimationController* controller, Resources::StateRect*& stateSelected, Resources::Link*& linkSelected)
{
	ImGui::SameLine();
	Math::Vector2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
	Math::Vector2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available
	if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
	if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
	Math::Vector2 canvas_p1 = Math::Vector2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
	static Math::Vector2 scrolling(0.0f, 0.0f);
	static float zoom = 1.f;

	// Draw border and background color
	ImGuiIO& io = ImGui::GetIO();
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
	draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(0, 0, 0, 255));

	// This will catch our interactions
	ImGui::InvisibleButton("animator", canvas_sz, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	const bool is_hovered = ImGui::IsItemHovered(); // Hovered
	const bool is_active = ImGui::IsItemActive();   // Held
	const Math::Vector2 origin(canvas_p0.x + scrolling.x, canvas_p0.y + scrolling.y); // Lock scrolled origin
	const Math::Vector2 mouse_pos_in_canvas((io.MousePos.x - origin.x) / zoom, (io.MousePos.y - origin.y) / zoom);

	// Pan (we use a zero mouse threshold when there's no context menu)
	// You may decide to make that threshold dynamic based on whether the mouse is hovering something etc.
	if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f))
	{
		scrolling.x += io.MouseDelta.x;
		scrolling.y += io.MouseDelta.y;
	}
	if (ImGui::GetIO().MouseWheel != 0.f && WrapperUI::IsWindowHovered())
	{
		zoom += ImGui::GetIO().MouseWheel * 0.05f;
	}
	zoom = std::clamp(zoom, 0.1f, 10.f);

	// Draw grid
	draw_list->PushClipRect(canvas_p0, canvas_p1, true);
	float GRID_STEP = 64.0f;
	GRID_STEP *= zoom;
	for (float x = fmodf(scrolling.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
		draw_list->AddLine(Math::Vector2(canvas_p0.x + x, canvas_p0.y), Math::Vector2(canvas_p0.x + x, canvas_p1.y), IM_COL32(200, 200, 200, 40));
	for (float y = fmodf(scrolling.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
		draw_list->AddLine(Math::Vector2(canvas_p0.x, canvas_p0.y + y), Math::Vector2(canvas_p1.x, canvas_p0.y + y), IM_COL32(200, 200, 200, 40));

	static bool createlink = false;
	static StateRect* linkStart;

	static StateRect* clickedState = nullptr;

	bool openStateRightClick = false;
	bool updateStateSelected = false;

	StateRect* newSelectedState = stateSelected;

	Link* newLinkselected = linkSelected;

	for (auto& link : controller->links)
	{
		WrapperUI::DrawLink(link, origin, zoom, mouse_pos_in_canvas, newLinkselected);
	}

	for (auto& state : controller->states)
	{
		if (WrapperUI::DrawStateRect(state.second, zoom, origin, mouse_pos_in_canvas, newSelectedState)) {
			openStateRightClick = true;
			clickedState = state.second;
		}
	}

	if (newSelectedState != stateSelected)
	{
		stateSelected = newSelectedState;
		updateStateSelected = true;
		linkSelected = nullptr;
	}
	else if (newLinkselected != linkSelected)
	{
		linkSelected = newLinkselected;
		stateSelected = nullptr;
	}


	if (clickedState && openStateRightClick)
	{
		WrapperUI::OpenPopup("StatePopup");
	}

	if (WrapperUI::BeginPopup("StatePopup"))
	{
		if (WrapperUI::Button("Create Link"))
		{
			createlink = true;
			linkStart = clickedState;
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
	else if (IsMouseClicked(MouseButton::Middle) && !openStateRightClick)
	{
		WrapperUI::OpenPopup("RightClick");
	}

	if (WrapperUI::BeginPopup("RightClick"))
	{
		if (WrapperUI::Button("Create New State"))
		{
			controller->AddState("New State", mouse_pos_in_canvas, Math::Vector4(0.970f, 0.469f, 0.0f, 1.f));
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}

	if (createlink && linkStart)
	{
		draw_list->AddLine(origin + linkStart->pos * zoom, io.MousePos, IM_COL32_WHITE);
		if (updateStateSelected)
		{
			controller->AddLink(linkStart, stateSelected);
			createlink = false;
			linkStart = nullptr;
		}
		else if (IsMouseClicked(MouseButton::Left))
		{
			createlink = false;
			linkStart = nullptr;
		}
	}

	if (WrapperUI::IsKeyPressed(Key::Key_Delete) && ImGui::IsWindowFocused())
	{
		if (stateSelected)
		{
			controller->RemoveState(stateSelected);
		}
		else if (linkSelected)
		{
			controller->RemoveLink(linkSelected);
		}
	}


	// Draw Canvas Rectangle
	draw_list->PopClipRect();
}


bool Core::Wrapper::WrapperUI::DrawStateRect(Resources::StateRect* state, float zoom, Math::Vector2 origin, Math::Vector2 mousePos, Resources::StateRect*& selected)
{
	static ImDrawList* draw_list = ImGui::GetWindowDrawList();
	Math::Vector2 stateSize = { 35.f + ImGui::CalcTextSize(state->name.c_str()).x * 2.f, 35.f };
	Math::Vector2 MousePos = WrapperUI::GetMousePos();
	auto newPos = origin + state->pos * zoom;
	Math::Vector2 rectMin = newPos - (stateSize / 2.f) * zoom; // define the minimum corner of the rectangle
	Math::Vector2 rectMax = newPos + (stateSize / 2.f) * zoom; // define the maximum corner of the rectangle

	auto isMouseInsideRect = [&]() {

		// check if the mouse position is within the rectangle
		return MousePos.x >= rectMin.x && MousePos.x <= rectMax.x &&
			MousePos.y >= rectMin.y && MousePos.y <= rectMax.y;
	};

	draw_list->AddRectFilled(rectMin, rectMax, ImColor(state->color.x, state->color.y, state->color.z, state->color.w));
	if (selected == state)
		draw_list->AddRect(rectMin, rectMax, ImColor(0.0f, 0.0f, 1.0f, 1.0f));
	draw_list->AddText(ImGui::GetIO().FontDefault, 24 * zoom, rectMin + Math::Vector2(20.f, 5.f) * zoom, IM_COL32_WHITE, state->name.c_str());

	static Vector2 clickOffset;
	static bool drag = false;
	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && isMouseInsideRect())
	{
		clickOffset = state->pos - mousePos;
		selected = state;
		drag = true;
	}
	if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && selected == state && drag)
	{
		state->pos = mousePos + clickOffset;
	}
	else if (selected == state && drag)
	{
		drag = false;
	}

	if (WrapperUI::IsMouseClicked(MouseButton::Middle) && isMouseInsideRect()) {
		selected = state;
		return true;
	}
	return false;
}

void Core::Wrapper::WrapperUI::DrawAnimatorList(Resources::AnimationController* controller)
{
	ImGui::BeginChild("List", ImVec2(ImGui::GetWindowWidth() * 0.25f, 0), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
	if (controller) {
		if (WrapperUI::Button("Add Parameter"))
		{
			controller->AddParameter("New bool");
		}
		int id = 0;
		bool change = false;
		static std::pair<const std::string, bool>* rightClicked = nullptr;
		for (auto&& param : controller->parameters)
		{
			bool selected = false;
			PushID(id++);
			WrapperUI::BeginGroup();
			WrapperUI::Selectable(param.first.c_str(), &selected, SelectableFlags::AllowItemOverlap);
			WrapperUI::SameLine();
			WrapperUI::SetCursorPosX(WrapperUI::GetCursorPosX() + 50.f + WrapperUI::GetWindowWidth() * 0.1f);
			WrapperUI::SetCursorPosY(WrapperUI::GetCursorPosY() - 2.f);
			if (WrapperUI::Checkbox("##", &param.second))
				controller->parameterUpdated = true;
			WrapperUI::EndGroup();

			if (WrapperUI::IsItemHovered() && WrapperUI::IsMouseClicked(MouseButton::Right))
			{
				change = true;
				rightClicked = &param;
			}
			PopID();
		}
		if (rightClicked)
		{
			if (change)
			{
				WrapperUI::OpenPopup("RightClickParameter");
			}
			if (WrapperUI::BeginPopup("RightClickParameter"))
			{
				char Name[64];
				strcpy_s(Name, 64, rightClicked->first.c_str());
				if (WrapperUI::InputText("Rename", Name, 64, InputTextFlags::EnterReturnsTrue) && Name[0] != '\0')
				{
					controller->RenameParameter(rightClicked->first, Name);
					rightClicked = nullptr;
				}
				if (WrapperUI::Button("Delete"))
				{
					controller->DeleteParameter(rightClicked->first);
					rightClicked = nullptr;
				}
				WrapperUI::EndPopup();
			}
		}
	}

	ImGui::EndChild();

}

bool isPointOnLine(Math::Vector2 point, Math::Vector2 p1, Math::Vector2 p2, float lineWidth) {
	// Find the minimum and maximum x and y coordinates of the line segment
	float minX = std::min(p1.x, p2.x) - lineWidth / 2;
	float maxX = std::max(p1.x, p2.x) + lineWidth / 2;
	float minY = std::min(p1.y, p2.y) - lineWidth / 2;
	float maxY = std::max(p1.y, p2.y) + lineWidth / 2;

	// Check if the point is within the bounding box of the line segment
	if (point.x < minX || point.x > maxX || point.y < minY || point.y > maxY) {
		return false;
	}

	// Calculate the distance between the point and the line segment
	float d = std::abs((p2.y - p1.y) * point.x - (p2.x - p1.x) * point.y + p2.x * p1.y - p2.y * p1.x)
		/ std::sqrtf((p2.y - p1.y) * (p2.y - p1.y) + (p2.x - p1.x) * (p2.x - p1.x));

	// Check if the distance is less than or equal to half the line width
	return d <= lineWidth / 2;
}

void Core::Wrapper::WrapperUI::DrawLink(Resources::Link* link, const Math::Vector2& origin, float zoom, const Math::Vector2& mousePos, Resources::Link*& selected)
{
	static ImDrawList* draw_list = ImGui::GetWindowDrawList();
	auto p1Pos = origin + (link->state1->pos + 10.f) * zoom;
	auto p2Pos = origin + (link->state2->pos - 10.f) * zoom;
	if (selected == link)
		draw_list->AddLine(p1Pos, p2Pos, ImColor(0.f, 0.f, 1.f, 1.f), 10.f * zoom);
	draw_list->AddLine(p1Pos, p2Pos, IM_COL32_WHITE, 5.f * zoom);

	// Calculate midpoint of the line
	auto midpoint = (p1Pos + p2Pos) / 2;

	// Calculate direction of the line
	auto direction = (p2Pos - p1Pos).GetNormalized();

	// Calculate length of the branches (triangles)
	float L = 20.f * zoom;

	// Calculate position of the vertices of the triangles
	auto v1 = midpoint - direction * L / 2 + direction.GetNormal() * L / 2;
	auto v2 = midpoint - direction * L / 2 - direction.GetNormal() * L / 2;

	// Draw the triangles
	if (selected == link)
		draw_list->AddTriangle(v1, midpoint, v2, ImColor(0.f, 0.f, 1.f, 1.f), zoom * 5.f);
	draw_list->AddTriangleFilled(v1, midpoint, v2, IM_COL32_WHITE);

	if (IsMouseClicked(MouseButton::Left) && isPointOnLine(GetMousePos(), p1Pos, p2Pos, 7.5f * zoom))
	{
		selected = link;
	}
}

void Core::Wrapper::WrapperUI::ShowPerformanceWindow(const char* name, bool* open)
{
	static float m_currentTime = 0;
	static std::vector<float> m_fps;
	const float Time = 0.005f;
	if (!*open)
		return;
#ifndef PANDOR_GAME
	if (WrapperUI::Begin(name, open))
#else
	if (WrapperUI::Begin(name, open, WindowFlags::NoDecoration))
#endif
	{
		if (m_currentTime <= 0)
		{
			m_currentTime = Time;
			if (m_fps.size() < 60) {
				m_fps.push_back(1 / WrapperUI::GetDeltaTime());
			}
			else {
				m_fps.erase(m_fps.begin());
				m_fps.push_back(1 / WrapperUI::GetDeltaTime());
			}
		}
		else
		{
			m_currentTime -= WrapperUI::GetDeltaTime();
		}
		WrapperUI::PlotHistogram("##", &m_fps[0], (int)m_fps.size(), 0, NULL, 0.0f, FLT_MAX, Math::Vector2(WrapperUI::GetWindowSize().x, 50));
		WrapperUI::BeginGroup();
		WrapperUI::Text("Fps Min : %.2f", *std::min_element(std::begin(m_fps), std::end(m_fps)));
		WrapperUI::Text("Fps Max : %.2f", *std::max_element(std::begin(m_fps), std::end(m_fps)));
		static float average = 0;
		auto value = fmod((float)WrapperWindow::GetTime(), 1.f);
		if (value <= 0.1f) {
			float sum = 0;
			for (int i = 0; i < m_fps.size(); i++)
			{
				sum += m_fps[i];
			}
			average = sum / m_fps.size();
		}
		WrapperUI::Text("Average Fps : %.2f", average);
		WrapperUI::EndGroup();

		if (WrapperUI::TreeNode("Other Informations "))
		{
			char txt[32];
			snprintf(txt, 32, "OpenGL Version : %s", WrapperRHI::GetVersion());
			WrapperUI::Text(txt);

			snprintf(txt, 32, "Vendor : %s", WrapperRHI::GetVendor());
			WrapperUI::Text(txt);

			snprintf(txt, 32, "GPU : %s", WrapperRHI::GetRenderer());
			WrapperUI::Text(txt);

			WrapperUI::Text("Draw Calls : %d", Core::App::Get().drawCall);
			WrapperUI::Text("Vertice Count : %d", Core::App::Get().verticeCount);
			WrapperUI::Text("Triangle Count : %d", Core::App::Get().triangleCount);
			WrapperUI::TreePop();
		}
	}
	WrapperUI::End();

	Core::App::Get().drawCall = 0;
	Core::App::Get().verticeCount = 0;
	Core::App::Get().triangleCount = 0;
}

InputTextCallbackData::InputTextCallbackData()
{
	memset(this, 0, sizeof(*this));
}

void InputTextCallbackData::DeleteChars(int pos, int bytes_count)
{
	IM_ASSERT(pos + bytes_count <= BufTextLen);
	char* dst = Buf + pos;
	const char* src = Buf + pos + bytes_count;
	while (char c = *src++)
		*dst++ = c;
	*dst = '\0';

	if (CursorPos >= pos + bytes_count)
		CursorPos -= bytes_count;
	else if (CursorPos >= pos)
		CursorPos = pos;
	SelectionStart = SelectionEnd = CursorPos;
	BufDirty = true;
	BufTextLen -= bytes_count;
}

void InputTextCallbackData::InsertChars(int pos, const char* new_text, const char* new_text_end /*= NULL*/)
{
	const bool is_resizable = ((int)Flags & (int)InputTextFlags::CallbackResize) != 0;
	const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
	if (new_text_len + BufTextLen >= BufSize)
	{
		if (!is_resizable)
			return;

		// Contrary to STB_TEXTEDIT_INSERTCHARS() this is working in the UTF8 buffer, hence the mildly similar code (until we remove the U16 buffer altogether!)
		ImGuiContext& g = *GImGui;
		ImGuiInputTextState* edit_state = &g.InputTextState;
		IM_ASSERT(edit_state->ID != 0 && g.ActiveId == edit_state->ID);
		IM_ASSERT(Buf == edit_state->TextA.Data);
		int new_buf_size = BufTextLen + ImClamp(new_text_len * 4, 32, ImMax(256, new_text_len)) + 1;
		edit_state->TextA.reserve(new_buf_size + 1);
		Buf = edit_state->TextA.Data;
		BufSize = edit_state->BufCapacityA = new_buf_size;
	}
#pragma warning(push)
#pragma warning(disable: 26451)
	if (BufTextLen != pos)
		memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
#pragma warning(pop)  
	memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
	Buf[BufTextLen + new_text_len] = '\0';

	if (CursorPos >= pos)
		CursorPos += new_text_len;
	SelectionStart = SelectionEnd = CursorPos;
	BufDirty = true;
	BufTextLen += new_text_len;
}


TextFilter::TextFilter(const char* default_filter /*= ""*/)
{
	InputBuf[0] = 0;
	CountGrep = 0;
	if (default_filter)
	{
		ImStrncpy(InputBuf, default_filter, IM_ARRAYSIZE(InputBuf));
		Build();
	}
}

bool TextFilter::Draw(const char* label /*= "Filter (inc,-exc)"*/, float width /*= 0.0f*/)
{
	if (width != 0.0f)
		ImGui::SetNextItemWidth(width);
	bool value_changed = ImGui::InputText(label, InputBuf, IM_ARRAYSIZE(InputBuf));
	if (value_changed)
		Build();
	return value_changed;
}

bool TextFilter::PassFilter(const char* text, const char* text_end /*= NULL*/) const
{
	if (Filters.empty())
		return true;

	if (text == NULL)
		text = "";

	for (int i = 0; i != Filters.size(); i++)
	{
		const TextRange& f = Filters[i];
		if (f.empty())
			continue;
		if (f.b[0] == '-')
		{
			// Subtract
			if (ImStristr(text, text_end, f.b + 1, f.e) != NULL)
				return false;
		}
		else
		{
			// Grep
			if (ImStristr(text, text_end, f.b, f.e) != NULL)
				return true;
		}
	}

	// Implicit * grep
	if (CountGrep == 0)
		return true;

	return false;
}

void TextFilter::Build()
{
	Filters.resize(0);
	TextRange input_range(InputBuf, InputBuf + strlen(InputBuf));
	input_range.split(',', &Filters);

	CountGrep = 0;
	for (int i = 0; i != Filters.size(); i++)
	{
		TextRange& f = Filters[i];
		while (f.b < f.e && ImCharIsBlankA(f.b[0]))
			f.b++;
		while (f.e > f.b && ImCharIsBlankA(f.e[-1]))
			f.e--;
		if (f.empty())
			continue;
		if (Filters[i].b[0] != '-')
			CountGrep += 1;
	}
}

void TextFilter::TextRange::split(char separator, std::vector<TextRange>* out) const
{
	out->resize(0);
	const char* wb = b;
	const char* we = wb;
	while (we < e)
	{
		if (*we == separator)
		{
			out->push_back(TextRange(wb, we));
			wb = we + 1;
		}
		we++;
	}
	if (wb != we)
		out->push_back(TextRange(wb, we));
}
