#pragma once
#include "PandorAPI.h"
#include <filesystem>

#include <unordered_map>
#include <string>
#include <memory>
#include <map>

extern "C" 
{ 
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoType MonoType;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
}

namespace Core
{
	class Scene;
	class GameObject;
}

namespace Component
{
	class BaseComponent;
	class Collider;
	class ScriptComponent;
}

namespace Physic
{
	enum class  CollisionType
	{
		ENTER = 0,
		STAY, 
		EXIT,
	};
}

namespace Scripting
{
	class ScriptClass;

	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		Vector2, Vector3, Vector4,
		GameObject, ScriptComponent,
		Rigidbody, Text, Button,
		Animator, SoundEmitter, BoxCollider
	};

	struct ScriptField
	{
		ScriptFieldType type;
		std::string name;

		MonoClassField* classField;
	};

	// ScriptField + data storage
	struct EditorField
	{
		ScriptField field;

		EditorField()
		{
			memset(m_buffer, 0, sizeof(m_buffer));
		}

		template<typename T>
		T GetValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_buffer;
		}

		template<typename T>
		void GetValues(T& a, T& b) const
		{
			static_assert(sizeof(T) <= 8, "Type too large!");
			memcpy(&a, m_buffer            , sizeof(T));
			memcpy(&b, m_buffer + sizeof(T), sizeof(T));
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_buffer, &value, sizeof(T));
		}

		template<typename T>
		void SetValues(T a, T b)
		{
			static_assert(sizeof(T) <= 8, "Type too large!");
			memcpy(m_buffer            , &a, sizeof(T));
			memcpy(m_buffer + sizeof(T), &b, sizeof(T));
		}

	public :
		uint8_t* GetBuffer() { return m_buffer; }

	private:
		uint8_t m_buffer[16];

		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	class PANDOR_API ScriptClass
	{
	private:
		std::string m_classNamespace;
		std::string m_className;

		std::unordered_map<std::string, ScriptField> m_fields;

		MonoClass* m_monoClass = nullptr;

		friend class ScriptEngine;

	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		std::vector<MonoMethod*> GetMethods();
		std::string GetMethodName(MonoMethod* method);
		bool InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		void InvokeMethodVoid(MonoObject* instance, MonoMethod* method);

		const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_fields; }
		MonoClass* GetMonoClass() const { return m_monoClass; }
		std::string GetName() const { return m_className; }
	};

	class PANDOR_API ScriptInstance
	{
	private:
		std::shared_ptr<ScriptClass> m_scriptClass;
		MonoObject* m_instance;
		MonoMethod* m_constructor= nullptr;
		MonoMethod* m_onCreateMethod = nullptr;
		MonoMethod* m_onUpdateMethod = nullptr;

		inline static uint8_t s_fieldValueBuffer[16];

		friend class ScriptEngine;

	private:
		bool GetFieldValueInternal(const std::string& name);
		bool SetFieldValueInternal(const std::string& name, const void* value);

	public:
		ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, uint64_t objectID, uint64_t componentID);

		void InvokeOnCreate();
		void InvokeOnUpdate(float dt);

		void InvokeOnCollision(Physic::CollisionType type, Component::Collider* collider);
		void InvokeOnTrigger(Physic::CollisionType type, Component::Collider* collider);

		std::shared_ptr<ScriptClass> GetScriptClass() { return m_scriptClass; }
		MonoObject* GetManagedObject() { return m_instance; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			if (!GetFieldValueInternal(name))
				return T();

			return *(T*)s_fieldValueBuffer;
		}

		uint64_t GetFieldObject(const std::string& name);
		std::pair<uint64_t, uint64_t> GetFieldComponent(const std::string& name);

		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			SetFieldValueInternal(name, &value);
		}

		void SetFieldObject(Component::ScriptComponent& scriptComp, const std::string& name, const uint64_t& objectID);
		void SetFieldComponent(Component::ScriptComponent& scriptComp, const std::string& name, const uint64_t& objectID, const uint64_t& compID);

	};

	class PANDOR_API ScriptEngine
	{
	private:
		static void InitMono();
		static void ShutdownMono();

		static void BuildSolution();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyData();
		static void LoadAssemblyClasses();

		static Component::ScriptComponent* GetScriptFromTypeInternal(const Core::GameObject* gameObject, const MonoType* type, const MonoClass* fieldClass);

		friend class ScriptClass;
		friend class ScriptGlue;

	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadGameAssembly(const std::filesystem::path& filepath);

		static void ReloadAssembly();

		static void OnStart(Core::Scene* scene);
		static void OnStop();

		static void CreateScriptFile(const std::string& scriptName);
		static void OpenSolution(const std::string& scriptFilePath, const std::string& scriptName);

		static bool ScriptClassExists(std::string& className);
		static void CreateScrpitInstance(Component::ScriptComponent& component);
		static void DestroyScrpitInstance(Component::ScriptComponent& component);
		static void OnCreateScript(Component::ScriptComponent& scriptComponent);
		static void OnUpdateScript(Component::ScriptComponent& component, float dt);
		static void UpdateObjectRefs();
		static void UpdateComponentRefs();
		static bool IsScriptNameAvaliable(const char* scriptName);

		static MonoObject* CreateComponentInstance(uint64_t objectID, uint64_t componentID);
		static Core::Scene* GetSceneContext();
		static std::shared_ptr<ScriptClass> GetScriptClass(const std::string& name);
		static MonoImage* GetCoreAssemblyImage();
		static void GetFieldValue(ScriptClass& scriptClass, MonoClassField* classField, const void* value);
		static std::string GetFieldTypeName(const ScriptField& scriptField);
		static Component::ScriptComponent* GetScriptFromType(const Core::GameObject* gameObject, const ScriptField& scriptField);
		static std::vector<Component::ScriptComponent*> GetScriptsFromType(const Core::GameObject* gameObject, const ScriptField& scriptField);
		static std::string GetMethodName(MonoMethod* method);

		static std::map<std::string, std::shared_ptr<ScriptClass>> GetScriptClasses();
	};

	namespace Helper {

		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
			case ScriptFieldType::None:				return "None";
			case ScriptFieldType::Float:			return "Float";
			case ScriptFieldType::Double:			return "Double";
			case ScriptFieldType::Bool:				return "Bool";
			case ScriptFieldType::Char:				return "Char";
			case ScriptFieldType::Byte:				return "Byte";
			case ScriptFieldType::Short:			return "Short";
			case ScriptFieldType::Int:				return "Int";
			case ScriptFieldType::Long:				return "Long";
			case ScriptFieldType::UByte:			return "UByte";
			case ScriptFieldType::UShort:			return "UShort";
			case ScriptFieldType::UInt:				return "UInt";
			case ScriptFieldType::ULong:			return "ULong";
			case ScriptFieldType::Vector2:			return "Vector2";
			case ScriptFieldType::Vector3:			return "Vector3";
			case ScriptFieldType::Vector4:			return "Vector4";
			case ScriptFieldType::GameObject:		return "GameObject";
			case ScriptFieldType::ScriptComponent:  return "ScriptComponent";
			case ScriptFieldType::Rigidbody:		return "Rigidbody";
			case ScriptFieldType::Text:				return "Text";
			case ScriptFieldType::Button:			return "Button";
			case ScriptFieldType::Animator:			return "Animator";
			case ScriptFieldType::SoundEmitter:		return "SoundEmitter";
			case ScriptFieldType::BoxCollider:			return "BoxCollider";
			}
			PrintError("Unknown ScriptFieldType");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")				return ScriptFieldType::None;
			if (fieldType == "Float")				return ScriptFieldType::Float;
			if (fieldType == "Double")				return ScriptFieldType::Double;
			if (fieldType == "Bool")				return ScriptFieldType::Bool;
			if (fieldType == "Char")				return ScriptFieldType::Char;
			if (fieldType == "Byte")				return ScriptFieldType::Byte;
			if (fieldType == "Short")				return ScriptFieldType::Short;
			if (fieldType == "Int")					return ScriptFieldType::Int;
			if (fieldType == "Long")				return ScriptFieldType::Long;
			if (fieldType == "UByte")				return ScriptFieldType::UByte;
			if (fieldType == "UShort")				return ScriptFieldType::UShort;
			if (fieldType == "UInt")				return ScriptFieldType::UInt;
			if (fieldType == "ULong")				return ScriptFieldType::ULong;
			if (fieldType == "Vector2")				return ScriptFieldType::Vector2;
			if (fieldType == "Vector3")				return ScriptFieldType::Vector3;
			if (fieldType == "Vector4")				return ScriptFieldType::Vector4;
			if (fieldType == "GameObject")			return ScriptFieldType::GameObject;
			if (fieldType == "ScriptComponent")		return ScriptFieldType::ScriptComponent;
			if (fieldType == "Rigidbody")			return ScriptFieldType::Rigidbody;
			if (fieldType == "Text")				return ScriptFieldType::Text;
			if (fieldType == "Button")				return ScriptFieldType::Button;
			if (fieldType == "Animator")			return ScriptFieldType::Animator;
			if (fieldType == "SoundEmitter")		return ScriptFieldType::SoundEmitter;
			if (fieldType == "BoxCollider")			return ScriptFieldType::BoxCollider;

			PrintError("Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}

	}
}