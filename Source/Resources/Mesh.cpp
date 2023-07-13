#include "pch.h"

#include <regex>

#include <Resources/Mesh.h>

#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Core/GameObject.h>
#include <Render/Camera.h>
#include <LowRenderer/LightManager.h>
#include <Resources/Material.h>
#include <Resources/Shader.h>
#include <Resources/Model.h>
#include <Utils/Utils.h>
#include <EditorUI/MainBar.h>
#include <EditorUI/EditorUIManager.h>
#include <Components/LightComponent.h>

Resources::Mesh::~Mesh()
{
	delete m_buffer;
	m_buffer = nullptr;
	Utils::AABB* boundBox = GetBoudingBox();
	delete boundBox;
	boundBox = nullptr;
}


void Resources::Mesh::Load()
{
}

void Resources::Mesh::SendResource()
{
	m_buffer = new WrapperRHI::Buffer(m_vertices.data(), m_vertices.size(), 0, 0);
	m_buffer->Bind();

	m_buffer->LinkAttribute(0, 3, PR_FLOAT, 11 * sizeof(float), (void*)0);
	m_buffer->LinkAttribute(1, 2, PR_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	m_buffer->LinkAttribute(2, 3, PR_FLOAT, 11 * sizeof(float), (void*)(5 * sizeof(float)));
	m_buffer->LinkAttribute(3, 3, PR_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));

	m_buffer->Unbind();
	this->hasBeenSent = true;

#ifndef PANDOR_GAME
	if (Core::App::Get().createMeshThumbnails)
	{
		std::string filePath = std::regex_replace(p_path, std::regex("/"), "~");
		filePath = std::regex_replace(filePath, std::regex("\\\\"), "~");
		filePath = std::regex_replace(filePath, std::regex(":"), "`");
		// Check the last update of the thumbnails and the model file, if the models was update after the thumbnail creation then Create a new
		std::string thumbnailPath = ENGINEPATH"Textures/Thumbnails/" + filePath + ".tmb";
		try {
			if (std::filesystem::exists(thumbnailPath)) {
				// Get the last update time of the file
				const auto last_update_thumbnail = std::filesystem::last_write_time(thumbnailPath);

				// Get the time of the last update of the model.
				const auto last_update_model = std::filesystem::last_write_time(m_fromModel->GetFullPath());

				// Print the time difference in seconds
				if (last_update_model > last_update_thumbnail)
				{
					Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Mesh);
				}
			}
			else {
				Core::SceneManager::Get()->GetCurrentScene()->CreateThumbnail(p_path, ResourcesType::Mesh);
			}
		}
		catch (const std::exception& ex) {
			PrintError("An error occurred: %s", ex.what());
		}
	}
#endif

	for (auto&& vertex : m_positions)
	{
		m_minAABB.x = std::min(m_minAABB.x, vertex.x);
		m_minAABB.y = std::min(m_minAABB.y, vertex.y);
		m_minAABB.z = std::min(m_minAABB.z, vertex.z);

		m_maxAABB.x = std::max(m_maxAABB.x, vertex.x);
		m_maxAABB.y = std::max(m_maxAABB.y, vertex.y);
		m_maxAABB.z = std::max(m_maxAABB.z, vertex.z);
	}
	m_boudingBox = Utils::AABB::GenerateAABB(this);
}

void Resources::Mesh::PickingResource(const Math::Matrix4& MVP, std::vector<class Material*> materials, int ID)
{
	int r = (ID & 0x000000FF) >> 0;
	int g = (ID & 0x0000FF00) >> 8;
	int b = (ID & 0x00FF0000) >> 16;
	if (materials.empty() || !materials[0])
		return;
	auto shaderData = materials[0]->GetPickingShader();
	if (!shaderData || !m_buffer)
		return;

	shaderData->Use();
	m_buffer->Bind();
	if (!shaderData->HasBeenSent())
		return;

	auto upVector = Core::App::Get().sceneManager->GetCurrentScene()->GetUpVector();
	auto rightVector = Core::App::Get().sceneManager->GetCurrentScene()->GetRightVector();
	for (int i = 0; i < m_subMeshes.size(); i++)
	{
		WrapperRHI::ShaderSendVec3(materials[i]->GetShader()->GetLocation("CamUp"), upVector);
		WrapperRHI::ShaderSendVec3(materials[i]->GetShader()->GetLocation("CamRight"), rightVector);
		WrapperRHI::ShaderSendMat4(shaderData->GetLocation("MVP"), MVP);
		WrapperRHI::ShaderSendVec4(shaderData->GetLocation("PickingColor"), { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f });
		WrapperRHI::DrawArrays(m_subMeshes[i].StartIndex, m_subMeshes[i].Count);
	}
}

