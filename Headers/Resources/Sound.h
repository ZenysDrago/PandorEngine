#pragma once
#include "PandorAPI.h"

#include <string>
#include "IResources.h"
#include <Core/Wrappers/WrapperAudio.h>

namespace Resources
{
	class PANDOR_API Sound : public IResources , public Core::Wrapper::WrapperAudio::Audio
	{
	private:
	public:
 
	private :
	public:
		Sound(std::string _name, ResourcesType _type);
		~Sound();

		void Load() override;
		void SendResource() override;

		static ResourcesType GetResourceType() { return ResourcesType::Sound; }

	};
}