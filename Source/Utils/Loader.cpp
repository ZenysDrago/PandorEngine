#include "pch.h"

#include <Utils/Loader.h>
#include <Core/App.h>
#include <Resources/ResourcesManager.h>

#include <Resources/Model.h>
#include <Resources/Mesh.h>
#include <Resources/SkeletalMesh.h>
#include <Resources/Animation.h>
#include <Resources/AnimationController.h>
#include <Resources/Skeleton.h>
#include <Resources/Material.h>
#include <OpenFBX/ofbx.h>

#define FBXScale 0.01f

std::string ExtractName(std::string path)
{
	if (path.empty())
	{
		std::cout << "Error: Empty string" << std::endl;
		return "";
	}

	size_t sepPos = path.find_last_of("/\\");
	if (sepPos == std::string::npos)
	{
		std::cout << "Error: Invalid file path" << std::endl;
		return "";
	}
	std::string fileName = path.substr(sepPos + 1);

	return fileName;
}


std::vector<float> ComputeVertices(const std::vector<Math::Vector3>& Positions,
	const std::vector<Math::Vector2>& TextureUVs,
	const std::vector<Math::Vector3>& Normals,
	const std::vector<Math::Vector3>& Indices)
{
	std::vector<float> vertices;
	vertices.reserve(Indices.size() * 11); // 11 floats per vertex
	std::vector<Math::Vector3> Tangents(Indices.size());

	for (size_t k = 0; k < Indices.size(); k += 3)
	{
		const Math::Vector3& idx0 = Indices[k];
		const Math::Vector3& idx1 = Indices[k + 1];
		const Math::Vector3& idx2 = Indices[k + 2];

		const Math::Vector3& Edge1 = Positions[(int)idx1.x] - Positions[(int)idx0.x];
		const Math::Vector3& Edge2 = Positions[(int)idx2.x] - Positions[(int)idx0.x];

		const float DeltaU1 = TextureUVs[(int)idx1.y].x - TextureUVs[(int)idx0.y].x;
		const float DeltaV1 = TextureUVs[(int)idx1.y].y - TextureUVs[(int)idx0.y].y;
		const float DeltaU2 = TextureUVs[(int)idx2.y].x - TextureUVs[(int)idx0.y].x;
		const float DeltaV2 = TextureUVs[(int)idx2.y].y - TextureUVs[(int)idx0.y].y;

		const float f = 1.0f / (DeltaU1 * DeltaV2 - DeltaU2 * DeltaV1);

		Math::Vector3 Tangent;

		Tangent.x = f * (DeltaV2 * Edge1.x - DeltaV1 * Edge2.x);
		Tangent.y = f * (DeltaV2 * Edge1.y - DeltaV1 * Edge2.y);
		Tangent.z = f * (DeltaV2 * Edge1.z - DeltaV1 * Edge2.z);

		Tangents[k] = Tangent;
		Tangents[k + 1] = Tangent;
		Tangents[k + 2] = Tangent;
	}
	for (size_t k = 0; k < Indices.size(); k++)
	{
		const Math::Vector3& idx = Indices[k];

		vertices.push_back(Positions[(int)idx.x].x);
		vertices.push_back(Positions[(int)idx.x].y);
		vertices.push_back(Positions[(int)idx.x].z);

		vertices.push_back(TextureUVs[(int)idx.y].x);
		vertices.push_back(TextureUVs[(int)idx.y].y);

		vertices.push_back(Normals[(int)idx.z].x);
		vertices.push_back(Normals[(int)idx.z].y);
		vertices.push_back(Normals[(int)idx.z].z);

		vertices.push_back(Tangents[k].x);
		vertices.push_back(Tangents[k].y);
		vertices.push_back(Tangents[k].z);
	}
	return vertices;
}

std::string Utils::Loader::ReadFile(const std::string& path)
{
	std::ifstream in(path, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return(contents);
	}
	else
	{
		PrintError("Failed To Read %s", path.c_str());
		return std::string();
	}
}

const char* Utils::Loader::ReadFile(const char* filename, uint32_t& size, bool& success)
{
	std::ifstream is(filename, std::ifstream::binary);
	if (is.is_open()) {
		success = true;
		// get length of file:
		is.seekg(0, is.end);
		uint32_t length = (uint32_t)is.tellg();
		is.seekg(0, is.beg);

		char* buffer = new char[length];

		// read data as a block:
		is.read(buffer, length);
		is.close();
		size = length;
		return buffer;
	}
	else
	{
		success = false;
		PrintWarning("File %s cannot be found", filename);
		return 0;
	}
}

#pragma region FBX
void LoadMultithread(Resources::Model* model, ofbx::IScene* scene, const std::string& path)
{
	Utils::Loader::FBX::LoadTextures(scene, path);
	Utils::Loader::FBX::LoadMeshes(model, scene, path);
	for (int i = 0, n = scene->getAnimationStackCount(); i < n; ++i)
	{
		Utils::Loader::FBX::LoadAnimation(scene, scene->getAnimationStack(i), path);
		break;
	}
	scene->destroy();
}

Math::Vector2 Utils::Loader::FBX::ToVector2(const ofbx::Vec2& vector)
{
	return { (float)vector.x, (float)vector.y };
}

Math::Vector3 Utils::Loader::FBX::ToVector3(const ofbx::Vec3& vector)
{
	return { (float)vector.x, (float)vector.y, (float)vector.z };
}

void Utils::Loader::FBX::Load(Resources::Model* model, const std::string& path)
{
	if (path.substr(path.find_last_of(".")) != ".fbx") {
		PrintError("%s is not a fbx file !", path.c_str());
		return;
	}
	uint32_t size;
	bool sucess;
	auto data = (ofbx::u8*)Utils::Loader::ReadFile(path.c_str(), size, sucess);
	if (!sucess) {
		delete[] data;
		data = nullptr;
		return;
	}

	auto begin = std::chrono::high_resolution_clock::now();

	PrintLog("Loading Model : %s", path.data());
	ofbx::IScene* Scene = ofbx::load(data, size, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);

	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	PrintLog("Model %s Loading Time : %.5f seconds.", path.data(), elapsed.count() * 1e-9);
	if (Scene)
		LoadMultithread(model, Scene, path);
	delete[] data;
	data = nullptr;
	model->Display(model->MustBeDisplay());
}

