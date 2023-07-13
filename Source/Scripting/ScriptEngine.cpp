#include "pch.h"
#include "Scripting/ScriptEngine.h"
#include "Debug/log.h"
#include <Core/App.h>

#include "Scripting/ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/threads.h"
#include "mono/metadata/exception.h"
#include "mono/metadata/image.h"
#include "mono/metadata/debug-helpers.h"

#include "Core/Scene.h"
#include "Core/SceneManager.h"
#include "Core/GameObject.h"
#include "Components/ScriptComponent.h"
#include "Components/Collider.h"

#include "filewatch/FileWatch.h"

#include <Windows.h>
#include <string>

namespace Scripting
{
	static std::map<std::string, ScriptFieldType> s_scriptFieldTypeMap =
	{
		{ "System.Single", ScriptFieldType::Float },
		{ "System.Double", ScriptFieldType::Double },
		{ "System.Boolean", ScriptFieldType::Bool },
		{ "System.Char", ScriptFieldType::Char },
		{ "System.Int16", ScriptFieldType::Short },
		{ "System.Int32", ScriptFieldType::Int },
		{ "System.Int64", ScriptFieldType::Long },
		{ "System.Byte", ScriptFieldType::Byte },
		{ "System.UInt16", ScriptFieldType::UShort },
		{ "System.UInt32", ScriptFieldType::UInt },
		{ "System.UInt64", ScriptFieldType::ULong },

		{ "Pandor.Vector2", ScriptFieldType::Vector2 },
		{ "Pandor.Vector3", ScriptFieldType::Vector3 },
		{ "Pandor.Vector4", ScriptFieldType::Vector4 },

		{ "Pandor.GameObject", ScriptFieldType::GameObject },
		{ "Pandor.ScriptComponent", ScriptFieldType::ScriptComponent },
		{ "Pandor.Rigidbody", ScriptFieldType::Rigidbody },
		{ "Pandor.Text", ScriptFieldType::Text },
		{ "Pandor.Button", ScriptFieldType::Button},
		{ "Pandor.Animator", ScriptFieldType::Animator},
		{ "Pandor.SoundEmitter", ScriptFieldType::SoundEmitter},
		{ "Pandor.BoxCollider", ScriptFieldType::BoxCollider},
	};

	namespace Helper
	{
		std::string ToLower(std::string str)
		{
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return tolower(c); });
			return str;
		}

		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				PrintError("Failed to open file: %s", filepath.c_str());
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = (uint32_t)(end - stream.tellg());

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = (uint32_t)size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				PrintError(errorMessage);
				return nullptr;
			}

			std::string pathStr = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathStr.c_str(), &status, 0);
			mono_image_close(image);

			delete[] fileData;
			fileData = nullptr;
			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				printf("%s.%s\n", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_scriptFieldTypeMap.find(typeName);
			if (it == s_scriptFieldTypeMap.end())
			{
				PrintError("Unknown script field type: %s", typeName.c_str());
				return ScriptFieldType::None;
			}

			return it->second;
		}

		bool IsComponentRefValid(EditorField& field, uint64_t& objectID, uint64_t& componentID, Component::BaseComponent*& component)
		{
			field.GetValues(objectID, componentID);

			Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(objectID);
			if (!object)
			{
				field.SetValue<bool>(false);
				return false;
			}

			component = object->GetComponentByID(componentID);
			if (!component)
			{
				field.SetValue<bool>(false);
				return false;
			}
			return true;
		}
	}

