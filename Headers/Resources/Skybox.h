#pragma once
#include "PandorAPI.h"

#include <string>
#include "IResources.h"

namespace Resources
{
	class PANDOR_API Skybox : public IResources
	{
	protected:
		virtual void SpecificLoad() = 0;
		virtual void SpecificSend() = 0;

	public:

		unsigned int ID = 0;

		Skybox(std::string _name, ResourcesType _type);

		~Skybox() override;
		virtual void Load() override;
		void SendResource() override;

		void SetSent() { hasBeenSent = true; }

		void TexUni(class Shader*& shader, const char* uniform, unsigned int unit);
		void Bind();
		void UnBind();
		void Delete();

		void Active(int index);

		void ShowInInspector() override;

		void Render(const Math::Matrix4& VP);

		static ResourcesType GetResourceType() { return ResourcesType::Skybox; }
	};

	class PANDOR_API CubeMap : public Skybox
	{
	private:
		unsigned char* m_texData;
		int m_texWidth, m_texHeight, m_nrChannels;

	protected:
		void SpecificLoad() override;
		void SpecificSend() override;

	public:
		CubeMap(std::string _name, ResourcesType _type) : Skybox(_name, _type) {}
		~CubeMap();
		void Load() override { Skybox::Load(); }
	};

	class PANDOR_API SixSided : public Skybox
	{
	private:
		std::string m_faces[6];
		unsigned char* m_texData[6];
		int m_texWidths[6], m_texHeights[6];

	protected:
		void SpecificLoad() override;
		void SpecificSend() override;

	public:
		SixSided(std::string _name, std::string faces[6]);
		~SixSided();
	};
}
