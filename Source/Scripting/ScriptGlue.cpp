#include "pch.h"

// The "Glue" between C# and C++
// Here are the definitions of C# functions that require data to be stored in C++

#include "Scripting/ScriptGlue.h"
#include "Scripting/ScriptEngine.h"
#include "Core/GameObject.h"
#include "Core/App.h"
#include "Core/Scene.h"
#include "Core/App.h"
#include "Physic/RaycastHit.h"
#include "Core/SceneManager.h"
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/SceneWindow.h>
#endif
#include "Components/BaseComponent.h"
#include <Components/MeshComponent.h>
#include <Components/BoxCollider.h>
#include <Components/Rigidbody.h>
#include <Components/ScriptComponent.h>
#include <Components/Ui/Text.h>
#include <Components/Ui/Button.h>
#include <Components/Animator.h>
#include <Components/SoundEmitter.h>

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"
#include "mono/metadata/assembly.h"

#include "Debug/Log.h"

#include <map>
#include "mono/jit/jit.h"


using namespace Core::Wrapper;
namespace Scripting
{
	static std::map<MonoType*, std::function<Component::BaseComponent* (Core::GameObject*)>> s_objectGetComponentFuncs;
	static std::map<MonoType*, std::function<Component::BaseComponent* (Core::GameObject*)>> s_objectAddComponentFuncs;

#pragma region General
	static void NativeLog(MonoString* string, int value)
	{
		char* cStr = mono_string_to_utf8(string);
		std::string str(cStr);
		mono_free(cStr);
		std::cout << str << ", " << value << std::endl;
	}

	static void NativeLog_Vector(Vector3* parameter, Vector3* outResult)
	{
		parameter->Print();
		*outResult = parameter->GetNormalized();
	}
#pragma endregion

#pragma region GameObject
	static bool Object_HasComponent(uint64_t objectID, MonoReflectionType* componentType)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		auto it = s_objectGetComponentFuncs.find(managedType);
		Assert(it == s_objectGetComponentFuncs.end());
		return it->second(gameObject) != nullptr;
	}

	static uint64_t Object_GetComponent(uint64_t objectID, MonoReflectionType* componentType)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		auto it = s_objectGetComponentFuncs.find(managedType);
		Assert(it == s_objectGetComponentFuncs.end());

		Component::BaseComponent* component = it->second(gameObject);
		if (component == nullptr)
			return 0;
		return component->uuid;
	}

	static void Object_GetComponents(uint64_t objectID, MonoArray** list)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*list = mono_array_new(mono_get_root_domain(), mono_get_uint64_class(), gameObject->GetChildrens().size());
		if (gameObject) {
			int j = 0;
			for (auto i : gameObject->GetComponents())
			{
				mono_array_set(*list, uint64_t, j, i->uuid);
				j++;
			}
		}
	}

	static uint64_t Object_AddComponent(uint64_t objectID, MonoReflectionType* componentType)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		auto it = s_objectAddComponentFuncs.find(managedType);
		Assert(it == s_objectAddComponentFuncs.end());

		Component::BaseComponent* component = it->second(gameObject);
		if (component == nullptr)
			return 0;
		return component->uuid;
	}

	static MonoObject* Object_GetScriptInstance(uint64_t objectID, MonoObject* scriptObject)
	{
		MonoClass* scriptClass = mono_object_get_class(scriptObject);
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);

		std::vector<Component::ScriptComponent*> scriptComponents = gameObject->GetComponents<Component::ScriptComponent>();
		for (Component::ScriptComponent* scriptComp : scriptComponents)
		{
			MonoClass* monoClass = scriptComp->instance->GetScriptClass()->GetMonoClass();
			if (monoClass == scriptClass)
				return scriptComp->instance->GetManagedObject();
		}
		return nullptr;
	}

	static void Object_Destroy(uint64_t objectID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->RemoveFromParent();
	}

	static uint64_t Object_GetParent(uint64_t objectID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject) {
			if (auto objectParent = gameObject->GetParent()) {
				return objectParent->uuid;
			}
		}
		return 0;
	}

	static uint64_t Object_GetChild(uint64_t objectID, uint32_t id)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject) {
			if (auto objectParent = gameObject->GetChild(id)) {
				return objectParent->uuid;
			}
		}
		return 0;
	}

	static void Object_GetChildren(uint64_t objectID, MonoArray** list)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*list = mono_array_new(mono_get_root_domain(), mono_get_uint64_class(), gameObject->GetChildrens().size());
		if (gameObject) {
			int j = 0;
			for (auto i : gameObject->GetChildrens())
			{
				mono_array_set(*list, uint64_t, j, i->uuid);
				j++;
			}
		}
	}

	static MonoString* Object_GetName(uint64_t objectID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject)
		{
			return mono_string_new(mono_get_root_domain(), gameObject->GetName().c_str());
		}
		return nullptr;
	}

	static void Object_SetName(uint64_t objectID, MonoString* name)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject)
		{
			gameObject->SetName(mono_string_to_utf8(name));
		}
	}

	static void Object_ClearParent(uint64_t objectID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject)
		{
			gameObject->SetParent(gameObject->GetScene()->GetSceneNode());
		}
	}

	static void Object_SetParent(uint64_t objectID, uint64_t parentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		Core::GameObject* parent = ScriptEngine::GetSceneContext()->GetObjectByID(parentID);
		if (gameObject && parent)
		{
			gameObject->SetParent(parent);
		}
	}

	static void Object_SetActive(uint64_t objectID, bool value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject)
		{
			gameObject->SetActive(value);
		}
	}

	static bool Object_GetActive(uint64_t objectID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (gameObject)
		{
			return *gameObject->GetActive();
		}
		return false;
	}
