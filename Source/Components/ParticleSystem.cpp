#include "pch.h"
#include <Components/ParticleSystem.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Mesh.h>
#include <Resources/Material.h>
#include <Resources/Shader.h>
#include <Utils/Loader.h>
#include <Utils/Utils.h>
#include <Core/GameObject.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Core/App.h>
#ifndef PANDOR_GAME
#include <Render/EditorIcon.h>
#endif
// Function to convert vector of InstanceData structs to float* pointer
std::vector<float> convertToFloatPointer(const std::vector<Component::InstanceData>& instances) {
	const size_t numInstances = instances.size();
	const size_t floatArraySize = numInstances * 8;  // Each struct has 8 floats

	std::vector<float> floatArray;  // Allocate memory using std::vector
	floatArray.resize(floatArraySize);

	for (int i = 0; i < numInstances; ++i) {
		const Component::InstanceData& instance = instances[i];
		const int baseIndex = i * 8;

		floatArray[baseIndex + 0] = instance.xyzs.x;
		floatArray[baseIndex + 1] = instance.xyzs.y;
		floatArray[baseIndex + 2] = instance.xyzs.z;
		floatArray[baseIndex + 3] = instance.xyzs.w;

		floatArray[baseIndex + 4] = instance.color.x;
		floatArray[baseIndex + 5] = instance.color.y;
		floatArray[baseIndex + 6] = instance.color.z;
		floatArray[baseIndex + 7] = instance.color.w;
	}

	return floatArray;  // Return the pointer to the underlying data
}

template<>
bool Component::MinMaxValue<float>::ShowInInspector(const std::string& name)
{
	WrapperUI::PushID(name.c_str());
	bool value = false;
	WrapperUI::SetNextItemWidth(250.f);
	switch (this->Mode)
	{
	case ValueMode::Constant:
		value = WrapperUI::DragFloat(name.c_str(), &Value.Min);
		break;
	case ValueMode::Random:
		value = WrapperUI::DragFloatRange2(name.c_str(), &Value.Min, &Value.Max);
		break;
	default:
		break;
	}
	WrapperUI::SameLine();
	WrapperUI::SetNextItemWidth(20.f);
	WrapperUI::Combo("##", (int*)&Mode, "Constant\0Random");
	WrapperUI::PopID();
	return value;
}

template<>
bool Component::MinMaxValue<Vector4>::ShowInInspector(const std::string& name)
{
	WrapperUI::PushID(name.c_str());
	bool value = false;
	WrapperUI::SetNextItemWidth(250.f);
	switch (this->Mode)
	{
	case ValueMode::Constant:
		value = WrapperUI::ColorEdit4(name.c_str(), &Value.Min.x);
		break;
	case ValueMode::Random:
		value |= WrapperUI::ColorEdit4(name.c_str(), &Value.Min.x);
		value |= WrapperUI::ColorEdit4((name + "##2").c_str(), &Value.Max.x);
		break;
	default:
		break;
	}
	WrapperUI::SameLine();
	WrapperUI::SetNextItemWidth(20.f);
	WrapperUI::Combo("##", (int*)&Mode, "Constant\0Random");
	WrapperUI::PopID();
	return value;
}

template<typename T>
void Component::MinMaxValue<T>::ReadValue(std::fstream& sceneFile)
{

}

template<>
void Component::MinMaxValue<float>::ReadValue(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		this->Mode = (ValueMode)std::stoi(line);

	if (getline(sceneFile, line) && line != "end") {
		auto valueMinMax = ParseVector2(line);
		this->Value.Min = valueMinMax.x;
		this->Value.Max = valueMinMax.y;
	}
}

template<>
void Component::MinMaxValue<Vector4>::ReadValue(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		this->Mode = (ValueMode)std::stoi(line);

	if (getline(sceneFile, line) && line != "end") {
		size_t commaCount = 0;
		size_t pos = 0;
		while (commaCount < 4 && pos != std::string::npos) {
			pos = line.find(',', pos + 1);
			commaCount++;
		}

		if (pos != std::string::npos) {
			// Séparer la chaîne en deux parties
			std::string firstPart = line.substr(0, pos);
			std::string secondPart = line.substr(pos + 1);

			auto valueMin = ParseVector4(firstPart);
			auto valueMax = ParseVector4(secondPart);
			this->Value.Min = valueMin;
			this->Value.Max = valueMax;
		}
	}
}

