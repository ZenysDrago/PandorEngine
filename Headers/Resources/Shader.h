#pragma once
#include "PandorAPI.h"

#include "IResources.h"
#include "mutex"
#include <unordered_map>

namespace Resources
{
	class VertexShader;
	class FragmentShader;
	
	struct PANDOR_API ShaderVariables
	{
		int index;
		std::string name;
		int location;
		unsigned int type;
		int size;
	};

	/* ===================== */
	/*      Shader class     */
	/* ===================== */
	
	class PANDOR_API Shader : public IResources
	{
	private :
		VertexShader* vertShader = nullptr;
		FragmentShader* fragShader = nullptr;

		bool isFragLoaded = false;
		bool isVertLoaded = false;
		std::unordered_map<std::string, int> m_locations;
		std::mutex shaderMutex;
		std::vector<ShaderVariables> variables;

		bool uniformSet;
	public:
		unsigned int ID = -1;

		Shader(const std::string& _name,const std::string& vertexPath,const std::string& fragmentPath,ResourcesType _type);
		Shader(const std::string& _name, ResourcesType _type);

		~Shader() override;
		void Load() override;
		void SendResource() override;

		void Recompile();

		int GetLocation(const std::string& locationName);
		VertexShader* GetVertex() { return vertShader; }
		FragmentShader* GetFrag() { return fragShader; }
		
		void Use();
		void Delete();

		void CheckResourceLoaded();
		void SetFragLoaded(bool _isFragLoaded);
		void SetVertLoaded(bool _isVertLoaded);

		bool GetUniformSet() { return uniformSet; }
		void SetUniformSet(bool uniformAreSet = true) { uniformSet = uniformAreSet; }

		std::vector<ShaderVariables>& GetShaderVariables() { return variables; }
		static ResourcesType GetResourceType() { return ResourcesType::Shader; }

	};

	/* ===================== */
	/* Fragment Shader class */
	/* ===================== */

	class PANDOR_API FragmentShader : public IResources
	{
	private :
		std::deque<Shader*> m_shaders;
	public :
		std::string fragmentFile;

		FragmentShader(std::string _name,ResourcesType _type);

		~FragmentShader() override;
		void Load() override;
		void SendResource() override;

		void Recompile();

		void AddShader(Shader* _shader) { m_shaders.push_back(_shader); }
		
		std::deque<Shader*> GetShaders() const {return m_shaders ;}
		void SetSendingStatus(bool _hasBeenSent) { hasBeenSent = _hasBeenSent; }

		static ResourcesType GetResourceType() { return ResourcesType::FragmentShader; }
	};

	/* ===================== */
	/*  Vertex Shader class  */
	/* ===================== */
	
	class PANDOR_API VertexShader : public IResources
	{
	private:
		std::deque<Shader*> m_shaders;
		
	public :
		std::string vertexFile;
		VertexShader(std::string _name,ResourcesType _type);

		~VertexShader() override;
		void Load() override;
		void SendResource() override;

		void Recompile();

		void AddShader(Shader* _shader) { m_shaders.push_back(_shader); }

		std::deque<Shader*> GetShaders() const { return m_shaders; }
		void SetSendingStatus(bool _hasBeenSent) { hasBeenSent = _hasBeenSent; }

		static ResourcesType GetResourceType() { return ResourcesType::VertexShader; }
	};
}