#pragma endregion

#pragma region BaseCompoenent
	static bool Component_GetEnable(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		for (std::shared_ptr<Component::BaseComponent> comp : gameObject->GetComponents())
		{
			if (comp->uuid == componentID)
				return *comp->GetEnable();
		}
		return false;
	}
	static void Component_SetEnable(uint64_t objectID, uint64_t componentID, bool* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		for (std::shared_ptr<Component::BaseComponent> comp : gameObject->GetComponents())
		{
			if (comp->uuid == componentID)
			{
				*comp->GetEnable() = *value;
				return;
			}
		}
	}
	static void Component_Destroy(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		Component::BaseComponent* component = gameObject->GetComponentByID(componentID);
		component->RemoveFromGameObject();
	}
#pragma endregion

#pragma region ScriptCompoenent
	static uint64_t Script_FindObjectByName(MonoString* name)
	{
		char* nameStr = mono_string_to_utf8(name);
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByName(nameStr);
		mono_free(nameStr);

		if (gameObject == nullptr)
			return 0;

		return gameObject->uuid;
	}
	static uint64_t Script_Instantiate(uint64_t objectID, Vector3 position, Quaternion rotation, uint64_t parentID = 0)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		Core::GameObject* parent = ScriptEngine::GetSceneContext()->GetObjectByID(parentID);
		Core::GameObject* newGO = gameObject->Instantiate(parent);
		newGO->transform->SetWorldPosition(position);
		newGO->transform->SetWorldRotation(rotation);
		return newGO->uuid;
	}
#pragma endregion