Component::ParticleSystem::ParticleSystem()
{

}

Component::ParticleSystem::~ParticleSystem()
{
	for (auto& particle : m_particles)
	{
		delete particle;
		particle = nullptr;
	}
	m_particles.clear();
#ifndef PANDOR_GAME
	delete m_icon;
	m_icon = nullptr;
#endif
}

void Component::ParticleSystem::ShowInInspector()
{
	// Debug Window
	if (WrapperUI::Begin("Particles", (bool*)true, WindowFlags::NoDocking))
	{
		if (WrapperUI::Button(m_play ? "Pause" : "Play")) { m_play = !m_play; }
		WrapperUI::SameLine();
		if (WrapperUI::Button("Restart"))
		{
			m_time = 0;
			InitializeParticles();
		}
		WrapperUI::SameLine();
		if (WrapperUI::Button("Stop")) {
			m_time = 0;
			m_play = false;
			//_drawParticles = false;
			//ResetPositions();
		}
		WrapperUI::PushItemWidth(100.f);
		WrapperUI::DragFloat("Playback Time", &m_time, 0.1f, 0.001f);
		if (m_time < 0) m_time = 0;
		WrapperUI::DragFloat("Playback Speed", &m_speed, 1.0f, 0.001f);
		if (m_speed < 0) m_speed = 0;
		WrapperUI::Text("Particle Number : %d", m_currentParticleNumber);
		WrapperUI::PopItemWidth();
	}
	WrapperUI::End();
	// Debug Window
	TreeNodeFlags flags = (TreeNodeFlags)((int)TreeNodeFlags::AllowItemOverlap | (int)TreeNodeFlags::NoTreePushOnOpen);
	if (WrapperUI::CollapsingHeader("Renderer", flags))
	{
		if (WrapperUI::Button("Mesh"))
		{
			WrapperUI::OpenPopup("MeshPopup");
		}
		WrapperUI::SameLine();
		WrapperUI::TextUnformatted(!m_mesh ? "None" : m_mesh->GetName().c_str());
		if (auto mesh = Resources::ResourcesManager::Get()->MeshPopup("MeshPopup"))
		{
			Resources::ResourcesManager::Get()->GetOrLoad<Resources::Mesh>(mesh->GetPath());
			SetMesh(mesh);
		}
		if (WrapperUI::Button("Material"))
		{
			WrapperUI::OpenPopup("MaterialPopup");
		}
		WrapperUI::SameLine();
		WrapperUI::TextUnformatted(!m_material ? "None" : m_material->GetName().c_str());
		if (auto mat = Resources::ResourcesManager::Get()->MaterialPopup("MaterialPopup"))
		{
			Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(mat->GetPath());
			SetMaterial(mat);
		}
	}

	if (WrapperUI::CollapsingHeader("Default", flags))
	{
		int size = (int)this->m_particleNumber;
		if (WrapperUI::InputInt("Particles Count", &size, 1, 100, InputTextFlags::EnterReturnsTrue)) {
			if (size != this->m_particleNumber && size >= 0)
			{
				m_particleNumber = size;
				SetParticleNumber(m_particleNumber);
			}
		}
		WrapperUI::DragFloat("Duration", &m_duration);
		WrapperUI::Checkbox("Loop", &m_loop);
		m_startDelay.ShowInInspector("Start Delay");
		if (m_particleLifeTime.ShowInInspector("Life Time"))
		{
			InitializeParticles();
		}
		m_startSpeed.ShowInInspector("Start Speed");
		m_startSize.ShowInInspector("Start Size");
		m_gravityModifier.ShowInInspector("Gravity Modifier");
	}

	WrapperUI::Checkbox("##Emission", &m_enableEmission);
	WrapperUI::SameLine();
	if (WrapperUI::CollapsingHeader("Emission", flags))
	{
		WrapperUI::BeginDisabled(!m_enableEmission);
		if (m_rateOverTime.ShowInInspector("Rate Over Time"))
			InitializeParticles();
		WrapperUI::EndDisabled();
	}

	WrapperUI::Checkbox("##Shape", &m_enableShape);
	WrapperUI::SameLine();
	if (WrapperUI::CollapsingHeader("Shape", flags))
	{
		WrapperUI::BeginDisabled(!m_enableShape);
		int shapeIndex = (int)m_shape;
		if (WrapperUI::Combo("Shape List", &shapeIndex, "Cone\0Sphere\0Rectangle"))
			m_shape = (ParticleShape)shapeIndex;
		switch (m_shape)
		{
		case Component::ParticleShape::Cone:
			m_radius.ShowInInspector("Radius");
			if (m_radius.Value.Min < 0) m_radius.Value.Min = 0;
			m_angle.ShowInInspector("Angle");
			break;
		case Component::ParticleShape::Sphere:
			m_radius.ShowInInspector("Radius");

			if (m_radius.Value.Min < 0) m_radius.Value.Min = 0;
			break;
		case Component::ParticleShape::Rectangle:
			WrapperUI::DragFloat3("Scale", &m_scale.x);
			break;
		default:
			break;
		}
		WrapperUI::EndDisabled();
	}
	WrapperUI::Checkbox("##ColorOverTime", &m_enableColorOverTime);
	WrapperUI::SameLine();
	if (WrapperUI::CollapsingHeader("ColorOverTime", flags))
	{
		WrapperUI::BeginDisabled(!m_enableColorOverTime);
		m_colorOverTime.ShowInInspector("Color");
		WrapperUI::EndDisabled();
	}
}

