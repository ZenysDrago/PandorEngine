#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>
#include <Math/Maths.h>

namespace Core::Wrapper::WrapperRHI {
	class Buffer;
}
namespace Resources
{
	class Material;
}
namespace Component::UI {
	enum class PANDOR_API Anchor
	{
		TOP_LEFT,
		TOP_MIDDLE,
		TOP_RIGHT,
		CENTER_LEFT,
		CENTER_MIDDLE,
		CENTER_RIGHT,
		BOTTOM_LEFT,
		BOTTOM_MIDDLE,
		BOTTOM_RIGHT,
		STRETCH_TOP,
		STRETCH_MIDDLE,
		STRETCH_BOTTOM,
		STRETCH_LEFT,
		STRETCH_CENTER,
		STRETCH_RIGHT,
		STRETCH_ALL
	};

	struct PANDOR_API Rect
	{
		Rect() {}
		Rect(float minx, float miny, float maxx, float maxy)
		{
			Min.x = minx;
			Min.y = miny;
			Max.x = maxx;
			Max.y = maxy;
		}
		Rect(Math::Vector2 mini, Math::Vector2 maxi)
		{
			Min = mini;
			Max = maxi;
		}
		Rect operator/(Vector2 vec)
		{
			Rect rect;
			rect.Min = Min / vec;
			rect.Max = Max / vec;
			return rect;
		};
		Rect operator+(Rect rect)
		{
			return { Min + rect.Min, Max + rect.Max };
		};
		Math::Vector2 Min;
		Math::Vector2 Max;

		void ToViewport(Math::Vector2 screenResolution);
		void ToScreen(Math::Vector2 screenResolution);

		void Print() { std::cout << Min << Max << std::endl; }
	};

	class PANDOR_API RectTransform : public Component::IComponent<Component::UI::RectTransform>
	{
	public:
		~RectTransform();

		void Initialize() override;

		void Update() override;

		void EditorDraw() override;

		void ShowInInspector() override;

		std::string GetComponentName() override { return "RectTransform"; }

		Math::Vector2 GetParentPosition();
		Math::Vector2 GetParentSize();

		Math::Vector2 GetScreenResolution();
		Math::Vector2 GetFinalPosition();
		Math::Vector2 GetSize();
		Math::Vector2 GetAnchorPosition();
		Math::Vector2 GetPivot();

		void SetSize(Math::Vector2 _size) { m_size = _size; }

		Rect GetRectangle();
		bool ShouldDrawAnchor() const { return m_drawAnchor; }

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;
	private:
		void OnChangeAnchor();
		Rect m_rect;
		Anchor m_anchor = Anchor::TOP_LEFT;
		Math::Vector2 m_position = { 0, 0 };
		Math::Vector2 m_anchorPosition = { 0, 0 };
		Math::Vector2 m_pivot = { 0.5f, 0.5f };
		Math::Vector2 m_size = { 100, 100 };
		bool m_drawAnchor = true;
	};
}