#pragma region Rigidbody
	static float Rigidbody_GetMass(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			return rb->GetMass();
		return 0;
	}
	static void Rigidbody_SetMass(uint64_t objectID, uint64_t componentID, float* mass)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->mass = *mass;
			rb->SetMass(*mass);
		}
	}
	static void Rigidbody_GetVelocity(uint64_t objectID, uint64_t componentID, Vector3* outVelocity)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			*outVelocity = rb->GetVelocity();
	}
	static void Rigidbody_SetVelocity(uint64_t objectID, uint64_t componentID, Vector3* velocity)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->velocity = *velocity;
			rb->SetVelocity(*velocity);
		}
	}
	static void Rigidbody_GetAngularVelocity(uint64_t objectID, uint64_t componentID, Vector3* outAngularVelocity)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			*outAngularVelocity = rb->GetAngularVelocity();
	}
	static void Rigidbody_SetAngularVelocity(uint64_t objectID, uint64_t componentID, Vector3* angularVelocity)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->angularVelocity = *angularVelocity;
			rb->SetAngularVelocity(*angularVelocity);
		}
	}
	static bool Rigidbody_GetGravity(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			return rb->useGravity;
		return false;
	}
	static void Rigidbody_SetGravity(uint64_t objectID, uint64_t componentID, bool* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->useGravity = *value;
			rb->SetGravityEnable(*value);
		}
	}
	static bool Rigidbody_GetKinematic(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			return rb->isKinematic;
		return false;
	}
	static void Rigidbody_SetKinematic(uint64_t objectID, uint64_t componentID, bool* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->isKinematic = *value;
			rb->SetKinematic(*value);
		}
	}
	static bool Rigidbody_GetRotationX(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
				return rb->fixedRotationX;
		return false;
	}
	static void Rigidbody_SetRotationX(uint64_t objectID, uint64_t componentID, bool* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->fixedRotationX = *value;
			rb->FixRotation(rb->fixedRotationX, rb->fixedRotationY, rb->fixedRotationZ);
		}
	}
	static bool Rigidbody_GetRotationY(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			return rb->fixedRotationY;
		return false;
	}
	static void Rigidbody_SetRotationY(uint64_t objectID, uint64_t componentID, bool* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->fixedRotationY = *value;
			rb->FixRotation(rb->fixedRotationX, rb->fixedRotationY, rb->fixedRotationZ);
		}
	}
	static bool Rigidbody_GetRotationZ(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			return rb->fixedRotationZ;
		return false;
	}
	static void Rigidbody_SetRotationZ(uint64_t objectID, uint64_t componentID, bool* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
		{
			rb->fixedRotationZ = *value;
			rb->FixRotation(rb->fixedRotationX, rb->fixedRotationY, rb->fixedRotationZ);
		}
	}
	static void Rigidbody_AddForce(uint64_t objectID, uint64_t componentID, Vector3* force)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			rb->AddForce(*force);
	}
	static void Rigidbody_AddTorque(uint64_t objectID, uint64_t componentID, Vector3* torque)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Rigidbody* rb = gameObject->GetComponentByID<Component::Rigidbody>(componentID))
			rb->AddTorque(*torque);
	}
#pragma endregion

#pragma region Transform
	static void Transform_GetPosition(uint64_t objectID, Vector3* outPosition)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outPosition = gameObject->transform->GetWorldPosition();
	}
	static void Transform_SetPosition(uint64_t objectID, Vector3* position)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetWorldPosition(*position);
	}

	static void Transform_GetRotation(uint64_t objectID, Quaternion* outRotation)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outRotation = gameObject->transform->GetWorldRotation();
	}
	static void Transform_SetRotation(uint64_t objectID, Quaternion* rotation)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetWorldRotation(*rotation);
	}

	static void Transform_GetEulerAngles(uint64_t objectID, Vector3* outEulerAngles)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outEulerAngles = gameObject->transform->GetWorldRotation().ToEuler();
	}

	static void Transform_SetEulerAngles(uint64_t objectID, Vector3* eulerRotation)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetWorldRotation((eulerRotation)->ToQuaternion());
	}

	static void Transform_GetLocalPosition(uint64_t objectID, Vector3* outLocalPosition)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outLocalPosition = gameObject->transform->GetLocalPosition();
	}
	static void Transform_SetLocalPosition(uint64_t objectID, Vector3* localPosition)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetLocalPosition(*localPosition);
	}

	static void Transform_GetLocalRotation(uint64_t objectID, Quaternion* outLocalRotation)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outLocalRotation = gameObject->transform->GetLocalRotation();
	}
	static void Transform_SetLocalRotation(uint64_t objectID, Quaternion* localRotation)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetLocalRotation(*localRotation);
	}

	static void Transform_GetLocalScale(uint64_t objectID, Vector3* outLocalScale)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outLocalScale = gameObject->transform->GetLocalScale();
	}
	static void Transform_SetLocalScale(uint64_t objectID, Vector3* localScale)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetLocalScale(*localScale);
	}

	static void Transform_GetLocalEulerAngles(uint64_t objectID, Vector3* outLocalEulerAngles)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outLocalEulerAngles = gameObject->transform->GetLocalRotation().ToEuler();
	}
	static void Transform_SetLocalEulerAngles(uint64_t objectID, Vector3* eulerAngles)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->SetLocalRotation(*eulerAngles);
	}

	static void Transform_GetForward(uint64_t objectID, Vector3* outForward)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outForward = gameObject->transform->GetForwardVector();
	}

	static void Transform_GetUp(uint64_t objectID, Vector3* outUp)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outUp = gameObject->transform->GetUpVector();
	}

	static void Transform_GetRight(uint64_t objectID, Vector3* outRight)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		*outRight = gameObject->transform->GetRightVector();
	}

	static void Transform_RotateArround(uint64_t objectID, Vector3 target, Vector3 axis, float angle)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		gameObject->transform->RotateArround(target, axis, angle);
	}