void Component::ParticleSystem::Initialize()
{
	m_instanceVertShader = Resources::ResourcesManager::Get()->GetOrLoad<Resources::VertexShader>(ENGINEPATH"Shaders/BillboardInstanceShader/instancing.vert");
	SetParticleNumber(m_particleNumber);

#ifndef PANDOR_GAME
	m_icon = new Render::EditorIcon();
	m_icon->Initialize();
	m_icon->SetIcon(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(ENGINEPATH"Textures/icons/ParticleIcon.png"));
#endif
}


#include <glad/glad.h>
void Component::ParticleSystem::ReInitialize()
{
	if (!m_mesh || !m_buffer)
		return;
	if (m_buffer2)
	{
		delete m_buffer2;
		m_buffer2 = nullptr;
	}
	m_datas.resize(m_particles.size());

	m_buffer2 = new WrapperRHI::Buffer();
	m_buffer2->GenVertexBuffer();
	m_buffer2->BufferData(m_datas.size() * sizeof(InstanceData), nullptr);
	m_worldPosition = gameObject->transform->GetWorldPosition();
	m_worldRotation = gameObject->transform->GetWorldRotation();

	InitializeParticles();
	m_buffer->Bind();
	m_buffer->LinkAttribute(8, 4, PR_FLOAT, sizeof(InstanceData), (void*)0);
	m_buffer->AttribDivisor(8, 1);
	m_buffer->LinkAttribute(9, 4, PR_FLOAT, sizeof(InstanceData), (void*)(sizeof(float[4])));
	m_buffer->AttribDivisor(9, 1);
	m_buffer->Unbind();
}

void Component::ParticleSystem::InitializeParticles()
{
	for (auto& particle : m_particles)
	{
		particle->Initialize();
	}
}

