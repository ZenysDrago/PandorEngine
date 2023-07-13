#pragma once
#include "PandorAPI.h"

#include "UIElement.h"
#include <Math/Maths.h>
namespace Resources
{
	class Texture;
}

namespace Component
{
	class ScriptComponent;
	namespace UI {
		enum class ButtonState
		{
			UP,
			DOWN,
			PRESSED,
			HOVERED
		};

		enum class ButtonMode
		{
			NONE,
			COLOR,
			SPRITE,
		};

		class PANDOR_API Button : public UIElement
		{
		public:
			~Button();

			void Initialize() override;

			void Awake() override;

			void Start() override;

			void GameUpdate() override;

			void Update() override;

			void EditorUpdate() override;

			void ShowInInspector() override;

			std::string GetComponentName() { return "Button"; }

			Component::BaseComponent* Clone() const { return new Button(*this); };

			template <typename F, typename... A> inline void AddOnClickFunction(F&& task, A&&... args)
			{
				std::function<void()> task_function = [task = std::forward<F>(task), args = std::make_tuple(std::forward<A>(args)...)]() mutable {
					std::apply(task, args);
				};
				m_onClickFunctions.push_back(task_function);
			}

			void OnClick();

			Math::Vector4 GetDefaultColor() { return m_defaultColor; }
			void SetDefaultColor(Math::Vector4 val) { m_defaultColor = val; }

			Math::Vector4 GetHighlightedColor() const { return m_highlightedColor; }
			void SetHighlightedColor(Math::Vector4 val) { m_highlightedColor = val; }

			Math::Vector4 GetPressedColor() const { return m_pressedColor; }
			void SetPressedColor(Math::Vector4 val) { m_pressedColor = val; }

			Resources::Texture* GetTextureNormal() { return m_normalTexture; }

			ButtonMode GetMode() { return m_mode; }

			void ReadComponent(std::fstream& sceneFile) override;

			std::ostream& operator<<(std::ostream& os) override;
		private:
			void ChangeState(ButtonState newState);

			ButtonMode m_mode = ButtonMode::COLOR;
			ButtonState m_state = ButtonState::UP;
			std::vector<std::function<void()>> m_onClickFunctions;
			Math::Vector4 m_defaultColor = { 1.f, 1.f, 1.f, 1.f };
			Math::Vector4 m_highlightedColor = { 0.9f, 0.9f, 0.9f, 1.f };
			Math::Vector4 m_pressedColor = { 0.75f, 0.75f, 0.75f, 1.f };
			Resources::Texture* m_normalTexture = nullptr;
			Resources::Texture* m_highlightedTexture = nullptr;
			Resources::Texture* m_pressedTexture = nullptr;

			std::vector<std::tuple<uint64_t, uint64_t, std::string>> m_waitingMethods;

			std::vector<std::tuple<Core::GameObject*, Component::ScriptComponent*, std::string>> m_scriptsRef;
		};
	}
}