#pragma endregion

#pragma region Input
    static bool Input_IsKeyDown(Key key)
    {
        return WrapperUI::IsKeyDown(key);
    }
    static bool Input_IsKeyPressed(Key key)
    {
        return WrapperUI::IsKeyPressed(key, false);
    }
    static bool Input_IsKeyReleased(Key key)
    {
        return WrapperUI::IsKeyReleased(key);
    }
	static void Input_GetMousePos(Vector2* vec)
	{
		POINT mouse = { (long)0.0 };
		GetCursorPos(&mouse);
		*vec = Vector2((float)mouse.x, (float)mouse.y);
	}
	static void Input_EnableCursor()
	{
		WrapperUI::SetMouseCursor(MouseCursor::Arrow);
	}
	static void Input_DisableCursor()
	{
		WrapperUI::SetMouseCursor(MouseCursor::None);
	}
	static void Input_FixCursor()
	{
		Vector2 windowSize = Core::App::Get().window->GetSize();
		SetCursorPos((int)(windowSize.x * 0.5f), (int)(windowSize.y * 0.5f));
	}
#pragma endregion

#pragma region Debug
	static void Debug_Print(MonoString* log)
	{
		if (!log) {
			PrintLog("");
				return;
		}
		char* logStr = mono_string_to_utf8(log);
		PrintLog(logStr);
	}
	static void Debug_PrintWarning(MonoString* log)
	{
		if (!log) {
			PrintWarning("");
			return;
		}
		char* logStr = mono_string_to_utf8(log);
		PrintWarning(logStr);
	}
	static void Debug_PrintError(MonoString* log)
	{
		if (!log) {
			PrintError("");
			return;
		}
		char* logStr = mono_string_to_utf8(log);
		PrintError(logStr);
	}
#pragma endregion

#pragma region Physic

	static bool Physic_Raycast(const Vector3& origin, const Vector3& direction, float distanceMax,
		bool* hit, Vector3* position, Vector3* normal, MonoObject* collider, uint32_t* distance, float* faceIndex)
	{
		Physic::RaycastHit out;
		bool value = Core::App::Get().physic->RayCast(origin, direction, distanceMax, out);
		*hit = out.Hit();
		if (value)
		{
			*distance = out.GetDistance();
			*normal = out.GetNormal();
			MonoObject* instance = ScriptEngine::CreateComponentInstance(out.GetCollider()->gameObject->uuid, out.GetCollider()->uuid);
			collider = instance;
			*position = out.GetPosition();
			*faceIndex = out.GetFaceIndex();
		}
		return value;
	}
#pragma endregion

#pragma region Text
	static MonoString* Text_GetText(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Text* text = gameObject->GetComponentByID<Component::UI::Text>(componentID))
			return mono_string_new(mono_get_root_domain(), text->GetText().c_str());
		return nullptr;
	}

	static void Text_SetText(uint64_t objectID, uint64_t componentID, MonoString* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Text* text = gameObject->GetComponentByID<Component::UI::Text>(componentID))
			text->SetText(mono_string_to_utf8(value));
	}

	static float Text_GetScale(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Text* text = gameObject->GetComponentByID<Component::UI::Text>(componentID))
			return text->GetScale();
		return 0.f;
	}

	static void Text_SetScale(uint64_t objectID, uint64_t componentID, float* value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Text* text = gameObject->GetComponentByID<Component::UI::Text>(componentID))
			text->SetScale(*value);
	}

	static void Text_GetColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Text* text = gameObject->GetComponentByID<Component::UI::Text>(componentID))
			*color = text->GetColor();
	}

	static void Text_SetColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Text* text = gameObject->GetComponentByID<Component::UI::Text>(componentID))
			text->SetColor(*color);
	}