#pragma region Structs

	struct ObjectRef
	{
		std::shared_ptr<ScriptInstance> scriptInstance;
		std::string fieldName;
		Core::GameObject* ref;
	};
	struct ComponentRef
	{
		std::shared_ptr<ScriptInstance> scriptInstance;
		std::string fieldName;
		Component::BaseComponent* ref;
	};

	struct CollisionMethods
	{
		CollisionMethods(std::shared_ptr<ScriptClass> scriptClass)
		{
			onCollisionEnter = scriptClass->GetMethod("OnCollisionEnter", 1);
			onCollisionStay = scriptClass->GetMethod("OnCollisionStay", 1);
			onCollisionExit = scriptClass->GetMethod("OnCollisionExit", 1);

			onTriggerEnter = scriptClass->GetMethod("OnTriggerEnter", 1);
			onTriggerStay = scriptClass->GetMethod("OnTriggerStay", 1);
			onTriggerExit = scriptClass->GetMethod("OnTriggerExit", 1);
		}

		MonoMethod* onCollisionEnter = nullptr;
		MonoMethod* onCollisionStay = nullptr;
		MonoMethod* onCollisionExit = nullptr;

		MonoMethod* onTriggerEnter = nullptr;
		MonoMethod* onTriggerStay = nullptr;
		MonoMethod* onTriggerExit = nullptr;
	};

	struct ScriptEngineData
	{
		ScriptEngineData();

		MonoDomain* rootDomain = nullptr;
		MonoDomain* appDomain = nullptr;

		MonoAssembly* coreAssembly = nullptr;
		MonoImage* coreAssemblyImage = nullptr;

		MonoAssembly* gameAssembly = nullptr;
		MonoImage* gameAssemblyImage = nullptr;

		std::filesystem::path coreAssemblyPath;
		std::filesystem::path gameAssemblyPath;

		std::string solutionPath = "PandorGame\\Assets\\Scripts\\PandorGame.sln";

		ScriptClass scriptClass;
		MonoClass* componentClass = nullptr;
		MonoMethod* componentCtor = nullptr;
		MonoClass* gameObjectClass = nullptr;
		MonoMethod* gameObjectCtor = nullptr;

		std::map<std::string, std::shared_ptr<ScriptClass>> scriptClasses;
		std::map<std::string, std::shared_ptr<CollisionMethods>> collisionMethods;

		std::vector<std::string> assemblyClasses;

		std::unique_ptr<filewatch::FileWatch<std::string>> gameAssemblyFileWatcher;

		// True  => reload event added to the queue
		// False => assembly is up-to-date
		bool assemblyReloadPending = false;

		// Play mode
		Core::Scene* sceneContext = nullptr;

		// Fields that contains a reference (script, gameobject, component)
		std::vector<ObjectRef> objectRefs;
		std::vector<ComponentRef> componentRefs;
	};

	ScriptEngineData::ScriptEngineData()
	{
		solutionPath = ResourcesManager::ProjectPath() + '/' + ResourcesManager::ProjectName() + ".sln";
	}

	static ScriptEngineData* s_data = nullptr;