void Component::ParticleSystem::SetMesh(Resources::Mesh* mesh)
{
	if (!mesh)
		return;
	m_mesh = mesh;
	m_buffer = new WrapperRHI::Buffer(m_mesh->m_vertices.data(), m_mesh->m_vertices.size(), 0, 0);

	m_buffer->Bind();
	m_buffer->LinkAttribute(0, 3, PR_FLOAT, 11 * sizeof(float), (void*)0);
	m_buffer->LinkAttribute(1, 2, PR_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	m_buffer->LinkAttribute(2, 3, PR_FLOAT, 11 * sizeof(float), (void*)(5 * sizeof(float)));
	m_buffer->LinkAttribute(3, 3, PR_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	m_buffer->Unbind();
	ReInitialize();
}

void Component::ParticleSystem::SetMaterial(Resources::Material* mat)
{
	m_material = mat;
}

void Component::ParticleSystem::EditorDraw()
{
#ifndef PANDOR_GAME
	m_icon->Draw(gameObject->transform->GetWorldPosition(), gameObject->IsSelected());
#endif

	if (gameObject->IsSelected())
	{
		static WrapperRHI::Line line(2.5f);
		line.Color = Vector4(0.130f, 0.739f, 1.0f, 1.0f);
		if (m_enableShape)
		{
			switch (m_shape)
			{
			case ParticleShape::Sphere:
			{
				line.DrawCircle(m_worldPosition, Vector3::Up(), m_radius.Value.Min, 32, Vector4(0.130f, 0.739f, 1.0f, 1.0f));
				line.DrawCircle(m_worldPosition, Vector3::Forward(), m_radius.Value.Min, 32, Vector4(0.130f, 0.739f, 1.0f, 1.0f));
				line.DrawCircle(m_worldPosition, Vector3::Right(), m_radius.Value.Min, 32, Vector4(0.130f, 0.739f, 1.0f, 1.0f));
				break;
			}
			case ParticleShape::Cone:
			{
				line.DrawCone(m_worldPosition, m_worldRotation, m_radius.Value.Min, m_angle.Value.Min, 25.f, Vector4(0.130f, 0.739f, 1.0f, 1.0f));
				break;
			}
			case ParticleShape::Rectangle:
			{
				//TODO
				Vector3 vertices0 = m_worldPosition + m_worldRotation * Vector3(-m_scale.x / 2.f, m_scale.y / 2.f, 0.f);
				Vector3 vertices1 = m_worldPosition + m_worldRotation * Vector3(m_scale.x / 2.f, m_scale.y / 2.f, 0.f);
				Vector3 vertices2 = m_worldPosition + m_worldRotation * Vector3(m_scale.x / 2.f, -m_scale.y / 2.f, 0.f);
				Vector3 vertices3 = m_worldPosition + m_worldRotation * Vector3(-m_scale.x / 2.f, -m_scale.y / 2.f, 0.f);

				line.Draw(vertices0, vertices1);
				line.Draw(vertices1, vertices2);
				line.Draw(vertices2, vertices3);
				line.Draw(vertices3, vertices0);
				break;
			}
			default:
				break;
			}
		}
	}
}

void Component::ParticleSystem::Draw()
{
	if (!m_mesh)
		return;
	if (Core::App::Get().GetGameState() != Core::GameState::Editor || gameObject->IsSelected())
		m_mesh->RenderInstancing(m_material, m_shader, m_particles.size(), m_buffer);
}

void Component::ParticleSystem::DrawPicking(int ID)
{
	if (m_mesh && gameObject->IsSelected())
		m_mesh->RenderInstancingPicking(m_pickingShader, m_particles.size(), ID);
#ifndef PANDOR_GAME
	if (m_icon)
		m_icon->DrawPicking(m_worldPosition, ID);
#endif
}


void Component::ParticleSystem::Update()
{
	if (!m_meshToLoad.empty() && !m_mesh)
	{
		SetMesh(Resources::ResourcesManager::Get()->Find<Resources::Mesh>(m_meshToLoad));
	}
	if (!m_meshToLoad.empty() && m_mesh) {
		if (m_mesh->HasBeenSent())
		{
			m_meshToLoad.clear();
			ReInitialize();
		}
	}
	m_currentParticleNumber = 0;
	m_worldPosition = gameObject->transform->GetWorldPosition();
	m_worldRotation = gameObject->transform->GetWorldRotation();
	if (m_material && m_material->GetShader() && m_material->GetShader()->GetFrag() && m_shader && m_shader->GetFrag() != m_material->GetShader()->GetFrag()
		|| m_material && m_material->GetShader() && m_material->GetShader()->GetFrag() && !m_shader)
	{
		auto name = Utils::StringFormat("%s + %s", m_instanceVertShader->GetPath().c_str(), ENGINEPATH"Shaders/unlit.frag");
		m_shader = Resources::ResourcesManager::Get()->Create(name, m_instanceVertShader->GetPath(), ENGINEPATH"Shaders/unlit.frag");
	}
	if (!m_pickingShader)
	{
		auto name = Utils::StringFormat("%s + %s", m_instanceVertShader->GetPath().c_str(), ENGINEPATH"Shaders/picking.frag");
		m_pickingShader = Resources::ResourcesManager::Get()->Create(name, m_instanceVertShader->GetPath(), ENGINEPATH"Shaders/picking.frag");
	}
	if (!m_buffer2)
		return;
	if (m_play && (Core::App::Get().GetGameState() != Core::GameState::Editor || gameObject->IsSelected()) || Core::App::Get().GetGameState() == Core::GameState::Play) {
		for (int i = 0; i < m_particles.size(); i++)
		{
			if (m_particles[i]->m_alive)
				m_currentParticleNumber++;
			m_particles[i]->Update(m_time - m_lastUpdateTime);
			m_datas[i] = m_particles[i]->GetDatas();
		}
		if (m_enableEmission && m_currentParticleNumber < m_rateOverTime.Value.Min || m_currentParticleNumber < 0)
		{
			m_shouldRestart = true;
		}
		else if (m_shouldRestart)
			m_shouldRestart = false;
		m_lastUpdateTime = m_time;
		if (m_particles.size() > 0) {
			m_buffer2->BindVertexBuffer();
			std::vector<float> datas = convertToFloatPointer(m_datas);
			m_buffer2->BufferSubData(0, m_datas.size() * sizeof(InstanceData), &datas[0]);
		}
		m_time += WrapperUI::GetDeltaTime() * m_speed;
	}
}

void Component::ParticleSystem::ResetParticles()
{
	for (auto& particle : m_particles)
	{
		particle->Reset();
	}
}

void Component::ParticleSystem::SetParticleNumber(size_t pn)
{
	m_particleNumber = pn;
	for (auto& p : m_particles)
	{
		delete p;
		p = nullptr;
	}
	m_particles.clear();
	for (int i = 0; i < m_particleNumber; i++)
	{
		m_particles.push_back(new Particle(this, i));
	}
	ReInitialize();
}

std::ostream& Component::ParticleSystem::operator<<(std::ostream& os)
{
	if (m_mesh) {
		os << m_mesh->GetModel()->GetPath() << "\n";
		os << m_mesh->GetPath() << "\n";
	}
	else {
		os << "nullptr" << "\n";
		os << "nullptr" << "\n";
	}
	if (m_material)
		os << m_material->GetPath() << "\n";
	else
		os << "nullptr" << "\n";
	os << m_particleNumber << "\n";
	os << m_duration << "\n";
	os << m_loop << "\n";
	os << m_startDelay;
	os << m_particleLifeTime;
	os << m_startSpeed;
	os << m_startSize;
	os << m_gravityModifier;
	os << m_enableEmission << '\n';
	os << m_rateOverTime;
	os << m_enableShape << '\n';
	os << (int)m_shape << '\n';
	os << m_radius;
	os << m_angle;
	os << m_scale << '\n';
	os << m_enableColorOverTime << '\n';
	os << m_colorOverTime;
	return os;
}

void Component::ParticleSystem::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_meshToLoad = line;
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_material = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(line);

	if (getline(sceneFile, line) && line != "end")
		m_particleNumber = std::stoi(line);

	if (getline(sceneFile, line) && line != "end")
		m_duration = std::stof(line);

	if (getline(sceneFile, line) && line != "end")
		m_loop = std::stoi(line);

	m_startDelay.ReadValue(sceneFile);
	m_particleLifeTime.ReadValue(sceneFile);
	m_startSpeed.ReadValue(sceneFile);
	m_startSize.ReadValue(sceneFile);
	m_gravityModifier.ReadValue(sceneFile);

	if (getline(sceneFile, line) && line != "end")
		m_enableEmission = std::stoi(line);

	m_rateOverTime.ReadValue(sceneFile);

	if (getline(sceneFile, line) && line != "end")
		m_enableShape = std::stoi(line);


	if (getline(sceneFile, line) && line != "end")
		m_shape = (ParticleShape)std::stoi(line);

	m_radius.ReadValue(sceneFile);
	m_angle.ReadValue(sceneFile);

	if (getline(sceneFile, line) && line != "end")
		m_scale = ParseVector3(line);


	if (getline(sceneFile, line) && line != "end")
		m_enableColorOverTime = std::stoi(line);

	m_colorOverTime.ReadValue(sceneFile);

	while (getline(sceneFile, line) && line != "end") {}
}

#pragma region Particle

Component::Particle::Particle(class ParticleSystem* ps, int index)
{
	m_particleSystem = ps;
	m_index = index;
}

Component::InstanceData Component::Particle::GetDatas()
{
	InstanceData data;
	data.xyzs = { m_particleSystem->m_worldPosition + m_position,  m_alive ? m_size : 0 };
	data.color = m_particleSystem->m_enableColorOverTime ? m_color : Vector4{1, 1, 1, 1};
	return data;
}

void Component::Particle::Initialize()
{
	m_startTime = m_index * (1 / m_particleSystem->m_rateOverTime.GetRandomValue(m_seed)) + m_particleSystem->m_startDelay.GetRandomValue(m_seed);
	SetParameters();
}

void Component::Particle::Update(float elapsedTime)
{
	m_lifeTime += elapsedTime;
	m_lifeTime = std::fmaxf(0.f, m_lifeTime);
	if (m_lifeTime > m_randomLifetime)
	{
		Reset();
	}
	else if (m_lifeTime >= m_startTime)
	{
		float time = m_lifeTime - m_startTime;
		m_position = m_offset + m_particleSystem->m_worldRotation * (m_direction * m_randomSpeed) * time
			+ Vector3::Up() * -9.81f * m_randomGravityModifier * 0.5f * time * time;
		if (m_particleSystem->m_enableColorOverTime)
		{
			float t = Arithmetics::Normalize(m_lifeTime, m_startTime, m_randomLifetime);
			m_color = m_particleSystem->m_colorOverTime.Value.Min.Lerp(m_particleSystem->m_colorOverTime.Value.Max, t);
		}
		m_alive = true;
	}
	else
	{
		m_alive = false;
	}
	Vector3 position = m_particleSystem->m_worldPosition + m_position;
}

void Component::Particle::Reset()
{
	if (m_particleSystem->m_loop && m_particleSystem->m_shouldRestart)
	{
		m_lifeTime = 0;
		m_position = 0;
		Initialize();
	}
	m_alive = false;
}

void Component::Particle::SetParameters()
{
	m_seed = std::mt19937(std::random_device()());
	m_randomLifetime = m_startTime + m_particleSystem->m_particleLifeTime.GetRandomValue(m_seed);
	m_randomSpeed = m_particleSystem->m_startSpeed.GetRandomValue(m_seed);
	m_randomGravityModifier = m_particleSystem->m_gravityModifier.GetRandomValue(m_seed);
	m_size = m_particleSystem->m_startSize.GetRandomValue(m_seed);
	if (m_particleSystem->m_enableShape) {
		switch (m_particleSystem->m_shape)
		{
		case ParticleShape::Sphere:
		{
			m_offset = Utils::RandomPointInSphere(m_particleSystem->m_radius.GetRandomValue(m_seed));
			m_direction = m_offset.GetNormalized();
			break;
		}
		case ParticleShape::Cone:
		{
			m_offset = Utils::RandomPointOnCircle(m_particleSystem->m_radius.GetRandomValue(m_seed), m_particleSystem->m_worldRotation * Vector3::Right());
			m_direction = Utils::RandomDirectionCone(Vector3::Forward(), m_particleSystem->m_radius.GetRandomValue(m_seed), m_particleSystem->m_angle.GetRandomValue(m_seed));
			break;
		}
		case ParticleShape::Rectangle:
		{
			m_offset = m_particleSystem->m_worldRotation * Utils::RandomPointOnRectangle(m_particleSystem->m_scale);
			m_direction = Vector3::Forward();
			break;
		}
		default:
			break;
		}
	}
	else {
		m_offset = 0;
		m_direction = Vector3::Forward();
	}
}

#pragma endregion