#pragma endregion

#pragma region Button
	static void Button_GetDefaultColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Button* button = gameObject->GetComponentByID<Component::UI::Button>(componentID))
			*color = button->GetDefaultColor();
	}

	static void Button_SetDefaultColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Button* button = gameObject->GetComponentByID<Component::UI::Button>(componentID))
			button->SetDefaultColor(*color);
	}
	static void Button_GetHighlightedColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Button* button = gameObject->GetComponentByID<Component::UI::Button>(componentID))
			*color = button->GetHighlightedColor();
	}

	static void Button_SetHighlightedColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Button* button = gameObject->GetComponentByID<Component::UI::Button>(componentID))
			button->SetHighlightedColor(*color);
	}
	static void Button_GetPressedColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Button* button = gameObject->GetComponentByID<Component::UI::Button>(componentID))
			*color = button->GetPressedColor();
	}

	static void Button_SetPressedColor(uint64_t objectID, uint64_t componentID, Vector4* color)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::UI::Button* button = gameObject->GetComponentByID<Component::UI::Button>(componentID))
			button->SetPressedColor(*color);
	}
#pragma endregion

#pragma region SceneManager
	static bool SceneManager_LoadScene(MonoString* name)
	{
		char* nameStr = mono_string_to_utf8(name);

		Core::Scene* scene = Core::App::Get().sceneManager->GetSceneWithPath(nameStr);
		if (scene)
		{
			Core::App::Get().sceneManager->SetSwitchSceneInGame(scene);
			return true;
		}
		Core::Scene* newScene = Core::App::Get().sceneManager->CreateSceneFromPath(nameStr);
		if (newScene)
		{
			Core::App::Get().sceneManager->SetSwitchSceneInGame(newScene);
			return true;
		}
		return false;
	}
#pragma endregion

#pragma region Animator
	static void Animator_SetBoolean(uint64_t objectID, uint64_t componentID, MonoString* name, bool value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::Animator* animator = gameObject->GetComponentByID<Component::Animator>(componentID))
		{
			char* nameStr = mono_string_to_utf8(name);
			animator->SetBoolean(nameStr, value);
			return;
		}
	}
#pragma endregion
#pragma region SoundEmitter

	static void SoundEmitter_Play(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::SoundEmitter* soundEmitter = gameObject->GetComponentByID<Component::SoundEmitter>(componentID))
			soundEmitter->Play();
	}
	static void SoundEmitter_Stop(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::SoundEmitter* soundEmitter = gameObject->GetComponentByID<Component::SoundEmitter>(componentID))
			soundEmitter->Stop();
	}
	static void SoundEmitter_Pause(uint64_t objectID, uint64_t componentID)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::SoundEmitter* soundEmitter = gameObject->GetComponentByID<Component::SoundEmitter>(componentID))
			soundEmitter->Pause();
	}
	static void SoundEmitter_FadeIn(uint64_t objectID, uint64_t componentID, int millisecondsFading)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::SoundEmitter* soundEmitter = gameObject->GetComponentByID<Component::SoundEmitter>(componentID))
			soundEmitter->FadeIn(millisecondsFading);
	}
	static void SoundEmitter_SetLooping(uint64_t objectID, uint64_t componentID, bool value)
	{
		Core::GameObject* gameObject = ScriptEngine::GetSceneContext()->GetObjectByID(objectID);
		if (Component::SoundEmitter* soundEmitter = gameObject->GetComponentByID<Component::SoundEmitter>(componentID))
			soundEmitter->SetLooping(value);
	}

#pragma endregion

#pragma region AudioManager
	static void AudioManager_PlaySoundByName(MonoString* name)
	{
		char* nameStr = mono_string_to_utf8(name);
		Core::App::Get().audioManager->PlaySoundFromName(Resources::ResourcesManager::AssetPath() + nameStr);
	}