void Utils::Loader::FBX::LoadTextures(ofbx::IScene* scene, const std::string& path)
{
	for (int i = 0; i < scene->getEmbeddedDataCount(); i++) {
		const int size = 4096;
		char tmp[size];
		scene->getEmbeddedFilename(i).toString(tmp);
		std::string texpath = ExtractName(tmp);
		auto name = texpath.substr(texpath.find_last_of('/') + 1);
		texpath = path.substr(0, path.find_last_of('/') + 1) + name;
		// If exist then exit, else load it.
		if (!std::filesystem::exists(texpath)) {
			auto embedded = scene->getEmbeddedData(i);
			auto m_handle = (void*)INVALID_HANDLE_VALUE;
			m_handle = (HANDLE)::CreateFileA(texpath.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			assert(INVALID_HANDLE_VALUE != (HANDLE)m_handle);
			int written = 0;
			::WriteFile((HANDLE)m_handle, embedded.begin + 4, (DWORD)(embedded.end - embedded.begin - 4), (LPDWORD)&written, nullptr);
			if (INVALID_HANDLE_VALUE != (HANDLE)m_handle)
			{
				::CloseHandle((HANDLE)m_handle);
				m_handle = (void*)INVALID_HANDLE_VALUE;
			}
		}
		// Texture Creation.
		Core::App::Get().resourcesManager->Create<Resources::Texture>(texpath);
	}
}

void Utils::Loader::FBX::LoadMeshes(Resources::Model* model, ofbx::IScene* scene, const std::string& path)
{
	int count = scene->getMeshCount();
	if (count == 0)
		return;
	for (size_t i = 0; i < count; i++)
	{
		const ofbx::Mesh* ofbxMesh = scene->getMesh((int)i);
		bool hasSkeleton = ofbxMesh->getGeometry()->getSkin() != nullptr;

		Resources::Mesh* Mesh;
		if (!hasSkeleton)
			Mesh = new Resources::Mesh(path + ":" + ofbxMesh->name, Resources::ResourcesType::Mesh);
		else
			Mesh = new Resources::SkeletalMesh(path + ":" + ofbxMesh->name, Resources::ResourcesType::SkeletalMesh);

		std::vector<Math::Vector3> Positions;
		std::vector<Math::Vector3> Normals;
		std::vector<Math::Vector2> TextureUVs;
		std::vector<Math::Vector3> Tangents;
		// Vertices

		if (!hasSkeleton) {
			ofbx::Vec3 translation = ofbxMesh->getLocalTranslation();
			ofbx::Vec3 rot = ofbxMesh->getLocalRotation();
			ofbx::Vec3 sca = ofbxMesh->getLocalScaling();

			ofbx::Vec3 pivot = ofbxMesh->getRotationPivot();

			Mesh->m_rotation = ToVector3(rot).ToQuaternion();
			Mesh->m_translation = Mesh->m_rotation * (ToVector3(translation) * FBXScale);

			for (size_t j = 0; j < ofbxMesh->getGeometry()->getVertexCount(); j++)
			{
				auto vec = ofbxMesh->getGeometry()->getVertices()[j];
				Positions.push_back(ToVector3(vec) * ToVector3(sca) * FBXScale);
				for (size_t i = 0; i < 3; i++)
				{
					if (Positions.back()[i] > model->m_maxXYZ[i])
					{
						model->m_maxXYZ = Positions.back()[i];
					}
				}
			}
		}
		else
		{
			for (size_t j = 0; j < ofbxMesh->getGeometry()->getVertexCount(); j++)
			{
				auto vec = ofbxMesh->getGeometry()->getVertices()[j];
				Positions.push_back(ToVector3(vec));
				for (size_t i = 0; i < 3; i++)
				{
					if (Positions.back()[i] > model->m_maxXYZ[i])
					{
						model->m_maxXYZ = Positions.back()[i];
					}
				}
			}
		}

		// UVs/Normals
		const ofbx::Vec3* normals = ofbxMesh->getGeometry()->getNormals();
		const ofbx::Vec2* uvs = ofbxMesh->getGeometry()->getUVs();
		int indexCount = ofbxMesh->getGeometry()->getIndexCount();
		for (size_t j = 0; j < indexCount; ++j)
		{
			if (!uvs || !normals) {
				PrintError("Error while loading Model (no Uvs or Normals) : %s", path.c_str())
					break;
			}
			ofbx::Vec3 n = normals[j];
			ofbx::Vec2 uv = uvs[j];
			Vector2 textureUV = ToVector2(uv);
			Normals.push_back(ToVector3(n));
			TextureUVs.push_back({ textureUV.x, 1 - textureUV.y });
		}

		// Load Materials
		// Material Loop.
		size_t lastMaterial = 0;
		size_t lastIndex = 0;
		std::unordered_map<int, Material*> mats;
		if (ofbxMesh->getMaterialCount() > 1) {
			for (size_t j = 0; j < indexCount / 3; j++)
			{
				auto currMaterial = ofbxMesh->getGeometry()->getMaterials()[j];
				if (lastMaterial != currMaterial) {
					std::string matName = ofbxMesh->getMaterial((int)lastMaterial)->name;
					std::string matPath = Mesh->GetFullPath().substr(0, Mesh->GetFullPath().find_last_of('/') + 1) + matName + ".mat";

					// Check if already exist.
					Resources::Material* mat;
					mat = LoadMaterial(matPath, ofbxMesh, lastMaterial, path, model);

					Mesh->m_subMeshes.push_back(Resources::SubMesh());
					Mesh->m_subMeshes.back().StartIndex = lastIndex;
					Mesh->m_subMeshes.back().Count = (j * 3 - lastIndex);
					lastIndex = j * 3;
					lastMaterial = ofbxMesh->getGeometry()->getMaterials()[j];
				}
			}
		}
		if (ofbxMesh->getMaterialCount() > 0) {
			std::string matName = ofbxMesh->getMaterial((int)lastMaterial)->name;
			std::string matPath = Mesh->GetFullPath().substr(0, Mesh->GetFullPath().find_last_of('/') + 1) + matName + ".mat";

			// Check if already exist.
			Resources::Material* mat;
			mat = LoadMaterial(matPath, ofbxMesh, lastMaterial, path, model);

		}

		Mesh->m_subMeshes.push_back(Resources::SubMesh());
		Mesh->m_subMeshes.back().StartIndex = lastIndex;
		Mesh->m_subMeshes.back().Count = (size_t)ofbxMesh->getGeometry()->getIndexCount() - lastIndex;

		for (size_t j = 0; j < indexCount; j += 3)
		{
			Math::Vector3 edge1 = Positions[j + 1] - Positions[j];
			Math::Vector3 edge2 = Positions[j + 2] - Positions[j];
			Math::Vector2 deltaUV1 = TextureUVs[j + 1] - TextureUVs[j];
			Math::Vector2 deltaUV2 = TextureUVs[j + 2] - TextureUVs[j];

			float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			Tangents.push_back(Math::Vector3());
			Tangents.back().x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			Tangents.back().y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			Tangents.back().z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			Tangents.push_back(Tangents.back());
			Tangents.push_back(Tangents.back());
		}

		if (hasSkeleton)
		{
			LoadSkeleton(ofbxMesh->getGeometry()->getSkin(), path, model, dynamic_cast<Resources::SkeletalMesh*>(Mesh), indexCount, Positions, TextureUVs, Normals, Tangents);
		}
		else
		{
			for (int j = 0; j < indexCount; j++)
			{
				Mesh->m_indicesPositions.push_back(j);
				Mesh->m_positions.push_back(Positions[j]);
				Mesh->m_vertices.push_back(Positions[j].x);
				Mesh->m_vertices.push_back(Positions[j].y);
				Mesh->m_vertices.push_back(Positions[j].z);
				Mesh->m_vertices.push_back(TextureUVs[j].x);
				Mesh->m_vertices.push_back(TextureUVs[j].y);
				Mesh->m_vertices.push_back(Normals[j].x);
				Mesh->m_vertices.push_back(Normals[j].y);
				Mesh->m_vertices.push_back(Normals[j].z);
				Mesh->m_vertices.push_back(Tangents[j].x);
				Mesh->m_vertices.push_back(Tangents[j].y);
				Mesh->m_vertices.push_back(Tangents[j].z);
			}
		}
		model->AddMesh(Mesh);

		Core::App::Get().resourcesManager->Add(Mesh->GetPath(), Mesh);
		Core::App::Get().AddResourceToSend(Mesh->GetPath());
	}
}

struct BonesVertices
{
	int index;
	float weigth;
};

void Utils::Loader::FBX::LoadSkeleton(const ofbx::Skin* Skel, std::string path, Resources::Model* model, Resources::SkeletalMesh* mesh, int index_count, const std::vector<Math::Vector3>& Positions
	, const std::vector<Math::Vector2>& TextureUVs
	, const std::vector<Math::Vector3>& Normals
	, const std::vector<Math::Vector3>& Tangents)
{
	if (!mesh)
		PrintError("Error mesh is null");

	std::map<int, std::vector<BonesVertices>> BoneWeight;
	std::map<std::string, Resources::Bone*> WaitingBones;
	auto newPath = path + "::" + "Skel";
	auto NewSkel = new Resources::Skeleton(newPath, ResourcesType::Skeleton);
	auto name = path.substr(path.find_last_of('/') + 1);
	name = name + "::" + "Skel";
	NewSkel->p_name = name;
	model->m_skeletons.push_back(NewSkel);
	Resources::ResourcesManager::Get()->Add(NewSkel->GetPath(), NewSkel);

	Bone* root = nullptr;
	std::vector<Bone*> Bones;
	NewSkel->BoneCount = Skel->getClusterCount();
	for (int i = 0; i < Skel->getClusterCount(); i++)
	{
		auto link = Skel->getCluster(i)->getLink();
		Bone* bone = new Bone();
		bone->m_name = link->name;
		bone->Id = i;

		for (int j = 0; j < Skel->getCluster(i)->getWeightsCount(); j++)
		{
			BoneWeight[Skel->getCluster(i)->getIndices()[j]].push_back({ i, (float)Skel->getCluster(i)->getWeights()[j] });
		}

		// Set Up Transform
		auto pos = link->getLocalTranslation();
		auto rot = link->getPreRotation();
		auto sca = link->getLocalScaling();

		Math::Vector3 vecPos = Math::Vector3((float)pos.x, (float)pos.y, (float)pos.z) * 0.01f;
		Math::Vector3 vecRot = Math::Vector3((float)rot.x, (float)rot.y, (float)rot.z);
		Math::Vector3 vecSca = Math::Vector3((float)sca.x, (float)sca.y, (float)sca.z);

		bone->transform->SetLocalPosition(vecPos);
		bone->DefaultPosition = vecPos;
		bone->transform->SetLocalRotation(vecRot.ToQuaternion());
		bone->DefaultRotation = vecRot.ToQuaternion();
		bone->transform->SetLocalScale(vecSca);

		std::string parentName = link->getParent()->name;
		if (parentName != "RootNode")
		{
			Bone* result = nullptr;
			for (auto b : Bones)
			{
				if (b->m_name == parentName)
				{
					result = b;
					break;
				}
			}
			if (result)
				bone->SetParent(result);
			else
			{
				WaitingBones[parentName] = bone;
			}
		}
		else
		{
			root = bone;
		}

		Bones.push_back(bone);
	}
	for (auto& b : WaitingBones)
	{
		for (auto a : Bones)
		{
			if (a->m_name == b.first)
			{
				b.second->SetParent(a);
				break;
			}
		}
	}

	for (auto bone : Bones)
	{
		bone->DefaultMatrix = bone->transform->GetModelMatrix().CreateInverse();
	}

	if (root)
		NewSkel->RootBone = root;

	NewSkel->Bones = Bones;

	auto getIndices = [&](int index, int i) -> int
	{
		if (BoneWeight[index].size() > i)
		{
			auto result = BoneWeight[index].at(i).index;
			return result;
		}
		return 0;
	};

	auto getWeight = [&](int index, int i) -> float
	{
		if (BoneWeight[index].size() > i)
		{
			auto result = BoneWeight[index].at(i).weigth;
			return BoneWeight[index].at(i).weigth;
		}
		return 0.0f;
	};

	for (int i = 0; i < index_count; i++)
	{
		mesh->m_indicesPositions.push_back(i);
		mesh->m_positions.push_back(Positions[i] * 0.01f);
		mesh->m_vertices.push_back(Positions[i].x * 0.01f);
		mesh->m_vertices.push_back(Positions[i].y * 0.01f);
		mesh->m_vertices.push_back(Positions[i].z * 0.01f);
		mesh->m_vertices.push_back(TextureUVs[i].x);
		mesh->m_vertices.push_back(TextureUVs[i].y);
		mesh->m_vertices.push_back(Normals[i].x);
		mesh->m_vertices.push_back(Normals[i].y);
		mesh->m_vertices.push_back(Normals[i].z);
		mesh->m_vertices.push_back(Tangents[i].x);
		mesh->m_vertices.push_back(Tangents[i].y);
		mesh->m_vertices.push_back(Tangents[i].z);
		for (int j = 0; j < 8; j++) {
			mesh->m_vertices.push_back((float)getIndices(i, j));
		}
		for (int j = 0; j < 8; j++) {
			mesh->m_vertices.push_back(getWeight(i, j));
		}
	}
	// Find the maximum size of a vector in the map
	auto result = std::max_element(BoneWeight.begin(), BoneWeight.end(), [](const auto& p1, const auto& p2) {
		return p1.second.size() < p2.second.size();
		});

	if (result->second.size() > 8)
		PrintError("Skeletal Mesh %s is over 8 bone weight : %d", mesh->GetPath().c_str(), result->second.size());

	NewSkel->m_maxBoneWeight = result->second.size();
	NewSkel->isLoaded = true;
	NewSkel->hasBeenSent = true;
}

Resources::Material* Utils::Loader::FBX::LoadMaterial(std::string matPath, const ofbx::Mesh* ofbxMesh, size_t lastMaterial, const std::string& path, Resources::Model* model)
{
	Resources::Material* mat = Core::App::Get().resourcesManager->GetOrLoad<Resources::Material>(matPath);
	if (!mat)
	{
		mat = new Resources::Material(matPath, Resources::ResourcesType::Material);
		mat->p_shouldBeLoaded = true;
		mat->isLoaded = true;
		mat->SetShader(Core::App::Get().resourcesManager->GetDefaultShader());
		auto mesh_mat = ofbxMesh->getMaterial((int)lastMaterial)->getDiffuseColor();
		mat->SetDiffuse(Math::Vector4(mesh_mat.r, mesh_mat.g, mesh_mat.b, 1.f));
		mesh_mat = ofbxMesh->getMaterial((int)lastMaterial)->getAmbientColor();
		mat->SetAmbient(Math::Vector4(mesh_mat.r, mesh_mat.g, mesh_mat.b, 1.f));
		mesh_mat = ofbxMesh->getMaterial((int)lastMaterial)->getSpecularColor();
		mat->SetSpecular(Math::Vector4(mesh_mat.r, mesh_mat.g, mesh_mat.b, 1.f));
		if (auto dif = ofbxMesh->getMaterial((int)lastMaterial)->getTexture(ofbx::Texture::DIFFUSE)) {
			const int size = 4096;
			char tmp[size];
			dif->getRelativeFileName().toString(tmp);
			std::string name = tmp;
			name = path.substr(0, path.find_last_of('/') + 1) + name.substr(name.find_last_of('\\') + 1);
			if (!std::filesystem::exists(name))
			{
				name = name.substr(name.find_last_of('/'));
				name = ENGINEPATH"Textures" + name;
			}
			if (auto tex = Core::App::Get().resourcesManager->GetOrLoad<Resources::Texture>(name))
				mat->SetTexture(tex);
		}
		if (auto dif = ofbxMesh->getMaterial((int)lastMaterial)->getTexture(ofbx::Texture::NORMAL)) {
			const int size = 4096;
			char tmp[size];
			dif->getRelativeFileName().toString(tmp);
			std::string name = tmp;
			name = path.substr(0, path.find_last_of('/') + 1) + name.substr(name.find_last_of('\\') + 1);
			if (!std::filesystem::exists(name))
			{
				name = name.substr(name.find_last_of('/'));
				name = ENGINEPATH"Textures" + name;
			}
			if (auto tex = Core::App::Get().resourcesManager->GetOrLoad<Resources::Texture>(name))
				mat->SetNormalMap(tex);

		}
		// Save material.
		mat->Save();
	}
	if (mat) {
		// Add Material to Model and resource manager
		model->AddMaterial(mat);
		Core::App::Get().resourcesManager->Add(mat->GetPath(), mat);
	}
	return mat;
}

void Utils::Loader::FBX::LoadAnimation(ofbx::IScene* scene, const ofbx::AnimationStack* stack, std::string path)
{
	// Return if no curve node
	if (!stack->getLayer(0)->getCurveNode(0))
		return;
	Resources::Animation* Animation = nullptr;
	auto newPath = path.substr(0, path.find_last_of('.')) + "." + "Anim";
	Animation = new Resources::Animation(newPath, ResourcesType::Animation);
	auto name = newPath.substr(path.find_last_of('/') + 1);
	Animation->p_name = name;
	Animation->FrameRate = scene->getSceneFrameRate();
	if (const ofbx::AnimationLayer* layer = stack->getLayer(0))
	{
		for (int k = 0; layer->getCurveNode(k); ++k)
		{
			// Get the k-th curve node
			const ofbx::AnimationCurveNode* node = layer->getCurveNode(k);

			// Check if the curve node is for translation ("T") or rotation ("R")
			if (!std::strcmp(node->name, "T")) {
				// Add a new vector of Math::Vec3 to the KeyPositions field
				Animation->KeyPositions.push_back(std::unordered_map<int, Math::Vector3>());
				size_t i = 0;

				if (node->getCurve((int)i)) {

					for (int p = 0; p < node->getCurve((int)i)->getKeyCount(); p++) {
						Math::Vector3 Position;
						int keyPosition = -1;
						for (i = 0; i < 3; i++) {
							Position[i] = node->getCurve((int)i)->getKeyValue()[p] * 0.01f;
							if (i == 0)
							{
								auto value = ofbx::fbxTimeToSeconds(node->getCurve((int)i)->getKeyTime()[p]);
								keyPosition = (int)(value * Animation->FrameRate);
							}
						}
						i = 0;
						Animation->KeyPositions.back()[keyPosition] = (Position);
					}
				}
			}
			else if (!std::strcmp(node->name, "R")) {
				// Add a new vector of Math::Quat to the KeyRotations field
				Animation->KeyRotations.push_back(std::unordered_map<int, Math::Quaternion>());
				size_t i = 0;

				if (node->getCurve((int)i)) {

					for (int p = 0; p < node->getCurve((int)i)->getKeyCount(); p++) {
						Math::Vector3 Rotation;
						int keyPosition = -1;
						for (i = 0; i < 3; i++) {
							Rotation[i] = node->getCurve((int)i)->getKeyValue()[p];
							if (i == 0)
							{
								auto value = ofbx::fbxTimeToSeconds(node->getCurve((int)i)->getKeyTime()[p]);
								keyPosition = (int)(value * Animation->FrameRate);
							}
						}
						i = 0;
						Animation->KeyRotations.back()[keyPosition] = (Rotation.ToQuaternion());
					}
				}
			}

		}

		Animation->SendResource();
		ResourcesManager::Get()->Add(Animation->GetPath(), Animation);
		Utils::Loader::ANIM::Save(Animation);
	}
}

#pragma endregion

#pragma region OBJ

void Utils::Loader::OBJ::Load(Resources::Model* model, const std::string& path)
{
	uint32_t size = 0;
	bool sucess = false;
	auto begin = std::chrono::high_resolution_clock::now();
	PrintLog("Loading Model : %s", path.c_str());
	auto data = Utils::Loader::ReadFile(path.c_str(), size, sucess);
	if (sucess)
		Parse(model, data, size);
	else
		PrintError("Failed loaded Model : %s", path.c_str());

	model->Display(model->MustBeDisplay());
	auto end = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
	PrintLog("Model %s Loading Time : %.5f seconds.", path.data(), elapsed.count() * 1e-9);
	delete[] data;
	data = nullptr;
}

void Utils::Loader::OBJ::MtlLoader(std::string path)
{
	// Read File.
	bool sucess;
	uint32_t size = 0;
	auto data = Utils::Loader::ReadFile(path.c_str(), size, sucess);
	if (!sucess)
		return;

	uint32_t pos = 0;
	Resources::Material* currentMaterial = nullptr;
	bool found = false;
	while (pos < size)
	{
		if (data[pos] == 'n')
		{
			if (currentMaterial && !std::filesystem::exists(path))
			{
				currentMaterial->Save();
				currentMaterial = nullptr;
			}
			auto name = GetString(data, pos, 7);
			path = path.substr(0, path.find_last_of('/') + 1) + name + ".mat";
			if (auto mat = ResourcesManager::Get()->GetOrLoad<Resources::Material>(path))
			{
				currentMaterial = mat;
				found = true;
				continue;
			}
			found = false;
			currentMaterial = new Resources::Material(path, ResourcesType::Material);
			ResourcesManager::Get()->Add(path, currentMaterial);
			currentMaterial->p_shouldBeLoaded = true;
			currentMaterial->isLoaded = true;
			//PrintLog("Successfully Loaded Material %s from MTL !", currentMaterial->GetPath().c_str());
		}
		else if (found)
		{
			SkipLine(data, pos);
		}
		else if (data[pos] == 'K' && data[pos + 1] == 'a')
		{
			currentMaterial->SetAmbient(GetVector3(data, pos, 3));
		}
		else if (data[pos] == 'K' && data[pos + 1] == 'd')
		{
			currentMaterial->SetDiffuse(GetVector3(data, pos, 3));
		}
		else if (data[pos] == 'K' && data[pos + 1] == 's')
		{
			currentMaterial->SetSpecular(GetVector3(data, pos, 3));
		}
		else if (data[pos] == 'N' && data[pos + 1] == 's')
		{
			currentMaterial->SetShiniess(GetFloat(data, pos, 3));
		}
		else if (data[pos] == 'd')
		{
			float transparency = GetFloat(data, pos, 2);
			currentMaterial->SetTransparency(transparency);
		}
		else if (data[pos] == 'm' && data[pos + 4] == 'K')
		{
			std::string texPath = GetString(data, pos, 7);
			if (auto texture = ResourcesManager::Get()->Find<Resources::Texture>(texPath))
			{
				currentMaterial->SetTexture(texture);
			}
			else
			{
				texture = ResourcesManager::Get()->Create<Resources::Texture>(texPath);
				currentMaterial->SetTexture(texture);
			}
		}
		else if (data[pos] == 'm' && data[pos + 4] == 'B')
		{
			std::string texPath = GetString(data, pos, 9);
			if (texPath[0] == '-')
			{
				texPath = texPath.substr(texPath.find_last_of(' ') + 1);
			}
			if (auto texture = ResourcesManager::Get()->Find<Resources::Texture>(texPath))
			{
				currentMaterial->SetNormalMap(texture);
			}
			else
			{
				texture = ResourcesManager::Get()->Create<Resources::Texture>(texPath);
				currentMaterial->SetNormalMap(texture);
			}
		}
		else
		{
			SkipLine(data, pos);
		}
	}
	if (currentMaterial && !std::filesystem::exists(path))
	{
		currentMaterial->Save();
		currentMaterial = nullptr;
	}
	delete[] data;
	data = nullptr;
}

void Utils::Loader::OBJ::Parse(Resources::Model* model, const char* data, uint32_t size)
{
	uint32_t pos = 0;

	Resources::Mesh* currentMesh = nullptr;

	std::vector<Math::Vector3> Indices;
	std::vector<Math::Vector3> Positions;
	std::vector<Math::Vector2> TextureUVs;
	std::vector<Math::Vector3> Normals;

	std::vector<Resources::SubMesh> subMeshes;
	Math::Vector3 lastSize;

	while (pos < size)
	{
		if (data[pos] == 'o' || data[pos] == 'g')
		{
			if (currentMesh) {
				lastSize = lastSize + Math::Vector3((float)Positions.size(), (float)TextureUVs.size(), (float)Normals.size());

				if (subMeshes.size() > 0)
					subMeshes.back().Count = Indices.size() - subMeshes.back().StartIndex;
				auto meshDataVertices = ComputeVertices(Positions, TextureUVs, Normals, Indices);
				auto meshVertices = &currentMesh->m_vertices;
				auto meshSubMeshes = &currentMesh->m_subMeshes;
				meshSubMeshes->insert(meshSubMeshes->begin(), subMeshes.begin(), subMeshes.end());
				meshVertices->insert(meshVertices->begin(), meshDataVertices.begin(), meshDataVertices.end());
				Core::App::Get().AddResourceToSend(currentMesh->GetPath());
				subMeshes.clear();
				Indices.clear();
				Positions.clear();
				TextureUVs.clear();
				Normals.clear();
			}

			std::string Name = GetString(data, pos, 2);
			currentMesh = new Resources::Mesh(model->GetPath() + ":" + Name, ResourcesType::Mesh);
			Resources::ResourcesManager::Get()->Add(currentMesh->GetPath(), currentMesh);
			model->AddMesh(currentMesh);
		}
		else if (data[pos] == 'm')
		{
			std::string path = GetString(data, pos, 7);
			path = path.substr(path.find_first_of(' ') + 1);
			path = model->GetPath().substr(0, model->GetPath().find_last_of('/') + 1) + path;
			if (std::filesystem::exists(ResourcesManager::ProjectPath() + '/' + path))
				MtlLoader(ResourcesManager::ProjectPath() + '/' + path);
			else
				MtlLoader(path);
		}
		else if (data[pos] == 'v' && data[pos + 1] == ' ')
		{
			if (!currentMesh)
			{
				std::string Name = model->GetName();
				currentMesh = new Resources::Mesh(model->GetPath() + ":" + Name, ResourcesType::Mesh);
				Resources::ResourcesManager::Get()->Add(currentMesh->GetPath(), currentMesh);
				model->AddMesh(currentMesh);
			}
			Positions.push_back(GetVector3(data, pos, 2));
			currentMesh->m_defaultVertices.push_back(Positions.back().x);
			currentMesh->m_defaultVertices.push_back(Positions.back().y);
			currentMesh->m_defaultVertices.push_back(Positions.back().z);
			currentMesh->m_positions.push_back(Positions.back());
			for (size_t i = 0; i < 3; i++)
			{
				if (Positions.back()[i] > model->m_maxXYZ[i])
				{
					model->m_maxXYZ = Positions.back()[i];
				}
			}
		}
		else if (data[pos] == 'v' && data[pos + 1] == 'n')
		{
			Normals.push_back(GetVector3(data, pos, 3));
			currentMesh->m_defaultVertices.push_back(Normals.back().x);
			currentMesh->m_defaultVertices.push_back(Normals.back().y);
			currentMesh->m_defaultVertices.push_back(Normals.back().z);
		}
		else if (data[pos] == 'v' && data[pos + 1] == 't')
		{
			TextureUVs.push_back(GetVector2(data, pos, 3));
			currentMesh->m_defaultVertices.push_back(TextureUVs.back().x);
			currentMesh->m_defaultVertices.push_back(TextureUVs.back().y);
		}
		else if (data[pos] == 'f')
		{
			// If No use material
			if (subMeshes.size() == 0)
			{
				subMeshes.push_back(SubMesh());
			}
			try
			{
				auto indices = GetIndices(data, pos);
				for (size_t i = 0; i < 3; i++)
				{
					currentMesh->m_indicesPositions.push_back((uint32_t)indices[i].x);
					indices[i] = indices[i] - lastSize;
					Indices.push_back(indices[i]);
				}
			}
			catch (const std::exception& except)
			{
				PrintError("Error while parsing %s : %s", model->GetPath().c_str(), except.what());
			}
		}
		else if (data[pos] == 'u')
		{
			// Set SubMesh
			if (subMeshes.size() > 0)
				subMeshes.back().Count = Indices.size() - subMeshes.back().StartIndex;
			auto subMesh = SubMesh();
			subMesh.StartIndex = Indices.size();

			// Set Name
			std::string MaterialName = GetString(data, pos, 7);
			MaterialName = model->GetPath().substr(0, model->GetPath().find_last_of('/') + 1) + MaterialName + ".mat";
			Resources::Material* mat;
			if (mat = Resources::ResourcesManager::Get()->Find<Resources::Material>(Resources::ResourcesManager::ProjectPath() + MaterialName)) {
				model->AddMaterial(mat);
			}
			else if (mat = Resources::ResourcesManager::Get()->Find<Resources::Material>(MaterialName))
			{
				model->AddMaterial(mat);
			}
			else
			{
				model->AddMaterial(Resources::ResourcesManager::Get()->Find<Resources::Material>(ENGINEPATH"/Materials/DefaultMaterial"));
			}
			subMeshes.push_back(subMesh);
		}
		else
		{
			SkipLine(data, pos);
		}
	}
	if (subMeshes.size() > 0)
	{
		subMeshes.back().Count = Indices.size() - subMeshes.back().StartIndex;
		auto meshDataVertices = ComputeVertices(Positions, TextureUVs, Normals, Indices);
		auto meshVertices = &currentMesh->m_vertices;
		auto meshSubMeshes = &currentMesh->m_subMeshes;
		meshSubMeshes->insert(meshSubMeshes->begin(), subMeshes.begin(), subMeshes.end());
		meshVertices->insert(meshVertices->begin(), meshDataVertices.begin(), meshDataVertices.end());
		Core::App::Get().AddResourceToSend(currentMesh->GetPath());
	}
}
#pragma endregion

#pragma region MAT

void Utils::Loader::MAT::Load(Resources::Material* material, const std::string& path)
{
	uint32_t size = 0;
	bool sucess = false;
	auto data = Utils::Loader::ReadFile(path.c_str(), size, sucess);
	if (sucess)
	{
		Parse(material, data, size);
	}
	delete[] data;
	data = nullptr;
}

void Utils::Loader::MAT::Save(Resources::Material* material)
{
	if (!material)
		return;
	std::string  output;
	if (material->GetShader())
		output += StringFormat("Shader : %s\n", material->GetShader()->GetPath().c_str());
	if (material->GetTexture())
		output += StringFormat("Texture : %s\n", material->GetTexture()->GetPath().c_str());
	if (material->GetNormalMap())
		output += StringFormat("NM : %s\n", material->GetNormalMap()->GetPath().c_str());
	if (material->GetRoughnessMap())
		output += StringFormat("RM : %s\n", material->GetRoughnessMap()->GetPath().c_str());
	else
		output += StringFormat("Roughness : %f\n", material->roughness);
	if (material->GetMetallicMap())
		output += StringFormat("MM : %s\n", material->GetMetallicMap()->GetPath().c_str());
	else
		output += StringFormat("Metallic : %f\n", material->metallic);
	output += StringFormat("Ambient : %s\n", material->GetAmbient().ToString().c_str());
	output += StringFormat("Diffuse : %s\n", material->GetDiffuse().ToString().c_str());
	output += StringFormat("Specular : %s\n", material->GetSpecular().ToString().c_str());
	output += StringFormat("Shininess : %f\n", material->GetShiniess());

	FILE* file;
	fopen_s(&file, material->GetFullPath().c_str(), "w");
	if (file)
	{
		PrintLog("Creating File Material %s!", material->GetFullPath().c_str());
		fputs(output.c_str(), file);
		fclose(file);
	}
}

void Utils::Loader::MAT::Parse(Resources::Material* material, const char* data, uint32_t size)
{
	uint32_t pos = 0;
	while (pos < size)
	{
		if (data[pos] == 'S' && data[pos + 1] == 'h' && data[pos + 2] == 'a')
		{
			auto shaderName = OBJ::GetString(data, pos, 9);
			auto shader = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(shaderName);
			material->SetShader(shader);
		}
		else if (data[pos] == 'T')
		{
			auto textureName = OBJ::GetString(data, pos, 10);
			auto texture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(textureName);
			material->SetTexture(texture);
		}
		else if (data[pos] == 'A')
		{
			Math::Vector4 ambient = GetVector4(data, pos, 10);
			material->SetAmbient(ambient);
		}
		else if (data[pos] == 'D')
		{
			Math::Vector4 diffuse = GetVector4(data, pos, 10);
			material->SetDiffuse(diffuse);
		}
		else if (data[pos] == 'S' && data[pos + 1] == 'p')
		{
			Math::Vector4 specular = GetVector4(data, pos, 11);
			material->SetSpecular(specular);
		}
		else if (data[pos] == 'N' && data[pos + 1] == 'M')
		{
			auto textureName = OBJ::GetString(data, pos, 5);
			auto texture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(textureName);
			material->SetNormalMap(texture);
		}
		else if (data[pos] == 'R' && data[pos + 1] == 'M')
		{
			auto textureName = OBJ::GetString(data, pos, 5);
			auto texture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(textureName);
			material->SetRoughnessMap(texture);
		}
		else if (data[pos] == 'R' && data[pos + 1] == 'o')
		{
			auto roughness = OBJ::GetFloat(data, pos, 12);
			material->roughness = roughness;
		}
		else if (data[pos] == 'M' && data[pos + 1] == 'M')
		{
			auto textureName = OBJ::GetString(data, pos, 5);
			auto texture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(textureName);
			material->SetMetallicMap(texture);
		}
		else if (data[pos] == 'M' && data[pos + 1] == 'e')
		{
			auto metallic = OBJ::GetFloat(data, pos, 11);
			material->metallic = metallic;
		}
		else
		{
			OBJ::SkipLine(data, pos);
		}
	}
	if (!material->GetShader())
		material->SetShader(Resources::ResourcesManager::Get()->GetDefaultShader());
}

#pragma endregion

void Utils::Loader::ANIM::Load(Resources::Animation* anim, const std::string& path)
{
	uint32_t size = 0;
	bool sucess = false;
	auto data = Utils::Loader::ReadFile(path.c_str(), size, sucess);
	if (sucess)
	{
		Parse(anim, data, size);
	}
	delete[] data;
	data = nullptr;
}

void Utils::Loader::ANIM::Save(Resources::Animation* anim)
{
	if (!anim)
		return;
	std::string  output;
	output += "Translation\n";
	for (int i = 0; i < anim->KeyPositions.size(); i++)
	{
		for (int j = 0; j < anim->KeyPositions[i].size(); j++)
		{
			output += StringFormat("%d, %d, %s\n", i, j, anim->KeyPositions[i][j].ToString().c_str());
		}
	}

	output += "Rotation\n";
	for (int i = 0; i < anim->KeyRotations.size(); i++)
	{
		for (int j = 0; j < anim->KeyRotations[i].size(); j++)
		{
			output += StringFormat("%d, %d, %s\n", i, j, anim->KeyRotations[i][j].ToString().c_str());
		}
	}

	output += "FrameRate : " + std::to_string(anim->FrameRate) + '\n';
	output += "KeyCount : " + std::to_string(anim->KeyCount) + '\n';

	FILE* file;
	fopen_s(&file, anim->GetFullPath().c_str(), "w");
	if (file)
	{
		PrintLog("Creating Animation File %s!", anim->GetFullPath().c_str());
		fputs(output.c_str(), file);
		fclose(file);
	}
}

void Utils::Loader::ANIM::Parse(Resources::Animation* anim, const char* data, uint32_t size)
{
	uint32_t pos = 0;
	bool keyposition = true;
	while (pos < size)
	{
		if (data[pos] == 'R')
		{
			keyposition = false;
			OBJ::SkipLine(data, pos);
		}
		else if (data[pos] == 'T')
		{
			keyposition = true;
			OBJ::SkipLine(data, pos);
		}
		else if (data[pos] == 'F')
		{
			anim->FrameRate = GetFloat(data, pos, 12);
		}
		else if (data[pos] == 'K')
		{
			anim->KeyCount = ANIMC::GetInt(data, pos, 11);
		}
		else if (data[pos] >= '0' && data[pos] <= '9')
		{
			if (keyposition)
				GetKeyPos(data, pos, anim);
			else
				GetKeyRot(data, pos, anim);
		}
		else
		{
			pos++;
		}
	}
	anim->hasBeenSent = true;
}

Math::Vector3 Utils::Loader::ANIM::GetVector3(const char* data, uint32_t& pos, int dec)
{
	Vector3 position;
	std::string str;
	pos += dec;

	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
	{
		str.push_back(data[pos]);
		pos++;
	}
	position.x = std::stof(str);
	str.clear();
	pos++;

	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
	{
		str.push_back(data[pos]);
		pos++;
	}
	position.y = std::stof(str);
	str.clear();
	pos++;

	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
	{
		str.push_back(data[pos]);
		pos++;
	}
	position.z = std::stof(str);
	str.clear();
	pos++;

	return position;

}

int Utils::Loader::ANIM::GetInt(const char* data, uint32_t& pos, int dec)
{
	pos += dec;
	std::string str;
	while (data[pos] != ',')
	{
		str.push_back(data[pos]);
		pos++;
	}
	return std::stoi(str);
}

int Utils::Loader::ANIM::GetInt2(const char* data, uint32_t pos, int dec)
{
	pos += dec;
	std::string str;
	while (data[pos] != ',')
	{
		str.push_back(data[pos]);
		pos++;
	}
	return std::stoi(str);
}

float Utils::Loader::ANIM::GetFloat(const char* data, uint32_t& pos, int dec)
{
	pos += dec;
	std::string str;
	while (data[pos] != '\n' && data[pos] != '\0')
	{
		str.push_back(data[pos]);
		pos++;
	}
	return std::stof(str);
}

inline void Utils::Loader::ANIM::GetKeyPos(const char* data, uint32_t& pos, Resources::Animation* anim)
{
	int currentIndex = GetInt2(data, pos, 0);
	int currentKey = 0;
	Core::App::Get().threadManager->Lock();
	anim->KeyPositions.push_back(std::unordered_map<int, Math::Vector3>());
	while (data[pos] >= '0' && data[pos] <= '9' && GetInt2(data, pos, 0) == currentIndex) {
		currentIndex = GetInt(data, pos, 0);
		currentKey = GetInt(data, pos, 1);

		auto vector = GetVector3(data, pos, 2);
		anim->KeyPositions.back()[currentKey] = vector;
	}
	Core::App::Get().threadManager->Unlock();
}

inline void Utils::Loader::ANIM::GetKeyRot(const char* data, uint32_t& pos, Resources::Animation* anim)
{
	int currentIndex = GetInt2(data, pos, 0);
	int currentKey = 0;
	Core::App::Get().threadManager->Lock();
	anim->KeyRotations.push_back(std::unordered_map<int, Math::Quaternion>());
	while (data[pos] >= '0' && data[pos] <= '9' && GetInt2(data, pos, 0) == currentIndex) {
		currentIndex = GetInt(data, pos, 0);
		currentKey = GetInt(data, pos, 1);

		auto vector = MAT::GetVector4(data, pos, 2);
		anim->KeyRotations.back()[currentKey] = vector;
	}
	Core::App::Get().threadManager->Unlock();
}

void Utils::Loader::ANIMC::Load(Resources::AnimationController* animC, const std::string& path)
{
	uint32_t size = 0;
	bool sucess = false;
	auto data = Utils::Loader::ReadFile(path.c_str(), size, sucess);
	if (sucess)
	{
		Parse(animC, data, size);
	}
	delete[] data;
	data = nullptr;
}

void Utils::Loader::ANIMC::Save(Resources::AnimationController* animC)
{
	std::string  output;
	output += "Parameters :\n";
	for (auto& parameter : animC->parameters)
	{
		output += parameter.first + ": " + std::to_string(parameter.second) + '\n';
	}
	output += "EndParameters\n";
	for (auto& state : animC->states)
	{
		output += "State : " + state.second->name + '\n';
		if (state.second->animation)
			output += "\tAnimation : " + state.second->animation->GetPath() + '\n';
		output += "\tSpeed : " + std::to_string(state.second->speed) + '\n';
		output += "\tLoop : " + std::to_string(state.second->loop) + '\n';
		output += "\tPos : " + state.second->pos.ToString() + '\n';
		output += "\tColor : " + state.second->color.ToString() + '\n';
		output += "EndState\n";
	}
	for (auto& link : animC->links)
	{
		output += "Link :\n";
		output += "\tState 1 : " + link->state1->name + '\n';
		output += "\tState 2 : " + link->state2->name + '\n';
		output += "\tHET : " + std::to_string(link->hasExitTime) + '\n';
		output += "\tET : " + std::to_string(link->exitTime) + '\n';
		output += "\tTD : " + std::to_string(link->transitionDuration) + '\n';
		output += "\tConditions :\n";
		for (auto& condition : link->conditions)
		{
			output += "\t\t" + condition.first->first + '\n';
			output += "\t\t" + std::to_string(condition.second) + '\n';
		}
		output += "\tEndConditions\n";
		output += "EndLink\n";
	}

	FILE* file;
	fopen_s(&file, animC->GetFullPath().c_str(), "w");
	if (file)
	{
		PrintLog("Creating Animation Controller File %s!", animC->GetFullPath().c_str());
		fputs(output.c_str(), file);
		fclose(file);
	}
}

void Utils::Loader::ANIMC::Parse(Resources::AnimationController* animC, const char* data, uint32_t size)
{
	uint32_t pos = 0;
	bool keyposition = true;
	while (pos < size)
	{
		if (data[pos] == 'S')
		{
			auto state = GetState(data, pos);
			animC->states[state->name] = (state);
		}
		else if (data[pos] == 'L')
		{
			auto link = GetLink(animC, data, pos);
			animC->links.push_back(link);
		}
		else if (data[pos] == 'P')
		{
			GetParameters(animC, data, pos);
		}
		else
		{
			OBJ::SkipLine(data, pos);
		}
	}
}

Resources::StateRect* Utils::Loader::ANIMC::GetState(const char* data, uint32_t& pos)
{
	StateRect* state = new StateRect();
	while (data[pos] != 'E')
	{
		if (data[pos] == 'S' && data[pos + 1] == 't')
		{
			auto name = OBJ::GetString(data, pos, 8);
			state->name = name;
		}
		else if (data[pos] == 'A')
		{
			auto animation = OBJ::GetString(data, pos, 12);
			if (auto anim = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Animation>(animation))
				state->animation = anim;
		}
		else if (data[pos] == 'S' && data[pos + 1] == 'p')
		{
			auto speed = OBJ::GetFloat(data, pos, 8);
			state->speed = speed;
		}
		else if (data[pos] == 'P')
		{
			auto position = GetVector2(data, pos, 6);
			state->pos = position;
		}
		else if (data[pos] == 'C')
		{
			auto color = MAT::GetVector4(data, pos, 8);
			state->color = color;
		}
		else if (data[pos] == 'L')
		{
			bool loop = GetInt(data, pos, 7);
			state->loop = loop;
		}
		else
		{
			SkipLine(data, pos);
		}
	}
	return state;
}

inline Resources::Link* Utils::Loader::ANIMC::GetLink(Resources::AnimationController* animC, const char* data, uint32_t& pos)
{
	Link* link = new Link();
	while (data[pos] != 'E' || data[pos + 1] != 'n')
	{
		if (data[pos] == 'S')
		{
			int index = GetInt(data, pos, 6);
			auto name = OBJ::GetString(data, pos, 3);
			if (index == 1)
			{
				link->state1 = animC->GetStateByName(name);
			}
			else if (index == 2)
			{
				link->state2 = animC->GetStateByName(name);
			}
		}
		else if (data[pos] == 'H')
		{
			int HET = GetInt(data, pos, 6);
			link->hasExitTime = HET;
		}
		else if (data[pos] == 'E')
		{
			float exitTime = OBJ::GetFloat(data, pos, 5);
			link->hasExitTime = exitTime;
		}
		else if (data[pos] == 'T')
		{
			float td = OBJ::GetFloat(data, pos, 5);
			link->transitionDuration = td;
		}
		else if (data[pos] == 'C')
		{
			GetConditions(animC, link, data, pos);
		}
		else
		{
			SkipLine(data, pos);
		}
	}
	link->animC = animC;
	return link;
}

void Utils::Loader::ANIMC::GetParameters(Resources::AnimationController*& animC, const char* data, uint32_t& pos)
{
	SkipLine(data, pos);
	while (data[pos] != 'E')
	{
		auto name = GetString(data, pos, 0);
		pos++;
		auto value = GetInt(data, pos, 0);
		animC->parameters[name] = value;
		SkipLine(data, pos);
	}
}

inline void Utils::Loader::ANIMC::GetConditions(Resources::AnimationController*& animC, Link*& link, const char* data, uint32_t& pos)
{
	SkipLine(data, pos);
	SkipLine(data, pos);
	SkipLine(data, pos);
	while (data[pos] != 'E')
	{
		auto name = GetString(data, pos, 0);
		pos += 2;
		bool value = GetInt(data, pos, 0);
		if (animC->parameters.count(name))
			link->conditions.push_back(std::make_pair(&(*animC->parameters.find(name)), value));
		SkipLine(data, pos);
		SkipLine(data, pos);
		if (data[pos] == '\t')
			SkipLine(data, pos);
	}
}

void Utils::Loader::ANIMC::SkipLine(const char* data, uint32_t& pos)
{
	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != '\t')
	{
		pos++;
	}
	pos++;
}

Math::Vector2 Utils::Loader::ANIMC::GetVector2(const char* data, uint32_t& pos, int dec)
{
	Vector2 vector2;
	std::string str;
	pos += dec;

	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
	{
		str.push_back(data[pos]);
		pos++;
	}
	vector2.x = std::stof(str);
	str.clear();
	pos++;

	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
	{
		str.push_back(data[pos]);
		pos++;
	}
	vector2.y = std::stof(str);
	str.clear();
	pos++;
	return vector2;
}

std::string Utils::Loader::ANIMC::GetString(const char* data, uint32_t& pos, int dec)
{
	std::string name;
	pos += dec;
	while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != '\r' && data[pos] != ':')
	{
		name.push_back(data[pos]);
		pos++;
	}
	pos++;
	return name;
}

int Utils::Loader::ANIMC::GetInt(const char* data, uint32_t& pos, int dec)
{
	pos += dec;
	std::string str;
	while (data[pos] != ' ' && data[pos] != '\0' && data[pos] != '\n')
	{
		str.push_back(data[pos]);
		pos++;
	}
	return std::stoi(str);
}
