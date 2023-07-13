#pragma once
#include "PandorAPI.h"

#include <vector>
#include <string>
#include <Windows.h>
#include <Math/Maths.h>

namespace Resources
{
	class Shader;
	class Texture;
}

namespace Render
{
	class Camera;
}

struct GLFWwindow;

namespace Core::Wrapper
{
	namespace WrapperRHI
	{
		/* Handle define to rewrite some Graphic API define */

#define PR_TEXTURE2D		0x0DE1
#define PR_TEXTURE0			0x84C0
#define PR_TEXTURE_CUBE_MAP 0x8513
#define PR_UNSIGNED_BYTE	0x1401
#define PR_RGB				0x1907
#define PR_RGBA				0x1908

#define PR_VECTOR2			0x8B50
#define PR_VECTOR3			0x8B51
#define PR_VECTOR4			0x8B52
#define PR_MAT4			    0x8B5C
#define PR_BOOL			    0x8B56
#define PR_FLOAT			0x1406
#define PR_INT				0x1404
#define	PR_UNPACK_ALIGNMENT 0x0CF5
#define	PR_REPEAT			0x2901
#define	PR_MIRRORED_REPEAT  0x8370
#define	PR_CLAMP_TO_EDGE    0x812F
#define	PR_CLAMP_TO_BORDER  0x812D
#define PR_NEAREST			0x2600
#define PR_LINEAR			0x2601

		class PANDOR_API Buffer
		{
		public:
			unsigned int VertexArray, VertexBuffer, IndexBuffer;

			Buffer() {}
			Buffer(float* vertices, size_t numVertices, unsigned int* indices, size_t numIndices);
			Buffer(float* vertices, size_t numVertices);

			void InitializePlane();
			void GenVertexBuffer();

			void LinkAttribute(unsigned int layout, unsigned int numComponents, unsigned int type, unsigned int stride, void* offset);
			void AttribDivisor(unsigned int index, unsigned int divisor);
			void Bind();
			void BindVertexBuffer();
			static void BufferSubData(unsigned int offset, size_t size, const void* data);
			void BufferData(size_t size, const void* data);
			void Unbind();
			void UnbindVertexBuffer();
			void Delete();
		};

		PANDOR_API void InitializeAPI();
		PANDOR_API void EnableDebugOutput();
		PANDOR_API void ClearColorAndBuffer(Vector4 clearColor);
		PANDOR_API void ClearColor(Vector4 clearColor);
		PANDOR_API void DrawElements(int count);
		PANDOR_API void DrawArrays(size_t start, size_t count, bool wireframe = false, bool cullface = true);
		PANDOR_API void DrawInstance(size_t first, size_t count, size_t number);
		PANDOR_API void CreateBuffer(Buffer*& buffer, float* vertices, size_t verticesSize, unsigned int* indices, size_t indicesSize);
		PANDOR_API void DepthActive();
		PANDOR_API void DepthDisable();
		PANDOR_API void CameraData(Resources::Shader*& shaderData, const char* name, Math::Matrix4 VP);
		PANDOR_API void SendTexture(unsigned int& ID, unsigned int format, unsigned int texType, unsigned int slot, unsigned int pixelType, const int& widthImg, const int& heightImg, unsigned char* bytes, int filter = PR_NEAREST);
		PANDOR_API void GenVertex(unsigned int& VAO, unsigned int& VBO, float vertices[], std::size_t size);
		PANDOR_API void BindVAO(unsigned int& VAO);
		PANDOR_API void SetDephtFunc(bool value);
		PANDOR_API void SendCubeMap(unsigned int& ID, int texWidth, int texHeight, int nrChannels, unsigned char* texData);
		PANDOR_API void SendSixSided(unsigned int& ID, int texWidths[6], int texHeights[6], unsigned char* texData[6]);
		PANDOR_API void UpdateTexture(unsigned int& ID, int filter = PR_NEAREST, int wrap = PR_REPEAT);
		PANDOR_API bool SendShader(std::string& vertShader, std::string& fragShader, unsigned int& ID, Resources::Shader* shader);
		PANDOR_API void GetAllUniform(Resources::Shader* shaderData);
		PANDOR_API void StencilActive();
		PANDOR_API void UseStencil();
		PANDOR_API void DrawStencil(Resources::Shader* shaderData, Vector4 Color);
		PANDOR_API void MaskStencil();
		PANDOR_API void DisableStencil();
		PANDOR_API void DepthRange(Vector2 r);

		PANDOR_API void ShaderUse(unsigned int& ID);
		PANDOR_API void ShaderDelete(unsigned int& ID);
		PANDOR_API int	ShaderGetLocation(unsigned int& ID, const char* name);

		PANDOR_API void ShaderSendSampler(const int shaderProgram, const char* name, const int value);
		PANDOR_API void ShaderSendInt(const int shaderProgram, const char* name, const int value);
		PANDOR_API void ShaderSendFloat(const int shaderProgram, const char* name, const float value);
		PANDOR_API void ShaderSendVec2(const int shaderProgram, const char* name, const Math::Vector2& value);
		PANDOR_API void ShaderSendVec3(const int shaderProgram, const char* name, const Math::Vector3& value);
		PANDOR_API void ShaderSendVec4(const int shaderProgram, const char* name, const Math::Vector4& value);
		PANDOR_API void ShaderSendMat4(const int shaderProgram, const char* name, const Math::Matrix4& value, bool transpose = true);
		PANDOR_API void ShaderSendBool(const int shaderProgram, const char* name, const bool& value);