#pragma endregion

#pragma region Application
	static void Application_QuitRequest()
	{
		Core::App::Get().CloseApp();
	}
	static void Application_SetTimeScale(float value)
	{
		Core::App::Get().timeScale = value;
	}
	static float Application_GetTimeScale()
	{
		return Core::App::Get().timeScale;
	}
#pragma endregion

	template<typename C>
	static void RegisterComponent()
	{
		std::string_view typeName = typeid(C).name();
		size_t pos = typeName.find_last_of(':');
		std::string className = "Pandor.";
		className += typeName.substr(pos + 1);

		MonoType* monoType = mono_reflection_type_from_name(className.data(), ScriptEngine::GetCoreAssemblyImage());
		if (monoType == nullptr)
		{
			PrintError("Could not find component type [%s] in C#", className.c_str());
			return;
		}

		s_objectGetComponentFuncs[monoType] = [](Core::GameObject* object) { return object->GetComponent<C>(); };
		s_objectAddComponentFuncs[monoType] = [](Core::GameObject* object) { return object->AddComponent<C>(); };
	}

	// Add every components usable in C#
	void ScriptGlue::RegisterComponents()
	{
		s_objectGetComponentFuncs.clear();
		RegisterComponent<Component::ScriptComponent>();
		RegisterComponent<Component::MeshComponent>();
		RegisterComponent<Component::BoxCollider>();
		RegisterComponent<Component::Collider>();
		RegisterComponent<Component::Rigidbody>();
		RegisterComponent<Component::UI::Text>();
		RegisterComponent<Component::UI::Button>();
		RegisterComponent<Component::SoundEmitter>();
	}

