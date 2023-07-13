#pragma once
#include "PandorAPI.h"

#include <string>
#include <vector>
#include <Math/Maths.h>

namespace ofbx
{
	struct IScene;
	struct Vec2;
	struct Vec3;
	struct Mesh;
	struct Skin;
	struct AnimationStack;
}
namespace Resources {
	class Model;
	class SkeletalMesh;
	class Material;
	class Animation;
	class AnimationController;
	struct StateRect;
	struct Link;
}
namespace Utils
{
	namespace Loader {
		PANDOR_API std::string ReadFile(const std::string& path); 
		PANDOR_API const char* ReadFile(const char* filename, uint32_t& size, bool& success);

		inline std::string GetLine(const char* data, uint32_t& pos)
		{
			std::string line;
			while (data[pos] != '\0' && data[pos] != '\n')
			{
				line.push_back(data[pos]);
				pos++;
			}
			if (line[0] == '\t')
				line = line.substr(line.find_first_not_of('\t'));
			return line;
		}

		class PANDOR_API FBX {
		public:
			static Vector2 ToVector2(const ofbx::Vec2& vector);
			static Vector3 ToVector3(const ofbx::Vec3& vector);
			static void Load(Resources::Model* model, const std::string& path);
			// This function will create the texture into the path of the fbx and load into resourceManager.
			static void LoadTextures(ofbx::IScene* scene, const std::string& path);
			static void LoadMeshes(Resources::Model* model, ofbx::IScene* scene, const std::string& path);
			static void LoadSkeleton(const ofbx::Skin* Skel, std::string path, Resources::Model* model, Resources::SkeletalMesh* mesh, int index_count
				, const std::vector<Math::Vector3>& Positions
				, const std::vector<Math::Vector2>& TextureUVs
				, const std::vector<Math::Vector3>& Normals
				, const std::vector<Math::Vector3>& Tangents);
			static Resources::Material* LoadMaterial(std::string matPath, const ofbx::Mesh* ofbxMesh, size_t lastMaterial, const std::string& path, Resources::Model* model);
			static void LoadAnimation(ofbx::IScene* scene, const ofbx::AnimationStack* stack, std::string path);

		};

		class PANDOR_API OBJ {
		public:
			static void Load(Resources::Model* model, const std::string& path);
			static void Parse(Resources::Model* model, const char* data, uint32_t size);
			static void MtlLoader(std::string path);

			static inline std::string GetString(const char* data, uint32_t& pos, int dec)
			{
				std::string name;
				pos += dec;
				while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != '\r')
				{
					name.push_back(data[pos]);
					pos++;
				}
				pos++;
				return name;
			}

			static inline void SkipLine(const char* data, uint32_t& pos)
			{
				while (data[pos] != '\0' && data[pos] != '\n')
				{
					pos++;
				}
				pos++;
			}

			static inline Math::Vector2 GetVector2(const char* data, uint32_t& pos, int dec)
			{
				Math::Vector2 texUV;
				std::string str;
				pos += dec;

				while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
				{
					str.push_back(data[pos]);
					pos++;
				}
				texUV.x = std::stof(str);
				str.clear();
				pos++;

				while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
				{
					str.push_back(data[pos]);
					pos++;
				}
				texUV.y = 1.f - std::stof(str);
				str.clear();
				pos++;

				return texUV;
			}

			static inline Math::Vector3 GetVector3(const char* data, uint32_t& pos, int dec)
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

			static inline std::vector<Math::Vector3> GetIndices(const char* data, uint32_t& pos)
			{
				std::vector<Math::Vector3> index;
				std::string str;
				int i = 0;
				pos += 2;
				while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != '\r') {
					index.push_back(Math::Vector3());
					while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ' && data[pos] != '/')
					{
						str.push_back(data[pos]);
						pos++;
					}
					index[i].x = (float)(std::stoi(str) - 1);
					str.clear();
					pos++;

					while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ' && data[pos] != '/')
					{
						str.push_back(data[pos]);
						pos++;
					}
					index[i].y = (float)(std::stoi(str) - 1);

					str.clear();
					pos++;

					while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ' && data[pos] != '/')
					{
						str.push_back(data[pos]);
						pos++;
					}
					index[i].z = (float)(std::stoi(str) - 1);
					str.clear();
					i++;
					if (data[pos] == ' ')
						pos++;
				}
				return index;
			}

			static inline float GetFloat(const char* data, uint32_t& pos, int dec)
			{
				std::string str;
				pos += dec;
				while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
				{
					str.push_back(data[pos]);
					pos++;
				}
				return std::stof(str);
			}
		};

		class PANDOR_API MAT
		{
		public:
			static void Load(Resources::Material* material, const std::string& path);
			static void Save(Resources::Material* material);
			static void Parse(Resources::Material* material, const char* data, uint32_t size);

			static inline Math::Vector4 GetVector4(const char* data, uint32_t& pos, int dec)
			{
				Vector4 position;
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

				while (data[pos] != '\0' && data[pos] != '\n' && data[pos] != ' ')
				{
					str.push_back(data[pos]);
					pos++;
				}
				position.w = std::stof(str);
				str.clear();
				pos++;

				return position;
			}
		};

		class PANDOR_API ANIM
		{
		public:
			static void Load(Resources::Animation* anim, const std::string& path);
			static void Save(Resources::Animation* anim);
			static void Parse(Resources::Animation* anim, const char* data, uint32_t size); 
			static inline Math::Vector3 GetVector3(const char* data, uint32_t& pos, int dec);
			static inline int GetInt(const char* data, uint32_t& pos, int dec);
			static inline int GetInt2(const char* data, uint32_t pos, int dec);
			static inline float GetFloat(const char* data, uint32_t& pos, int dec);
			static inline void GetKeyPos(const char* data, uint32_t& pos, Resources::Animation* anim);
			static inline void GetKeyRot(const char* data, uint32_t& pos, Resources::Animation* anim);
		};

		class PANDOR_API ANIMC
		{
		public:
			static void Load(Resources::AnimationController* anim, const std::string& path);
			static void Save(Resources::AnimationController* anim);
			static void Parse(Resources::AnimationController* anim, const char* data, uint32_t size);
			static inline Resources::StateRect* GetState(const char* data, uint32_t& pos);
			static inline Resources::Link* GetLink(Resources::AnimationController* animC, const char* data, uint32_t& pos);
			static inline void GetParameters(Resources::AnimationController*& animC, const char* data, uint32_t& pos);
			static inline void GetConditions(Resources::AnimationController*& animC, Resources::Link*& link, const char* data, uint32_t& pos);
			static inline void SkipLine(const char* data, uint32_t& pos);
			static inline Math::Vector2 GetVector2(const char* data, uint32_t& pos, int dec);
			static inline std::string GetString(const char* data, uint32_t& pos, int dec);
			static inline int GetInt(const char* data, uint32_t& pos, int dec);
		};
	}
}