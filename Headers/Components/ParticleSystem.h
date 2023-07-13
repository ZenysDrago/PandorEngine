#pragma once
#include <Components/BaseComponent.h>
#include <random>
namespace Resources
{
	class Mesh;
	class Shader;
	class Material;
	class VertexShader;
}
namespace Render
{
	class EditorIcon;
}
namespace Core::Wrapper::WrapperRHI
{
	class Buffer;
}
namespace Component
{
	struct InstanceData
	{
		Vector4 xyzs = { 0, 0, 0, 0 };
		Vector4 color = { 1, 1, 1, 1 };

		float* datas()
		{
			std::vector<float> floatArray(8);  // Allocate memory for 8 floats (2 vectors * 4 components)

			floatArray[0] = xyzs.x;
			floatArray[1] = xyzs.y;
			floatArray[2] = xyzs.z;
			floatArray[3] = xyzs.w;

			floatArray[4] = color.x;
			floatArray[5] = color.y;
			floatArray[6] = color.z;
			floatArray[7] = color.w;

			return floatArray.data();  // Return pointer to the underlying float array
		}
	};

	class Particle
	{
	public:
		Particle(class ParticleSystem* ps, int index);
		~Particle() {}

		inline InstanceData GetDatas();

		inline void Initialize();

		inline void Update(float elapsedTime);

		inline void Reset();

		inline void SetParameters();
	private:
		friend class ParticleSystem;
		class ParticleSystem* m_particleSystem = nullptr;
		size_t m_index = 0;

		Math::Matrix4 m_xyzs;

		float m_startTime = 0.f;
		float m_lifeTime = 0.f; // current time

		float m_size = 0.1f;
		bool m_alive = false;

		Vector3 m_position;
		Vector4 m_color = Vector4(1.f);

		//For shape
		Vector3 m_offset;
		Vector3 m_direction;

		std::mt19937 m_seed;

		float m_randomLifetime;
		float m_randomSpeed;
		float m_randomGravityModifier;
	};

	enum class ParticleShape : int
	{
		Cone,
		Sphere,
		Rectangle,
	};

	enum class ValueMode
	{
		Constant,
		Random,
	};

	template<typename T>
	struct MinMax
	{
		T Min;
		T Max;

		MinMax() {}

		MinMax(T value)
		{
			Min = value;
			Max = value;
		}

		MinMax(T value1, T value2)
		{
			Min = value1;
			Max = value2;
		}

		inline T GetRandomValue(std::mt19937 seed)
		{
			if (Min == Max || Min > Max) return Min;
			std::uniform_real_distribution<T> value(Min, Max);
			return value(seed);
		}
	};

	template<typename T>
	struct MinMaxValue
	{
		ValueMode Mode = ValueMode::Constant;
		MinMax<T> Value;

		MinMaxValue() {}

		MinMaxValue(T value)
		{
			Value = MinMax<T>(value);
		}

		MinMaxValue(T value, ValueMode mode)
		{
			Value = MinMax<T>(value);
			Mode = mode;
		}

		MinMaxValue(T value1, T value2)
		{
			Value = MinMax<T>(value1, value2);
		}

		bool ShowInInspector(const std::string& name);

		void ReadValue(std::fstream& sceneFile);

		T GetRandomValue(std::mt19937 seed)
		{
			if (Mode == ValueMode::Constant)
			{
				return Value.Min;
			}
			else
			{
				return Value.GetRandomValue(seed);
			}
		}

		friend std::ostream& operator<<(std::ostream& os, const MinMaxValue<T>& mmv)
		{
			os << (int)mmv.Mode << '\n';
			os << mmv.Value.Min << ',' << mmv.Value.Max << "\n";
			return os;
		}
	};

	class ParticleSystem : public IComponent<ParticleSystem>
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		void ShowInInspector() override;

		void Initialize() override;

		void ReInitialize();

		void InitializeParticles();

		void SetMesh(Resources::Mesh* mesh);

		void SetMaterial(Resources::Material* mat);

		void EditorDraw() override;

		void Draw() override;

		void DrawPicking(int ID) override;

		void Update() override;

		void ResetParticles();

		void SetParticleNumber(size_t pn);

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;

		std::string GetComponentName() override { return "Particle System"; }

	private:
		friend class Particle;
		std::string m_meshToLoad;

		Resources::Mesh* m_mesh = nullptr;
		Resources::VertexShader* m_instanceVertShader = nullptr;
		Resources::Shader* m_shader = nullptr;
		Resources::Shader* m_pickingShader = nullptr;
		Resources::Material* m_material = nullptr;
		Core::Wrapper::WrapperRHI::Buffer* m_buffer = nullptr;
		Core::Wrapper::WrapperRHI::Buffer* m_buffer2 = nullptr;

		std::vector<InstanceData>  m_datas;
		std::vector<Particle*> m_particles;

		bool m_play = false;
		float m_time = 0.0f;
		float m_lastUpdateTime = 0.0f;
		float m_speed = 1.f;
		size_t m_particleNumber = 1000;
		size_t m_currentParticleNumber = 0;
		Vector3 m_worldPosition;
		Quaternion m_worldRotation;

		// Main
		float m_duration = 5.f;
		bool m_loop = true;
		bool m_shouldRestart = false;
		MinMaxValue<float> m_startDelay = 0.f;
		MinMaxValue<float> m_particleLifeTime = 5.f;
		MinMaxValue<float> m_startSpeed = 5.f;
		MinMaxValue<float> m_startSize = 0.1f;
		MinMaxValue<float> m_gravityModifier = 0;

		//Emission
		bool m_enableEmission = true;
		MinMaxValue<float> m_rateOverTime = 10.f;

		//Shape
		bool m_enableShape = false;
		ParticleShape m_shape = ParticleShape::Sphere;
		MinMaxValue<float> m_radius = 1.f;
		MinMaxValue<float> m_angle = 25.f;
		Vector3 m_scale = Vector3(1.f);

		bool m_enableColorOverTime = false;
		MinMaxValue<Vector4> m_colorOverTime = MinMaxValue(Vector4{ 1, 1, 1, 1 }, ValueMode::Random);

#ifndef PANDOR_GAME
		Render::EditorIcon* m_icon = nullptr;
#endif
	};
}