Matrix4 Resources::Mesh::ShadowVP()
{
	Matrix4 lightProj;
	Matrix4 orthoProj;
	Matrix4 lightView;

	std::vector<Component::LightComponent*> light = LowRenderer::LightManager::getInstance().GetAllLight();

	for (auto&& component : light)
	{
		Component::DirectionalLight* dirLight = dynamic_cast<Component::DirectionalLight*>(component);

		if (dirLight != NULL)
		{
			float value = dirLight->gameObject->transform->GetWorldPosition().y > 90 ? dirLight->gameObject->transform->GetWorldPosition().y : 100;
			orthoProj = CreateOrthoMatrix(-1000, 1000, -1000, 1000, -value, value);
			lightView = LookAt(Vector3(0.0, value, 0), Vector3(dirLight->gameObject->transform->GetForwardVector().x, dirLight->gameObject->transform->GetForwardVector().y, -dirLight->gameObject->transform->GetForwardVector().z), Vector3(0.0, 1.0, 0.0));

			lightProj = lightView * orthoProj;
		}

	}

	return lightProj;
}

void Resources::Mesh::Render(const Math::Matrix4& MVP, const Math::Matrix4& model, const std::vector<Material*>& materials, bool wireframe, bool cullface, bool drawOutline, bool drawShadow)
{
	if (!m_buffer || !hasBeenSent || materials.size() == 0)
		return;
	m_buffer->Bind();

	Resources::Shader* shaderData = nullptr;
	static Resources::Shader* lightShaderData = ResourcesManager::Get()->Find<Resources::Shader>(ENGINEPATH"Shaders/Shadow/ShadowMap");
#ifndef PANDOR_GAME

	if (drawOutline) {
		if (!materials[0] || !materials[0]->GetShader() || !materials[0]->GetOutlineShader())
			return;
		shaderData = materials[0]->GetOutlineShader();
	}
#endif
	if (Core::App::Get().shadowMap->stencilDepthTest)
		WrapperRHI::StencilActive();

	auto upVector = Core::App::Get().sceneManager->GetCurrentScene()->GetUpVector();
	auto rightVector = Core::App::Get().sceneManager->GetCurrentScene()->GetRightVector();

	for (size_t i = 0; i < m_subMeshes.size(); i++)
	{
		if (drawShadow && lightShaderData->HasBeenSent())
		{
			lightShaderData->Use();
			WrapperRHI::ShaderSendMat4(lightShaderData->GetLocation("MVP"), MVP);
			WrapperRHI::DrawArrays(m_subMeshes[i].StartIndex, m_subMeshes[i].Count, wireframe, cullface);
			return;
		}

		Material* mat;
		if (materials.size() <= i && materials[0])
		{
			mat = materials[0];
		}
		else
		{
			mat = materials[i];
		}
		if (!mat || !mat->GetShader() || !mat->GetShader()->HasBeenSent())
			return;
		mat->GetShader()->Use();

		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("MVP"), MVP);
		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("model"), model);
		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("lightSpaceMatrix"), ShadowVP());
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("material.ambient"), mat->GetAmbient());
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("material.diffuse"), mat->GetDiffuse());
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("material.specular"), mat->GetSpecular());
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableTexture"), mat->GetTexture() != nullptr);
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableNormalMap"), mat->GetNormalMap() != nullptr);
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableRoughnessMap"), mat->GetRoughnessMap() != nullptr);
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableMetallicMap"), mat->GetMetallicMap() != nullptr);
		WrapperRHI::ShaderSendVec3(mat->GetShader()->GetLocation("CamUp"), upVector);
		WrapperRHI::ShaderSendVec3(mat->GetShader()->GetLocation("CamRight"), rightVector);
		if (mat->GetTexture() != nullptr) {
			mat->GetTexture()->Active(0);
			mat->GetTexture()->Bind();
			WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("tex0"), 0);

		}
		if (mat->GetNormalMap() != nullptr) {
			mat->GetNormalMap()->Active(1);
			mat->GetNormalMap()->Bind();
			WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("normalMap"), 1);
		}
		if (mat->GetRoughnessMap() != nullptr) {
			mat->GetRoughnessMap()->Active(2);
			mat->GetRoughnessMap()->Bind();
			WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("roughnessMap"), 2);
		}
		else {
			WrapperRHI::ShaderSendFloat(mat->GetShader()->GetLocation("roughnessValue"), mat->roughness);
		}
		if (mat->GetMetallicMap() != nullptr) {
			mat->GetMetallicMap()->Active(3);
			mat->GetMetallicMap()->Bind();
			WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("metallicMap"), 3);
		}
		else {
			WrapperRHI::ShaderSendFloat(mat->GetShader()->GetLocation("metallicValue"), mat->metallic);
		}
		if (Core::SceneManager::Get()->GetCurrentScene()->currentCamera->skybox)
		{
			Core::SceneManager::Get()->GetCurrentScene()->currentCamera->skybox->Active(4);
			Core::SceneManager::Get()->GetCurrentScene()->currentCamera->skybox->Bind();
			WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("skybox"), 4);
		}
		if (Core::App::Get().shadowMap)
		{
			Core::App::Get().shadowMap->Active(5);
			Core::App::Get().shadowMap->Bind();
			WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("shadowMap"), 5);
		}
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("ourColor"), mat->GetDiffuse());

		WrapperRHI::UseStencil();
		WrapperRHI::DrawArrays(m_subMeshes[i].StartIndex, m_subMeshes[i].Count, wireframe, cullface);