#define ADD_INTERNAL_CALL(Name) mono_add_internal_call("Pandor.InternalCalls::" #Name, Name)

	void ScriptGlue::RegisterFunctions()
	{
		ADD_INTERNAL_CALL(NativeLog);
		ADD_INTERNAL_CALL(NativeLog_Vector);

		ADD_INTERNAL_CALL(Object_HasComponent);
		ADD_INTERNAL_CALL(Object_GetComponent);
		ADD_INTERNAL_CALL(Object_GetComponents);
		ADD_INTERNAL_CALL(Object_AddComponent);
		ADD_INTERNAL_CALL(Object_GetScriptInstance);
		ADD_INTERNAL_CALL(Object_Destroy);
		ADD_INTERNAL_CALL(Object_GetParent);
		ADD_INTERNAL_CALL(Object_GetChild);
		ADD_INTERNAL_CALL(Object_GetChildren);
		ADD_INTERNAL_CALL(Object_GetName);
		ADD_INTERNAL_CALL(Object_SetName);
		ADD_INTERNAL_CALL(Object_ClearParent);
		ADD_INTERNAL_CALL(Object_SetParent);
		ADD_INTERNAL_CALL(Object_GetActive);
		ADD_INTERNAL_CALL(Object_SetActive);

		ADD_INTERNAL_CALL(Component_GetEnable);
		ADD_INTERNAL_CALL(Component_SetEnable);
		ADD_INTERNAL_CALL(Component_Destroy);

		ADD_INTERNAL_CALL(Script_FindObjectByName);
		ADD_INTERNAL_CALL(Script_Instantiate);

		ADD_INTERNAL_CALL(Rigidbody_GetMass);
		ADD_INTERNAL_CALL(Rigidbody_SetMass);
		ADD_INTERNAL_CALL(Rigidbody_GetVelocity);
		ADD_INTERNAL_CALL(Rigidbody_SetVelocity);
		ADD_INTERNAL_CALL(Rigidbody_GetAngularVelocity);
		ADD_INTERNAL_CALL(Rigidbody_SetAngularVelocity);
		ADD_INTERNAL_CALL(Rigidbody_GetGravity);
		ADD_INTERNAL_CALL(Rigidbody_SetGravity);
		ADD_INTERNAL_CALL(Rigidbody_GetKinematic);
		ADD_INTERNAL_CALL(Rigidbody_SetKinematic);
		ADD_INTERNAL_CALL(Rigidbody_GetRotationX);
		ADD_INTERNAL_CALL(Rigidbody_SetRotationX);
		ADD_INTERNAL_CALL(Rigidbody_GetRotationY);
		ADD_INTERNAL_CALL(Rigidbody_SetRotationY);
		ADD_INTERNAL_CALL(Rigidbody_GetRotationZ);
		ADD_INTERNAL_CALL(Rigidbody_SetRotationZ);
		ADD_INTERNAL_CALL(Rigidbody_AddForce);
		ADD_INTERNAL_CALL(Rigidbody_AddTorque);

		ADD_INTERNAL_CALL(Transform_GetPosition);
		ADD_INTERNAL_CALL(Transform_SetPosition);
		ADD_INTERNAL_CALL(Transform_GetRotation);
		ADD_INTERNAL_CALL(Transform_SetRotation);
		ADD_INTERNAL_CALL(Transform_GetEulerAngles);
		ADD_INTERNAL_CALL(Transform_SetEulerAngles);
		ADD_INTERNAL_CALL(Transform_GetLocalPosition);
		ADD_INTERNAL_CALL(Transform_SetLocalPosition);
		ADD_INTERNAL_CALL(Transform_GetLocalRotation);
		ADD_INTERNAL_CALL(Transform_SetLocalRotation);
		ADD_INTERNAL_CALL(Transform_GetLocalScale);
		ADD_INTERNAL_CALL(Transform_SetLocalScale);
		ADD_INTERNAL_CALL(Transform_GetLocalEulerAngles);
		ADD_INTERNAL_CALL(Transform_SetLocalEulerAngles);
		ADD_INTERNAL_CALL(Transform_GetForward);
		ADD_INTERNAL_CALL(Transform_GetUp);
		ADD_INTERNAL_CALL(Transform_GetRight);
		ADD_INTERNAL_CALL(Transform_RotateArround);

		ADD_INTERNAL_CALL(Physic_Raycast);

		ADD_INTERNAL_CALL(Text_GetText);
		ADD_INTERNAL_CALL(Text_SetText);
		ADD_INTERNAL_CALL(Text_GetScale);
		ADD_INTERNAL_CALL(Text_SetScale);
		ADD_INTERNAL_CALL(Text_GetColor);
		ADD_INTERNAL_CALL(Text_SetColor);

		ADD_INTERNAL_CALL(Button_GetDefaultColor);
		ADD_INTERNAL_CALL(Button_SetDefaultColor);
		ADD_INTERNAL_CALL(Button_GetHighlightedColor);
		ADD_INTERNAL_CALL(Button_SetHighlightedColor);
		ADD_INTERNAL_CALL(Button_GetPressedColor);
		ADD_INTERNAL_CALL(Button_GetPressedColor);

        ADD_INTERNAL_CALL(Input_IsKeyDown);
        ADD_INTERNAL_CALL(Input_IsKeyPressed);
		ADD_INTERNAL_CALL(Input_IsKeyReleased);
		ADD_INTERNAL_CALL(Input_GetMousePos);
		ADD_INTERNAL_CALL(Input_EnableCursor);
		ADD_INTERNAL_CALL(Input_DisableCursor);
		ADD_INTERNAL_CALL(Input_FixCursor);

		ADD_INTERNAL_CALL(Debug_Print);
		ADD_INTERNAL_CALL(Debug_PrintWarning);
		ADD_INTERNAL_CALL(Debug_PrintError);

		ADD_INTERNAL_CALL(SceneManager_LoadScene);

		ADD_INTERNAL_CALL(Animator_SetBoolean);

		ADD_INTERNAL_CALL(SoundEmitter_Play);
		ADD_INTERNAL_CALL(SoundEmitter_Stop);
		ADD_INTERNAL_CALL(SoundEmitter_Pause);
		ADD_INTERNAL_CALL(SoundEmitter_FadeIn);
		ADD_INTERNAL_CALL(SoundEmitter_SetLooping);

		ADD_INTERNAL_CALL(AudioManager_PlaySoundByName);

		ADD_INTERNAL_CALL(Application_QuitRequest);
		ADD_INTERNAL_CALL(Application_GetTimeScale);
		ADD_INTERNAL_CALL(Application_SetTimeScale);
	}

}