		PANDOR_API inline void ShaderSendSampler(int location, const int value);
		PANDOR_API inline void ShaderSendInt(int location, const int value);
		PANDOR_API inline void ShaderSendFloat(int location, const float value);
		PANDOR_API inline void ShaderSendVec3(int location, const Math::Vector3& value);
		PANDOR_API inline void ShaderSendMat4(int location, const Math::Matrix4& value, bool transpose = true);
		PANDOR_API inline void ShaderSendMat4(int location, const std::vector<Math::Matrix4>& value, int count, bool transpose = true);
		PANDOR_API inline void ShaderSendVec2(int location, const Math::Vector2& value);
		PANDOR_API inline void ShaderSendVec4(int location, const Math::Vector4& value);
		PANDOR_API inline void ShaderSendBool(int location, const bool& value);

		PANDOR_API void ShaderGetSampler(const int shaderID, const int& index, int* value);
		PANDOR_API void ShaderGetInt(const int shaderID, const int& index, int* value);
		PANDOR_API void ShaderGetFloat(const int shaderID, const int& index, float* value);
		PANDOR_API void ShaderGetVec2(const int shaderID, const int& index, float* value);
		PANDOR_API void ShaderGetVec3(const int shaderID, const int& index, float* value);
		PANDOR_API void ShaderGetVec4(const int shaderID, const int& index, float* value);
		PANDOR_API void ShaderGetMat4(const int shaderID, const int& index, float* value);
		PANDOR_API void ShaderGetBool(const int shaderID, const int& index, bool* value);

		PANDOR_API void GenerateTexture(unsigned int& tex);
		PANDOR_API void TextureData(Resources::Shader* shader, const char* name, unsigned int unit);
		PANDOR_API void TextureBind(unsigned int ID, unsigned int type = PR_TEXTURE2D);
		PANDOR_API void TextureUnBind(unsigned int type = PR_TEXTURE2D);
		PANDOR_API void TextureDelete(unsigned int& ID);
		PANDOR_API void ActivateTexture(unsigned int index = 0);
		PANDOR_API void FontTexture(unsigned int width, unsigned int rows, unsigned char* buffer);

		PANDOR_API const char* GetVersion();
		PANDOR_API const char* GetVendor();
		PANDOR_API const char* GetRenderer();

		PANDOR_API void PushToGPU(unsigned char* data, int x, int y);
		PANDOR_API void ViewPort(int x, int y, int sizeX, int sizeY);

		PANDOR_API void PixelStore(unsigned int type, int value);

		static void APIENTRY DebugOutput(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam);

		//FrameBuffer
		class PANDOR_API RenderBuffer
		{
		public:
			RenderBuffer();
			~RenderBuffer();

			void Generate(unsigned int& texture, const Math::Vector2& size, unsigned int texUnit = 0);

			void Resize(const Math::Vector2& windowSize);
			void Bind();
			void UnBind();

			void Read(int width, int height, unsigned char*& data);


			unsigned int renderBuffer, frameBuffer;

		private:

		};

		class PANDOR_API Line
		{
		public:
			Line();
			Line(float lineWidth);
			~Line();

			static inline Line& Get() { return *m_line.get(); }

			void Initialize();
			void Draw(Math::Vector3 p1, Math::Vector3 p2);
			void DrawCube(const Math::Vector3& position, const Math::Vector3& extent, const Vector4& color = Vector4(1, 1, 1, 1));
			void DrawCircle(const Math::Vector3& position, const Math::Vector3& direction, float radius, int segment = 64, const Vector4& color = Vector4(1, 1, 1, 1));
			void DrawCone(const Math::Vector3& position, const Math::Quaternion& rotation, float topRadius, float angle, float height = 25.f, const Vector4& color = Vector4(1, 1, 1, 1));
			Math::Vector4 Color = Math::Vector4(1, 0, 0, 1);
			float LineWidth = 1.f;

			void ReloadShader();
		private:
			Math::Vector3 _point1;
			Math::Vector3 _point2;

			unsigned int m_VAO = 0;
			unsigned int m_VBO = 0;
			Resources::Shader* m_shader = nullptr;
			bool m_initialized = false;

			static std::unique_ptr<Line> m_line;
		};

		class PANDOR_API ShadowMap
		{
		private:
			unsigned int w;
			unsigned int h;
			unsigned int m_shadowMap;
			unsigned int m_buffer;
			const unsigned int SHADOW_WIDTH = 3048, SHADOW_HEIGHT = 3048;
		public: 
			bool stencilDepthTest;
			void InitShadow();
			void BeginShadowMapGeneration();
			void EndShadowMapGeneration(unsigned int width, unsigned int height);

			void Active(int index);
			void Bind();
		};

	}

}