#ifndef PANDOR_GAME
		if (drawOutline && shaderData->HasBeenSent())
		{
			shaderData->Use();
			WrapperRHI::DepthRange({ 0.1f, 0.2f });
			WrapperRHI::DrawStencil(shaderData, Core::App::Get().GetEditorUIManager().GetMainBar().GetOutlineColor());
			WrapperRHI::ShaderSendMat4(shaderData->GetLocation("MVP"), MVP);
			WrapperRHI::ShaderSendVec3(shaderData->GetLocation("CamUp"), upVector);
			WrapperRHI::ShaderSendVec3(shaderData->GetLocation("CamRight"), rightVector);
			WrapperRHI::DrawArrays(m_subMeshes[i].StartIndex, m_subMeshes[i].Count, true, false);
			WrapperRHI::MaskStencil();
			WrapperRHI::DepthRange({ 0.2f , 1 });
		}
#endif

		if (mat->GetMetallicMap()) {
			mat->GetMetallicMap()->Active(3);
			mat->GetMetallicMap()->UnBind();
		}
		if (mat->GetRoughnessMap()) {
			mat->GetRoughnessMap()->Active(2);
			mat->GetRoughnessMap()->UnBind();
		}
		if (mat->GetNormalMap()) {
			mat->GetNormalMap()->Active(1);
			mat->GetNormalMap()->UnBind();
		}
		if (mat->GetTexture()) {
			mat->GetTexture()->Active(0);
			mat->GetTexture()->UnBind();
		}

	}

	WrapperRHI::DisableStencil();

}

void Resources::Mesh::RenderInstancing(class Material* material, class Shader* shader, size_t count, WrapperRHI::Buffer* buffer /*= nullptr*/)
{
	if (!m_buffer || !shader || !shader->HasBeenSent() || !material)
		return;
	if (!buffer)
		m_buffer->Bind();
	else
		buffer->Bind();
	shader->Use();
	auto upVector = Core::App::Get().sceneManager->GetCurrentScene()->GetUpVector();
	auto rightVector = Core::App::Get().sceneManager->GetCurrentScene()->GetRightVector();
	auto forwardVector = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetTransform()->GetForwardVector();
	auto VP = Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
	WrapperRHI::ShaderSendMat4(shader->GetLocation("VPMatrix"), VP);
	WrapperRHI::ShaderSendVec4(shader->GetLocation("material.ambient"), material->GetAmbient());
	WrapperRHI::ShaderSendVec4(shader->GetLocation("material.diffuse"), material->GetDiffuse());
	WrapperRHI::ShaderSendVec4(shader->GetLocation("material.specular"), material->GetSpecular());
	WrapperRHI::ShaderSendInt(shader->GetLocation("enableTexture"), material->GetTexture() != nullptr);
	WrapperRHI::ShaderSendInt(shader->GetLocation("enableNormalMap"), material->GetNormalMap() != nullptr);
	WrapperRHI::ShaderSendInt(shader->GetLocation("enableRoughnessMap"), material->GetRoughnessMap() != nullptr);
	WrapperRHI::ShaderSendInt(shader->GetLocation("enableMetallicMap"), material->GetMetallicMap() != nullptr);
	WrapperRHI::ShaderSendVec3(shader->GetLocation("CamUp"), upVector);
	WrapperRHI::ShaderSendVec3(shader->GetLocation("CamRight"), rightVector);
	WrapperRHI::ShaderSendVec3(shader->GetLocation("CamForward"), forwardVector);
	if (material->GetTexture() != nullptr) {
		material->GetTexture()->Active(0);
		material->GetTexture()->Bind();
		WrapperRHI::ShaderSendInt(shader->GetLocation("tex0"), 0);
	}
	if (material->GetNormalMap() != nullptr) {
		material->GetNormalMap()->Active(1);
		material->GetNormalMap()->Bind();
		WrapperRHI::ShaderSendInt(shader->GetLocation("normalMap"), 1);
	}
	if (material->GetRoughnessMap() != nullptr) {
		material->GetRoughnessMap()->Active(2);
		material->GetRoughnessMap()->Bind();
		WrapperRHI::ShaderSendInt(shader->GetLocation("roughnessMap"), 2);
	}
	else {
		WrapperRHI::ShaderSendFloat(shader->GetLocation("roughnessValue"), material->roughness);
	}
	if (material->GetMetallicMap() != nullptr) {
		material->GetMetallicMap()->Active(3);
		material->GetMetallicMap()->Bind();
		WrapperRHI::ShaderSendInt(shader->GetLocation("metallicMap"), 3);
	}
	else {
		WrapperRHI::ShaderSendFloat(shader->GetLocation("metallicValue"), material->metallic);
	}
	WrapperRHI::ShaderSendVec4(shader->GetLocation("ourColor"), material->GetDiffuse());
	WrapperRHI::DrawInstance(0, m_subMeshes[0].Count, count);
}