#pragma endregion

	// Called whenever "PandorGame" assembly is rebuilt
	// Adds an event to the main thread to reload the assembly (Script Hot Reload)
	static void OnGameAssemblyFileEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_data->assemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_data->assemblyReloadPending = true;
			Core::App::Get().AddEvent([]()
				{
					s_data->gameAssemblyFileWatcher.reset();
					ScriptEngine::ReloadAssembly();
				});
		}
	}

	void ScriptEngine::Init()
	{
		s_data = new ScriptEngineData();
		BuildSolution();

		InitMono();
		ScriptGlue::RegisterFunctions();

		std::string Assembly = std::filesystem::current_path().string() + "/Resources/Scripts/netstandard2.1/Pandor-ScriptCore.dll";
		LoadAssembly(Assembly);
		std::string GameAssembly = ResourcesManager::ProjectPath() + "/bin/netstandard2.1/" + ResourcesManager::ProjectName() + ".dll";
		LoadGameAssembly(GameAssembly);
		LoadAssemblyData();
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_data;
		s_data = nullptr;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("PandorJITRuntime");
		if (rootDomain == nullptr)
		{
			PrintError("Could not init Mono Domain");
			return;
		}

		// Store the root domain pointer
		s_data->rootDomain = rootDomain;

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_data->appDomain);
		s_data->appDomain = nullptr;

		mono_jit_cleanup(s_data->rootDomain);
		s_data->rootDomain = nullptr;
	}

	void ScriptEngine::BuildSolution()
	{
		// Construct the command to build the solution using MSBuild
		std::string buildCommand = "dotnet msbuild \"" + std::string(s_data->solutionPath) + "\"";

		// Execute the build command
		std::string restoreCMD = "dotnet restore \"" + std::string(s_data->solutionPath) + "\"";
		int result = system(restoreCMD.c_str());
		if (result != 0)
			PrintError("Restore failed");
		result = system(buildCommand.c_str());
		if (result != 0)
			PrintError("Project's solution build failed");
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_data->appDomain = mono_domain_create_appdomain((char*)"PandorScriptRuntime", nullptr);
		mono_domain_set(s_data->appDomain, true);

		s_data->coreAssemblyPath = filepath;
		s_data->coreAssembly = Helper::LoadMonoAssembly(filepath);
		s_data->coreAssemblyImage = mono_assembly_get_image(s_data->coreAssembly);
		// Helper::PrintAssemblyTypes(s_data->coreAssembly);
	}

	void ScriptEngine::LoadGameAssembly(const std::filesystem::path& filepath)
	{
		s_data->gameAssemblyPath = filepath;
		s_data->gameAssembly = Helper::LoadMonoAssembly(filepath);
		s_data->gameAssemblyImage = mono_assembly_get_image(s_data->gameAssembly);
		// Helper::PrintAssemblyTypes(s_data->gameAssembly);

		// Reset file watcher
		s_data->gameAssemblyFileWatcher = std::make_unique<filewatch::FileWatch<std::string>>(filepath.string(), OnGameAssemblyFileEvent);
		s_data->assemblyReloadPending = false;
	}

	void ScriptEngine::LoadAssemblyData()
	{
		LoadAssemblyClasses();
		ScriptGlue::RegisterComponents();

		// Retrieve and instantiate class
		s_data->scriptClass = ScriptClass("Pandor", "ScriptComponent", true);

		// Retrieve component class and contructor
		s_data->componentClass = mono_class_from_name(s_data->coreAssemblyImage, "Pandor", "BaseComponent");
		s_data->componentCtor = mono_class_get_method_from_name(s_data->componentClass, ".ctor", 2);

		// Retrieve gameObject class and contructor
		s_data->gameObjectClass = mono_class_from_name(s_data->coreAssemblyImage, "Pandor", "GameObject");
		s_data->gameObjectCtor = mono_class_get_method_from_name(s_data->gameObjectClass, ".ctor", 1);
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_data->appDomain);

		LoadAssembly(s_data->coreAssemblyPath);
		LoadGameAssembly(s_data->gameAssemblyPath);
		LoadAssemblyData();

		PrintLog("Reloaded C# Assembly");
	}

	void ScriptEngine::OnStart(Core::Scene* scene)
	{
		s_data->sceneContext = scene;
	}

	void ScriptEngine::OnStop()
	{
		s_data->sceneContext = nullptr;
		s_data->objectRefs.clear();
		s_data->componentRefs.clear();
	}

	void AddFileToSolution(const std::string& filePath)
	{
		const std::string command = "dotnet sln " + s_data->solutionPath + " add " + filePath;
		std::system(command.c_str());
	}

	void ScriptEngine::CreateScriptFile(const std::string& scriptName)
	{
		// Construct the script file path
		std::string scriptPath = ResourcesManager::AssetPath();

		// Create and open the script file
		std::ofstream scriptFile(scriptPath + scriptName + ".cs");
		if (scriptFile.is_open())
		{
			// Write some code to the script file
			scriptFile << "using System;\n";
			scriptFile << "using Pandor;\n\n";
			scriptFile << "public class " << scriptName << " : ScriptComponent" "\n";
			scriptFile << "{\n";
			scriptFile << "\t// Called once at the beginning of the Play Mode\n";
			scriptFile << "\tvoid OnCreate()\n";
			scriptFile << "\t{\n\n";
			scriptFile << "\t}\n\n";
			scriptFile << "\t// Called at each frame in Play Mode\n";
			scriptFile << "\tvoid OnUpdate(float dt)\n";
			scriptFile << "\t{\n\n";
			scriptFile << "\t}\n";
			scriptFile << "}\n";

			// Close the script file
			scriptFile.close();

			// Add the script file to the solution
			AddFileToSolution(scriptPath + scriptName + ".cs");

			// Open the solution in Visual Studio
			OpenSolution(scriptPath, scriptName + ".cs");
			BuildSolution();
		}
	}

	std::string searchFiles(const std::filesystem::path& directory, const std::regex& pattern)
	{
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			if (std::filesystem::is_directory(entry))
			{
				std::string out = searchFiles(entry.path(), pattern);  // Appel récursif pour les sous-répertoires
				if (!out.empty())
					return out;
			}
			else
			{
				std::string fileName = entry.path().filename().string();
				if (std::regex_search(fileName, pattern))
				{
					return entry.path().string();
				}
			}
		}
		return "";
	}

	void ScriptEngine::OpenSolution(const std::string& scriptFilePath, const std::string& scriptName)
	{
		const std::string solutionPath = s_data->solutionPath; // Replace with the actual solution path

		// Find the Visual Studio window by its class name or window title
		HWND hwnd = FindWindowA(nullptr, (Resources::ResourcesManager::ProjectName() + " - Microsoft Visual Studio").c_str());

		if (hwnd)
		{
			// Visual Studio window exists, bring it to the foreground
			SetForegroundWindow(hwnd);
			SetActiveWindow(hwnd);

			std::string command = " /edit ";
			std::string env = "devenv.exe";
			std::string newPath = searchFiles(scriptFilePath, std::regex(scriptName));
			command += newPath;

			ShellExecuteA(hwnd, "open", env.c_str(), command.c_str(), NULL, SW_SHOWNORMAL);
		}
		else
		{
			std::string newPath = searchFiles(scriptFilePath, std::regex(scriptName));
			// No Visual Studio window found, open a new window
			const std::string commandLineArgs = "\"" + solutionPath + "\" /command \"Edit.OpenFile " + newPath + "\"";
			HINSTANCE result = ShellExecuteA(nullptr, "open", "devenv.exe", commandLineArgs.c_str(), nullptr, SW_SHOWNORMAL);
		}
	}

	bool ScriptEngine::ScriptClassExists(std::string& className)
	{
		if (s_data->scriptClasses.find(className) != s_data->scriptClasses.end())
			return true;

		className = "None";
		return false;
	}

	MonoObject* ScriptEngine::CreateComponentInstance(uint64_t objectID, uint64_t componentID)
	{
		MonoObject* instance = mono_object_new(s_data->appDomain, s_data->componentClass);

		void* args[2] = { &objectID,  &componentID };
		mono_runtime_invoke(s_data->componentCtor, instance, args, nullptr);
		return instance;
	}

	void ScriptEngine::CreateScrpitInstance(Component::ScriptComponent& scriptComponent)
	{
		DestroyScrpitInstance(scriptComponent);
		scriptComponent.instance = std::make_shared<ScriptInstance>(s_data->scriptClasses[scriptComponent.name], scriptComponent.gameObject->uuid, scriptComponent.uuid);
	}

	void ScriptEngine::DestroyScrpitInstance(Component::ScriptComponent& scriptComponent)
	{
		if (scriptComponent.instance && scriptComponent.instance->m_instance)
		{
			mono_gchandle_free(mono_gchandle_new(scriptComponent.instance->m_instance, TRUE));
			scriptComponent.instance->m_instance = nullptr;
		}
	}

	void ScriptEngine::OnCreateScript(Component::ScriptComponent& scriptComponent)
	{
		auto& fields = scriptComponent.editorFields;
		for (auto& [name, fieldInstance] : *fields)
		{
			if (fieldInstance.field.type == ScriptFieldType::GameObject)
			{
				uint64_t id = fieldInstance.GetValue<uint64_t>();
				Core::GameObject* gameObject = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(id);
				if (!gameObject || Helper::ScriptFieldTypeToString(ScriptEngine::GetScriptClass(scriptComponent.name)->GetFields().find(name)->second.type) != "GameObject")
				{
					fieldInstance.SetValue<bool>(false);
					continue;
				}

				// Create a new instance of the GameObject class
				MonoObject* instance = mono_object_new(s_data->appDomain, s_data->gameObjectClass);

				// Call the constructor method on the GameObject instance
				void* args[1] = { fieldInstance.m_buffer };
				mono_runtime_invoke(s_data->gameObjectCtor, instance, args, nullptr);

				scriptComponent.instance->SetFieldValueInternal(name, instance);
				mono_gchandle_free(mono_gchandle_new(instance, TRUE));

				// Store the field ref
				s_data->objectRefs.push_back({ scriptComponent.instance, name, gameObject });
			}
			else if (fieldInstance.field.type == ScriptFieldType::ScriptComponent)
			{
				Component::BaseComponent* component = nullptr;
				uint64_t objectID, componentID;
				if (!Helper::IsComponentRefValid(fieldInstance, objectID, componentID, component))
					continue;

				Component::ScriptComponent* scriptRef = dynamic_cast<Component::ScriptComponent*>(component);
				std::shared_ptr<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(scriptComponent.name);

				if (!scriptRef->instance || !scriptClass || ScriptEngine::GetFieldTypeName(ScriptEngine::GetScriptClass(scriptComponent.name)->GetFields().find(name)->second) != scriptRef->name)
				{
					fieldInstance.SetValue<bool>(false);
					continue;
				}

				scriptComponent.instance->SetFieldValueInternal(name, scriptRef->instance->m_instance);

				// Store the field ref
				s_data->componentRefs.push_back({ scriptComponent.instance, name, scriptRef });
			}
			else if (fieldInstance.field.type > ScriptFieldType::ScriptComponent)
			{
				Component::BaseComponent* component = nullptr;
				uint64_t objectID, componentID;
				if (!Helper::IsComponentRefValid(fieldInstance, objectID, componentID, component))
					continue;

				if (Helper::ScriptFieldTypeToString(ScriptEngine::GetScriptClass(scriptComponent.name)->GetFields().find(name)->second.type) != Helper::ScriptFieldTypeToString(fieldInstance.field.type))
				{
					fieldInstance.SetValue<bool>(false);
					continue;
				}

				// Create a new instance of the GameObject class
				MonoObject* instance = mono_object_new(s_data->appDomain, s_data->componentClass);

				// Call the constructor method on the GameObject instance
				void* args[2] = { &objectID,  &componentID };
				mono_runtime_invoke(s_data->componentCtor, instance, args, nullptr);

				scriptComponent.instance->SetFieldValueInternal(name, instance);
				mono_gchandle_free(mono_gchandle_new(instance, TRUE));

				// Store the field ref
				s_data->componentRefs.push_back({ scriptComponent.instance, name, component });
			}
			else
			{
				scriptComponent.instance->SetFieldValueInternal(name, fieldInstance.m_buffer);
			}
		}

		scriptComponent.instance->InvokeOnCreate();
	}

	void ScriptEngine::OnUpdateScript(Component::ScriptComponent& scriptComponent, float dt)
	{
		scriptComponent.instance->InvokeOnUpdate(dt);
	}

	void ScriptEngine::UpdateObjectRefs()
	{
		if (!s_data)
			return;

		for (int i = 0; i < s_data->objectRefs.size(); i++)
		{
			if (s_data->objectRefs[i].ref->uuid == 0 && s_data->objectRefs[i].scriptInstance->m_instance)
			{
				s_data->objectRefs[i].scriptInstance->SetFieldValueInternal(s_data->objectRefs[i].fieldName, nullptr);
				s_data->objectRefs.erase(s_data->objectRefs.begin() + i);
				i--;
			}
		}
	}

	void ScriptEngine::UpdateComponentRefs()
	{
		if (!s_data)
			return;

		for (int i = 0; i < s_data->componentRefs.size(); i++)
		{
			if (s_data->componentRefs[i].ref->uuid == 0)
			{
				s_data->componentRefs[i].scriptInstance->SetFieldValueInternal(s_data->componentRefs[i].fieldName, nullptr);
				s_data->componentRefs.erase(s_data->componentRefs.begin() + i);
				i--;
			}
		}
	}

	bool ScriptEngine::IsScriptNameAvaliable(const char* scriptName)
	{
		std::string name = Helper::ToLower(scriptName);
		if (name.size() == 0)
			return false;
		for (const std::string className : s_data->assemblyClasses)
		{
			if (className == name)
				return false;
		}
		return true;
	}

	Core::Scene* ScriptEngine::GetSceneContext()
	{
		return s_data->sceneContext;
	}

	std::shared_ptr<ScriptClass> ScriptEngine::GetScriptClass(const std::string& name)
	{
		if (s_data->scriptClasses.find(name) == s_data->scriptClasses.end())
			return nullptr;

		return s_data->scriptClasses.at(name);
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_data->coreAssemblyImage;
	}

	void ScriptEngine::GetFieldValue(ScriptClass& scriptClass, MonoClassField* classField, const void* value)
	{
		mono_field_get_value(scriptClass.Instantiate(), classField, (void*)value);
	}

	std::string ScriptEngine::GetFieldTypeName(const ScriptField& scriptField)
	{
		MonoType* type = mono_field_get_type(scriptField.classField);
		std::string name = (std::string)mono_type_get_name(type);
		return name.substr(name.find_last_of('.') + 1);
	}

	Component::ScriptComponent* ScriptEngine::GetScriptFromTypeInternal(const Core::GameObject* gameObject, const MonoType* type, const MonoClass* fieldClass)
	{
		std::vector<Component::ScriptComponent*> components = gameObject->GetComponents<Component::ScriptComponent>();

		if (components.size() != 0)
		{
			if (fieldClass == s_data->scriptClass.GetMonoClass())
				return components[0];

			for (Component::ScriptComponent* script : components)
			{
				auto scriptClass = s_data->scriptClasses.find(script->name);
				if (scriptClass != s_data->scriptClasses.end() && scriptClass->second->m_monoClass == fieldClass)
					return script;
			}
		}

		for (Core::GameObject* child : gameObject->GetChildrens())
		{
			if (Component::ScriptComponent* res = ScriptEngine::GetScriptFromTypeInternal(child, type, fieldClass))
				return res;
		}

		return nullptr;
	}

	Component::ScriptComponent* ScriptEngine::GetScriptFromType(const Core::GameObject* gameObject, const ScriptField& scriptField)
	{
		MonoType* type = mono_field_get_type(scriptField.classField);
		MonoClass* fieldClass = mono_class_from_mono_type(type);

		Component::ScriptComponent* res = GetScriptFromTypeInternal(gameObject, type, fieldClass);
		return res;
	}

	std::vector<Component::ScriptComponent*> ScriptEngine::GetScriptsFromType(const Core::GameObject* gameObject, const ScriptField& scriptField)
	{
		MonoType* type = mono_field_get_type(scriptField.classField);
		MonoClass* fieldClass = mono_class_from_mono_type(type);

		std::vector<Component::ScriptComponent*> components = gameObject->GetComponents<Component::ScriptComponent>();
		if (components.size() == 0)
			return {};
		if (fieldClass == s_data->scriptClass.GetMonoClass())
			return components;

		std::vector<Component::ScriptComponent*> list;
		for (Component::ScriptComponent* script : components)
		{
			auto scriptClass = s_data->scriptClasses.find(script->name);
			if (scriptClass != s_data->scriptClasses.end() && scriptClass->second->m_monoClass == fieldClass)
				list.push_back(script);
		}
		return list;
	}

	std::string ScriptEngine::GetMethodName(MonoMethod* method)
	{
		return mono_method_get_name(method);
	}

	std::map<std::string, std::shared_ptr<ScriptClass>> ScriptEngine::GetScriptClasses()
	{
		return s_data->scriptClasses;
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_data->scriptClasses.clear();
		s_data->assemblyClasses.clear();
		s_data->collisionMethods.clear();

		// Store core assembly class names
		const MonoTableInfo* coreTypeDefinitionsTable = mono_image_get_table_info(s_data->coreAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t coreNumTypes = mono_table_info_get_rows(coreTypeDefinitionsTable);
		for (int32_t i = 1; i < coreNumTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(coreTypeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* className = mono_metadata_string_heap(s_data->coreAssemblyImage, cols[MONO_TYPEDEF_NAME]);

			s_data->assemblyClasses.push_back(Helper::ToLower(className));
		}

		// Store game assembly classes
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_data->gameAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* scriptComponentClass = mono_class_from_name(s_data->coreAssemblyImage, "Pandor", "ScriptComponent");

		for (int32_t i = 1; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_data->gameAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_data->gameAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			s_data->assemblyClasses.push_back(Helper::ToLower(className));

			std::string fullName;
			if (strlen(nameSpace) != 0)
			{
				fullName += nameSpace;
				fullName += ".";
				fullName += className;
			}
			else
			{
				fullName = className;
			}
			MonoClass* monoClass = mono_class_from_name(s_data->gameAssemblyImage, nameSpace, className);

			if (monoClass == nullptr || monoClass == scriptComponentClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, scriptComponentClass, false);
			if (!isEntity)
				continue;

			std::shared_ptr<ScriptClass> scriptClass = std::make_shared<ScriptClass>(nameSpace, className);
			s_data->scriptClasses.emplace(fullName, scriptClass);
			std::shared_ptr<CollisionMethods> collisionMethods = std::make_shared<CollisionMethods>(scriptClass);
			s_data->collisionMethods.emplace(fullName, collisionMethods);

			// This routine is an iterator routine for retrieving the fields in a class.
			// You must pass a gpointer that points to zero and is treated as an opaque handle
			// to iterate over all of the elements. When no more values are available, the return value is NULL.

			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);

					MonoClass* monoClass = mono_class_from_mono_type(type);
					if (mono_class_is_subclass_of(monoClass, scriptComponentClass, false))
						type = mono_class_get_type(scriptComponentClass);

					ScriptFieldType fieldType = Helper::MonoTypeToScriptFieldType(type);

					scriptClass->m_fields[fieldName] = { fieldType, fieldName, field };
				}
			}
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_data->appDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_classNamespace(classNamespace), m_className(className)
	{
		m_monoClass = mono_class_from_name(isCore ? s_data->coreAssemblyImage : s_data->gameAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_monoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_monoClass, name.c_str(), parameterCount);
	}

	std::vector<MonoMethod*> ScriptClass::GetMethods()
	{
		void* iter = NULL;
		std::vector<MonoMethod*> methods;
		MonoMethod* method;
		while ((method = mono_class_get_methods(m_monoClass, &iter)))
		{
			methods.push_back(method);
		}
		return methods;
	}

	std::string ScriptClass::GetMethodName(MonoMethod* method)
	{
		return mono_method_get_name(method);
	}

	bool ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		MonoObject* result = mono_runtime_invoke(method, instance, params, &exception);

		if (exception != nullptr)
		{
			// Exception occurred, retrieve information
			MonoClass* exceptionClass = mono_object_get_class(exception);
			const char* exceptionTypeName = mono_class_get_name(exceptionClass);

			// Get the exception message property
			MonoProperty* exceptionMessageProperty = mono_class_get_property_from_name(exceptionClass, "Message");
			MonoMethod* exceptionMessageGetMethod = mono_property_get_get_method(exceptionMessageProperty);
			MonoString* exceptionMessage = reinterpret_cast<MonoString*>(mono_runtime_invoke(exceptionMessageGetMethod, exception, nullptr, nullptr));
			const char* exceptionMessageString = mono_string_to_utf8(exceptionMessage);
			const char* methodName = mono_method_get_name(method);

			// Print the exception type and message
			PrintError("Exception Type: %s\nClass: %s | Method: %s => %s", exceptionTypeName, m_className.c_str(), methodName, exceptionMessageString);
			return false;
		}
		return true;
	}

	void ScriptClass::InvokeMethodVoid(MonoObject* instance, MonoMethod* method)
	{
		InvokeMethod(instance, method);
	}

	ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, uint64_t objectID, uint64_t componentID)
		:m_scriptClass(scriptClass)
	{
		m_instance = scriptClass->Instantiate();

		m_constructor = s_data->componentCtor;
		m_onCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_onUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		// Call contructor
		void* param[2] = { &objectID, &componentID };
		m_scriptClass->InvokeMethod(m_instance, m_constructor, param);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_onCreateMethod)
			m_scriptClass->InvokeMethod(m_instance, m_onCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float dt)
	{
		if (m_onUpdateMethod)
		{
			void* param = &dt;
			if (!m_scriptClass->InvokeMethod(m_instance, m_onUpdateMethod, &param))
				m_onUpdateMethod = nullptr;
		}
	}

	void ScriptInstance::InvokeOnCollision(Physic::CollisionType type, Component::Collider* collider)
	{
		std::shared_ptr<CollisionMethods> methods = s_data->collisionMethods.find(m_scriptClass->GetName())->second;
		switch (type)
		{
		case Physic::CollisionType::ENTER:
			if (methods->onCollisionEnter) {
				void* instance = ScriptEngine::CreateComponentInstance(collider->gameObject->uuid, collider->uuid);
				m_scriptClass->InvokeMethod(m_instance, methods->onCollisionEnter, &instance);
			}
			break;
		case Physic::CollisionType::STAY:
			if (methods->onCollisionStay) {
				void* instance = ScriptEngine::CreateComponentInstance(collider->gameObject->uuid, collider->uuid);
				m_scriptClass->InvokeMethod(m_instance, methods->onCollisionStay, &instance);
			}
			break;
		case Physic::CollisionType::EXIT:
			if (methods->onCollisionExit) {
				void* instance = ScriptEngine::CreateComponentInstance(collider->gameObject->uuid, collider->uuid);
				m_scriptClass->InvokeMethod(m_instance, methods->onCollisionExit, &instance);
			}
			break;
		default:
			break;
		}
	}

	void ScriptInstance::InvokeOnTrigger(Physic::CollisionType type, Component::Collider* collider)
	{
		std::shared_ptr<CollisionMethods> methods = s_data->collisionMethods.find(m_scriptClass->GetName())->second;
		switch (type)
		{
		case Physic::CollisionType::ENTER:
			if (methods->onTriggerEnter) {
				void* instance = ScriptEngine::CreateComponentInstance(collider->gameObject->uuid, collider->uuid);
				m_scriptClass->InvokeMethod(m_instance, methods->onTriggerEnter, &instance);
			}
			break;
		case Physic::CollisionType::STAY:
			if (methods->onTriggerStay) {
				void* instance = ScriptEngine::CreateComponentInstance(collider->gameObject->uuid, collider->uuid);
				m_scriptClass->InvokeMethod(m_instance, methods->onTriggerStay, &instance);
			}
			break;
		case Physic::CollisionType::EXIT:
			if (methods->onTriggerExit) {
				void* instance = ScriptEngine::CreateComponentInstance(collider->gameObject->uuid, collider->uuid);
				m_scriptClass->InvokeMethod(m_instance, methods->onTriggerExit, &instance);
			}
			break;
		default:
			break;
		}
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name)
	{
		const auto& fields = m_scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_instance, field.classField, s_fieldValueBuffer);
		return true;
	}

	uint64_t ScriptInstance::GetFieldObject(const std::string& name)
	{
		const auto& fields = m_scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return uint64_t();

		const ScriptField& field = it->second;

		MonoObject* nestedClassObject = mono_field_get_value_object(s_data->appDomain, field.classField, m_instance);
		if (!nestedClassObject)
			return uint64_t();
		MonoClass* nestedClass = mono_object_get_class(nestedClassObject);
		MonoClassField* idField = mono_class_get_field_from_name(nestedClass, "ID");

		mono_field_get_value(nestedClassObject, idField, s_fieldValueBuffer);
		return *(uint64_t*)s_fieldValueBuffer;
	}

	std::pair<uint64_t, uint64_t> ScriptInstance::GetFieldComponent(const std::string& name)
	{
		const auto& fields = m_scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return std::pair<uint64_t, uint64_t>();

		const ScriptField& field = it->second;

		uint64_t objectID;
		uint64_t componentID;
		// Get component ID from mono field
		MonoObject* nestedClassObject = mono_field_get_value_object(s_data->appDomain, field.classField, m_instance);
		if (!nestedClassObject)
			return std::pair<uint64_t, uint64_t>();
		MonoClass* nestedClass = mono_object_get_class(nestedClassObject);
		MonoClassField* idField = mono_class_get_field_from_name(nestedClass, "ID");
		mono_field_get_value(nestedClassObject, idField, &componentID);

		// Get gameObject ID from mono field
		MonoClassField* objectField = mono_class_get_field_from_name(nestedClass, "gameObject");
		nestedClassObject = mono_field_get_value_object(s_data->appDomain, objectField, nestedClassObject);
		idField = mono_class_get_field_from_name(nestedClass, "ID");
		mono_field_get_value(nestedClassObject, idField, &objectID);

		memcpy(s_fieldValueBuffer, &objectID, sizeof(uint64_t));
		memcpy(s_fieldValueBuffer + sizeof(uint64_t), &componentID, sizeof(uint64_t));
		return std::make_pair(objectID, componentID);
	}

	void ScriptInstance::SetFieldObject(Component::ScriptComponent& scriptComp, const std::string& name, const uint64_t& objectID)
	{
		auto fields = scriptComp.editorFields;
		auto it = (*fields).find(name);
		if (it == (*fields).end())
			return;

		// Create a new instance of the GameObject class
		MonoObject* instance = mono_object_new(s_data->appDomain, s_data->gameObjectClass);

		// Call the constructor method on the GameObject instance
		uint64_t ID[1] = { objectID };
		void* args[1] = { ID };
		mono_runtime_invoke(s_data->gameObjectCtor, instance, args, nullptr);

		scriptComp.instance->SetFieldValueInternal(name, instance);

	}

	void ScriptInstance::SetFieldComponent(Component::ScriptComponent& scriptComp, const std::string& name, const uint64_t& objectID, const uint64_t& compID)
	{
		auto fields = scriptComp.editorFields;
		auto it = (*fields).find(name);
		if (it == (*fields).end())
			return;

		// Create a new instance of the GameObject class
		MonoObject* instance = mono_object_new(s_data->appDomain, s_data->componentClass);

		// Call the constructor method on the GameObject instance
		uint64_t ID1[1] = { objectID };
		uint64_t ID2[1] = { compID };
		void* args[2] = { ID1, ID2 };
		mono_runtime_invoke(s_data->componentCtor, instance, args, nullptr);

		scriptComp.instance->SetFieldValueInternal(name, instance);
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_scriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end() || !m_instance)
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_instance, field.classField, (void*)value);
		return true;
	}
}

