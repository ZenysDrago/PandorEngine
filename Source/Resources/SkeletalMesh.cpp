#include "pch.h"
#include <regex>
#include <Resources\SkeletalMesh.h>
#include <Resources\Skeleton.h>

#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/MainBar.h>

#include <Render/Camera.h>

Resources::SkeletalMesh::~SkeletalMesh()
{
}

void Resources::SkeletalMesh::SendResource()
{
	m_buffer = new WrapperRHI::Buffer(m_vertices.data(), m_vertices.size(), 0, 0);
	m_buffer->Bind();

	m_buffer->LinkAttribute(0, 3, PR_FLOAT, 27 * sizeof(float), (void*)0);
	m_buffer->LinkAttribute(1, 2, PR_FLOAT, 27 * sizeof(float), (void*)(3 * sizeof(float)));
	m_buffer->LinkAttribute(2, 3, PR_FLOAT, 27 * sizeof(float), (void*)(5 * sizeof(float)));
	m_buffer->LinkAttribute(3, 3, PR_FLOAT, 27 * sizeof(float), (void*)(8 * sizeof(float)));
	m_buffer->LinkAttribute(4, 4, PR_FLOAT, 27 * sizeof(float), (void*)(11 * sizeof(float)));
	m_buffer->LinkAttribute(5, 4, PR_FLOAT, 27 * sizeof(float), (void*)(15 * sizeof(float)));
	m_buffer->LinkAttribute(6, 4, PR_FLOAT, 27 * sizeof(float), (void*)(19 * sizeof(float)));
	m_buffer->LinkAttribute(7, 4, PR_FLOAT, 27 * sizeof(float), (void*)(23 * sizeof(float)));

	m_buffer->Unbind(); 

#ifndef PANDOR_GAME
	auto filePath = std::regex_replace(p_path, std::regex("/"), "~");
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
#endif
	this->hasBeenSent = true;
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

void Resources::SkeletalMesh::Render(const Math::Matrix4& model, const std::vector<Resources::Material*>& materials, class Skeleton* skel, bool outline)
{
	if (!m_buffer || !hasBeenSent || materials.empty())
		return;
	m_buffer->Bind();

	Resources::Shader* shaderData = nullptr;
#ifndef	PANDOR_GAME
	if (outline) {
		if (!materials[0] || !materials[0]->GetShader() || !materials[0]->GetOutlineShader())
			return;
		shaderData = materials[0]->GetOutlineShader();
	}
#endif

	auto V = Core::SceneManager::Get()->GetCurrentScene()->GetViewMatrix();
	auto P = Core::SceneManager::Get()->GetCurrentScene()->GetProjectionMatrix();
	WrapperRHI::StencilActive();
	for (size_t i = 0; i < m_subMeshes.size(); i++)
	{
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

		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("modelMatrix"), model);
		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("viewMatrix"), V);
		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("projectionMatrix"), P);
		WrapperRHI::ShaderSendMat4(mat->GetShader()->GetLocation("skinningMatrices"), skel->GetBonesMatrices(), (int)skel->Bones.size());

		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("material.ambient"), mat->GetAmbient());
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("material.diffuse"), mat->GetDiffuse());
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("material.specular"), mat->GetSpecular());
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableTexture"), mat->GetTexture() != nullptr);
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableNormalMap"), mat->GetNormalMap() != nullptr);
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableRoughnessMap"), mat->GetRoughnessMap() != nullptr);
		WrapperRHI::ShaderSendInt(mat->GetShader()->GetLocation("enableMetallicMap"), mat->GetMetallicMap() != nullptr);
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
		WrapperRHI::ShaderSendVec4(mat->GetShader()->GetLocation("ourColor"), mat->GetDiffuse());

		WrapperRHI::UseStencil();
		WrapperRHI::DrawArrays(m_subMeshes[i].StartIndex, m_subMeshes[i].Count, false, true);

#ifndef	PANDOR_GAME
		if (outline && shaderData->HasBeenSent())
		{
			shaderData->Use();
			WrapperRHI::DepthRange({ 0.1f, 0.2f });
			WrapperRHI::DrawStencil(shaderData, Core::App::Get().GetEditorUIManager().GetMainBar().GetOutlineColor());

			WrapperRHI::ShaderSendMat4(shaderData->GetLocation("modelMatrix"), model);
			WrapperRHI::ShaderSendMat4(shaderData->GetLocation("viewMatrix"), V);
			WrapperRHI::ShaderSendMat4(shaderData->GetLocation("projectionMatrix"), P);
			WrapperRHI::ShaderSendMat4(shaderData->GetLocation("skinningMatrices"), skel->GetBonesMatrices(), (int)skel->Bones.size());

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

void Resources::SkeletalMesh::PickingResource(const Math::Matrix4& model, std::vector<class Material*> materials, class Skeleton* skel, int ID)
{
	int r = (ID & 0x000000FF) >> 0;
	int g = (ID & 0x0000FF00) >> 8;
	int b = (ID & 0x00FF0000) >> 16;

	if (!materials[0])
		return;
	auto shaderData = materials[0]->GetPickingShader();
	if (!shaderData)
		return;

	shaderData->Use();
	m_buffer->Bind();
	if (!shaderData->HasBeenSent())
		return;

	auto V = Core::SceneManager::Get()->GetCurrentScene()->GetViewMatrix();
	auto P = Core::SceneManager::Get()->GetCurrentScene()->GetProjectionMatrix();
	for (int i = 0; i < m_subMeshes.size(); i++)
	{
		WrapperRHI::ShaderSendMat4(shaderData->GetLocation("modelMatrix"), model);
		WrapperRHI::ShaderSendMat4(shaderData->GetLocation("viewMatrix"), V);
		WrapperRHI::ShaderSendMat4(shaderData->GetLocation("projectionMatrix"), P);
		WrapperRHI::ShaderSendMat4(shaderData->GetLocation("skinningMatrices"), skel->GetBonesMatrices(), (int)skel->Bones.size());

		WrapperRHI::ShaderSendVec4(shaderData->GetLocation("PickingColor"), { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f });
		WrapperRHI::DrawArrays(m_subMeshes[i].StartIndex, m_subMeshes[i].Count);
	}
}