void Resources::Mesh::RenderInstancingPicking(class Shader* shader, size_t count, int ID)
{
	if (!m_buffer || !shader || !shader->HasBeenSent())
		return;
	m_buffer->Bind();
	shader->Use();
	auto upVector = Core::App::Get().sceneManager->GetCurrentScene()->GetUpVector();
	auto rightVector = Core::App::Get().sceneManager->GetCurrentScene()->GetRightVector();
	auto forwardVector = Core::App::Get().sceneManager->GetCurrentScene()->currentCamera->GetTransform()->GetForwardVector();
	auto VP = Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
	WrapperRHI::ShaderSendMat4(shader->GetLocation("VPMatrix"), VP);
	WrapperRHI::ShaderSendVec3(shader->GetLocation("CamUp"), upVector);
	WrapperRHI::ShaderSendVec3(shader->GetLocation("CamRight"), rightVector);
	WrapperRHI::ShaderSendVec3(shader->GetLocation("CamForward"), forwardVector);

	int r = (ID & 0x000000FF) >> 0;
	int g = (ID & 0x0000FF00) >> 8;
	int b = (ID & 0x00FF0000) >> 16;
	WrapperRHI::ShaderSendVec4(shader->GetLocation("PickingColor"), { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f });
	WrapperRHI::DrawInstance(0, m_subMeshes[0].Count, count);
}

void Resources::Mesh::RenderUI(const Math::Vector2& Position, const Math::Vector2& Size, Resources::Material* material, float depth)
{
	m_buffer->Bind();
	if (!material->GetShader() || !material->GetShader()->HasBeenSent())
		return;
	material->GetShader()->Use();
	if (material->GetTexture())
		material->GetTexture()->Bind();

	WrapperRHI::ShaderSendVec3(material->GetShader()->GetLocation("position"), { Position.x, Position.y, depth });
	WrapperRHI::ShaderSendVec2(material->GetShader()->GetLocation("size"), Size);
	WrapperRHI::ShaderSendInt(material->GetShader()->GetLocation("enableTexture"), material->GetTexture() != nullptr);
	WrapperRHI::ShaderSendVec4(material->GetShader()->GetLocation("ourColor"), material->GetDiffuse());
	WrapperRHI::DrawArrays(m_subMeshes[0].StartIndex, m_subMeshes[0].Count);
}

void Resources::Mesh::BindBuffer()
{
	m_buffer->Bind();
}

void Resources::Mesh::UnBindBuffer()
{
	m_buffer->Unbind();
}

Resources::Texture* Resources::Mesh::GetThumbnail()
{
	if (!m_thumbnail)
	{
		auto filePath = std::regex_replace(p_path, std::regex("/"), "~");
		filePath = std::regex_replace(filePath, std::regex(":"), "`");
		std::string thumbnailPath = ENGINEPATH"Textures/Thumbnails/" + filePath + ".tmb";
		std::string assetPath = "assets~";
		if (filePath.substr(0, assetPath.length()) == assetPath)
		{
			std::string folder = ENGINEPATH"Textures/Thumbnails/" + Resources::ResourcesManager::ProjectName();
			if (!std::filesystem::exists(folder))
			{
				std::filesystem::create_directory(folder);
			}
			thumbnailPath = folder + '/' + filePath + ".tmb";
		}
		m_thumbnail = Resources::ResourcesManager::Get()->Reload<Resources::Texture>(thumbnailPath);
	}
	return m_thumbnail;
}

bool Resources::Mesh::IsVisible(Render::Camera* camera, Component::Transform* transform)
{
	if (m_boudingBox)
		return m_boudingBox->isOnFrustum(camera->frustum, transform);
	return false;
}
