#pragma once
#include "PandorAPI.h"
#include <Resources/IResources.h>
#include <vector>
#include <unordered_map>

namespace Utils::Loader
{
	class OBJ;
	class FBX;
	class ANIM;
}

namespace Resources
{
	class PANDOR_API Animation : public IResources
	{
	public:
		Animation(std::string _path, Resources::ResourcesType _type) : IResources(_path, _type) {};
		~Animation();

		void Load() override;
		void SendResource() override;

		void GetAnimAtFrame(int id, float time, Math::Vector3& Position, Math::Quaternion& Rotation);

		float FrameRate = 0;

		size_t KeyCount = 0;

		// List of all Bones, With all Key Inside with :
		// - first = Current Keyframe
		// - second = Transform at this keyFrame
		std::vector < std::unordered_map<int, Math::Vector3>> KeyPositions;
		std::vector < std::unordered_map<int, Math::Quaternion>> KeyRotations;

		static ResourcesType GetResourceType() { return ResourcesType::Animation; }
	private:
		friend class Utils::Loader::FBX;
		friend class Utils::Loader::ANIM;
	};
}