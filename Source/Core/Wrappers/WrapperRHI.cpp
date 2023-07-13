#include "pch.h"

#include <LowRenderer/LightManager.h>
#include <Resources/Shader.h>
#include <Render/Camera.h>
#include <Utils/Loader.h>
#include <Resources/Texture.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <glad/glad.h>
#include <LowRenderer/LightManager.h>
#include <Components/LightComponent.h>
#include <Core/GameObject.h>

using namespace Core::Wrapper;
using namespace Core::Wrapper::WrapperRHI;

// =============================================[Shader]============================================== \\

// Use Shader
void Core::Wrapper::WrapperRHI::ShaderUse(unsigned int& ID)
{
	glUseProgram(ID);
}

// Delete Shader
void Core::Wrapper::WrapperRHI::ShaderDelete(unsigned int& ID)
{
	glDeleteProgram(ID);
}

/* Sending uniforms */
#pragma region SendUniform

void Core::Wrapper::WrapperRHI::ShaderSendSampler(const int shaderData, const char* name, const int value)
{
	glUniform1i(glGetUniformLocation(shaderData, name), value);
}

void Core::Wrapper::WrapperRHI::ShaderSendSampler(int location, const int value)
{
	if (location == -1)
		return;
	glUniform1i(location, value);
}

void Core::Wrapper::WrapperRHI::ShaderSendInt(const int shaderData, const char* name, const int value)
{
	glUniform1i(glGetUniformLocation(shaderData, name), value);
}

void Core::Wrapper::WrapperRHI::ShaderSendInt(int location, const int value)
{
	if (location == -1)
		return;
	glUniform1i(location, value);
}

void Core::Wrapper::WrapperRHI::ShaderSendFloat(const int shaderData, const char* name, const float value)
{
	glUniform1fv(glGetUniformLocation(shaderData, name), 1, &value);
}

void Core::Wrapper::WrapperRHI::ShaderSendFloat(int location, const float value)
{
	if (location == -1)
		return;
	glUniform1f(location, value);
}

void Core::Wrapper::WrapperRHI::ShaderSendVec2(const int shaderProgram, const char* name, const Math::Vector2& value)
{
	glUniform2fv(glGetUniformLocation(shaderProgram, name), 1, &value.x);
}

void Core::Wrapper::WrapperRHI::ShaderSendVec2(int location, const Math::Vector2& value)
{
	if (location == -1)
		return;
	glUniform2fv(location, 1, &value.x);
}

void Core::Wrapper::WrapperRHI::ShaderSendVec3(const int shaderData, const char* name, const Math::Vector3& value)
{
	glUniform3fv(glGetUniformLocation(shaderData, name), 1, &value.x);
}

void Core::Wrapper::WrapperRHI::ShaderSendVec3(int location, const Math::Vector3& value)
{
	if (location == -1)
		return;
	glUniform3fv(location, 1, &value.x);
}

void Core::Wrapper::WrapperRHI::ShaderSendVec4(const int shaderProgram, const char* name, const Math::Vector4& value)
{
	glUniform4fv(glGetUniformLocation(shaderProgram, name), 1, &value.x);
}

void Core::Wrapper::WrapperRHI::ShaderSendVec4(int location, const Math::Vector4& value)
{
	if (location == -1)
		return;
	glUniform4fv(location, 1, &value.x);
}

void Core::Wrapper::WrapperRHI::ShaderSendBool(const int shaderProgram, const char* name, const bool& value)
{
	glUniform1i(glGetUniformLocation(shaderProgram, name), value);
}

void Core::Wrapper::WrapperRHI::ShaderSendBool(int location, const bool& value)
{
	if (location == -1)
		return;
	glUniform1i(location, value);
}

void Core::Wrapper::WrapperRHI::ShaderSendMat4(const int shaderData, const char* name, const Math::Matrix4& value, bool transpose /*= true*/)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderData, name), 1, transpose, &value[0][0]);
}

void Core::Wrapper::WrapperRHI::ShaderSendMat4(int location, const Math::Matrix4& value, bool transpose /*= true*/)
{
	if (location == -1)
		return;
	glUniformMatrix4fv(location, 1, transpose, &value[0][0]);
}

void Core::Wrapper::WrapperRHI::ShaderSendMat4(int location, const std::vector<Math::Matrix4>& value, int count, bool transpose /*= true*/)
{
	if (location == -1)
		return;
	glUniformMatrix4fv(location, count, transpose, &value.data()[0][0][0]);
}

#pragma endregion

/*   Get uniforms   */
#pragma region GetUniform

void Core::Wrapper::WrapperRHI::ShaderGetSampler(const int shaderID, const int& index, int* value)
{
	glGetUniformiv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetInt(const int shaderID, const int& index, int* value)
{
	glGetUniformiv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetFloat(const int shaderID, const int& index, float* value)
{
	glGetUniformfv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetVec2(const int shaderID, const int& index, float* value)
{
	glGetUniformfv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetVec3(const int shaderID, const int& index, float* value)
{
	glGetUniformfv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetVec4(const int shaderID, const int& index, float* value)
{
	glGetUniformfv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetMat4(const int shaderID, const int& index, float* value)
{
	glGetUniformfv(shaderID, index, value);
}

void Core::Wrapper::WrapperRHI::ShaderGetBool(const int shaderID, const int& index, bool* value)
{
	int iBool = 0;
	glGetUniformiv(shaderID, index, &iBool);
	*value = iBool ? true : false;
}

#pragma endregion

// =================================================================================================== \\

// =============================================[Texture]============================================= \\



//Texture Uniform
void Core::Wrapper::WrapperRHI::TextureData(Resources::Shader* shaderData, const char* name, unsigned int unit)
{
	GLuint texUni = glGetUniformLocation(shaderData->ID, name);
	shaderData->Use();
	glUniform1i(texUni, unit);
}

//Bind Texture
void Core::Wrapper::WrapperRHI::TextureBind(unsigned int ID, unsigned int type /*= PR_TEXTURE2D*/)
{
	glBindTexture(type, ID);
}

//Unbind Texture
void Core::Wrapper::WrapperRHI::TextureUnBind(unsigned int type)
{
	glBindTexture(type, 0);
}

//Texture Delete
void Core::Wrapper::WrapperRHI::TextureDelete(unsigned int& ID)
{
	glDeleteTextures(1, &ID);
}

void Core::Wrapper::WrapperRHI::SendTexture(unsigned int& ID, unsigned int format, unsigned int texType, unsigned int slot, unsigned int pixelType, const int& widthImg, const int& heightImg, unsigned char* bytes, int filter /*= PR_NEAREST*/)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glGenTextures(1, &ID);
	glBindTexture(texType, ID);

	glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, filter);

	glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(texType, 0, format, widthImg, heightImg, 0, format, pixelType, bytes);
	glGenerateMipmap(texType);

	glBindTexture(texType, 0);
}

void Core::Wrapper::WrapperRHI::GenVertex(unsigned int& VAO, unsigned int& VBO, float vertices[], std::size_t size)
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Core::Wrapper::WrapperRHI::BindVAO(unsigned int& VAO)
{
	glBindVertexArray(VAO);
}

void Core::Wrapper::WrapperRHI::SetDephtFunc(bool value)
{
	if (value)
		glDepthFunc(GL_LEQUAL);
	else
		glDepthFunc(GL_LESS);
}

void copyPixels(unsigned char* src, unsigned char* dst, int faceWidth, int faceHeight, int texWidth, int nrChannels, int offsetX, int offsetY) {
	for (int j = 0; j < faceHeight; j++) {
		for (int i = 0; i < faceWidth; i++) {
			for (int k = 0; k < nrChannels; k++) {
				dst[(j * faceWidth + i) * nrChannels + k] = src[(j + offsetY) * texWidth * nrChannels + (i + offsetX) * nrChannels + k];
			}
		}
	}
}

void Core::Wrapper::WrapperRHI::SendCubeMap(unsigned int& ID, int texWidth, int texHeight, int nrChannels, unsigned char* texData)
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	int faceWidth = texWidth / 4;
	int faceHeight = texHeight / 3;

	unsigned char* faces[6];

	// right face
	faces[0] = new unsigned char[faceWidth * faceHeight * nrChannels];
	copyPixels(texData, faces[0], faceWidth, faceHeight, texWidth, nrChannels, 2 * faceWidth, faceHeight);

	// left face
	faces[1] = new unsigned char[faceWidth * faceHeight * nrChannels];
	copyPixels(texData, faces[1], faceWidth, faceHeight, texWidth, nrChannels, 0, faceHeight);

	// top face
	faces[2] = new unsigned char[faceWidth * faceHeight * nrChannels];
	copyPixels(texData, faces[2], faceWidth, faceHeight, texWidth, nrChannels, faceWidth, 0);

	// bottom face
	faces[3] = new unsigned char[faceWidth * faceHeight * nrChannels];
	copyPixels(texData, faces[3], faceWidth, faceHeight, texWidth, nrChannels, faceWidth, 2 * faceHeight);

	// front face
	faces[4] = new unsigned char[faceWidth * faceHeight * nrChannels];
	copyPixels(texData, faces[4], faceWidth, faceHeight, texWidth, nrChannels, faceWidth, faceHeight);

	// back face
	faces[5] = new unsigned char[faceWidth * faceHeight * nrChannels];
	copyPixels(texData, faces[5], faceWidth, faceHeight, texWidth, nrChannels, 3 * faceWidth, faceHeight);


	unsigned int colorType = (nrChannels > 3) ? GL_RGBA : GL_RGB;
	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, colorType, faceWidth, faceHeight, 0, colorType, GL_UNSIGNED_BYTE, faces[i]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Core::Wrapper::WrapperRHI::SendSixSided(unsigned int& ID, int texWidths[6], int texHeights[6], unsigned char* texData[6])
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, ID);

	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, texWidths[i], texHeights[i], 0, GL_RGB, GL_UNSIGNED_BYTE, texData[i]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

bool Core::Wrapper::WrapperRHI::SendShader(std::string& vertShader, std::string& fragShader, unsigned int& ID, Resources::Shader* shader)
{
	const char* vFile = vertShader.c_str();
	const char* fFile = fragShader.c_str();

	GLuint m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_vertexShader, 1, &vFile, NULL);
	glCompileShader(m_vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(m_vertexShader, 512, NULL, infoLog);
		PrintError("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s", infoLog);
		return false;
	}
	else
		PrintLog("Successfully Compiling Vertex Shader %s", shader->GetPath().c_str());


	GLuint m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_fragmentShader, 1, &fFile, NULL);
	glCompileShader(m_fragmentShader);

	glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(m_fragmentShader, 512, NULL, infoLog);
		PrintError("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED %s", infoLog);
		return false;
	}
	else
		PrintLog("Successfully Compiling Fragment Shader %s", shader->GetPath().c_str());

	ID = glCreateProgram();
	glAttachShader(ID, m_vertexShader);
	glAttachShader(ID, m_fragmentShader);
	glLinkProgram(ID);

	success;
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		PrintError("ERROR::SHADER::PROGRAM::LINKING_FAILED %s", infoLog);
		return false;
	}
	else {
		PrintLog("Successfully link Shader %s", shader->GetPath().c_str());
	}
	glDeleteShader(m_vertexShader);
	glDeleteShader(m_fragmentShader);
	return true;
}

void Core::Wrapper::WrapperRHI::GetAllUniform(Resources::Shader* shaderData)
{
	Vector3 v = { 1,1,1 };
	ShaderSendVec3(shaderData->ID, "testV", v);
	// Get total number of uniforms
	GLint n = 0;
	glGetProgramiv(shaderData->ID, GL_ACTIVE_UNIFORMS, &n);

	// Get max length of uniforms
	GLint length_max = 0;
	glGetProgramiv(shaderData->ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &length_max);

	char* name = new char[length_max];
	std::vector<Resources::ShaderVariables>& data = shaderData->GetShaderVariables();

	// Get data of all uniforms
	for (int index = 0; index < n; index++)
	{
		GLint glsl_size;
		GLenum glsl_type;
		glGetActiveUniform(shaderData->ID, index, length_max, NULL, &glsl_size, &glsl_type, name);

		GLint location = glGetUniformLocation(shaderData->ID, name);

		data.push_back(
			{
			index,
			name ,
			location,
			glsl_type,
			glsl_size
			}
		);
	}
	delete[] name;
	name = nullptr;

	shaderData->SetUniformSet();
}

// =================================================================================================== \\


// ===============================================[Buffer]============================================= \\

Core::Wrapper::WrapperRHI::Buffer::Buffer(float* vertices, size_t numVertices, unsigned int* indices, size_t numIndices)
{
	glGenVertexArrays(1, &VertexArray);
	glBindVertexArray(VertexArray);

	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &IndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(unsigned int), indices, GL_STATIC_DRAW);
}

Core::Wrapper::WrapperRHI::Buffer::Buffer(float* vertices, size_t numVertices)
{
	glGenVertexArrays(1, &VertexArray);
	glBindVertexArray(VertexArray);

	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(float), vertices, GL_STATIC_DRAW);
}

void Core::Wrapper::WrapperRHI::Buffer::BufferSubData(unsigned int offset, size_t size, const void* data)
{
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

void Core::Wrapper::WrapperRHI::Buffer::BufferData(size_t size, const void* data)
{
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
}

void Buffer::GenVertexBuffer()
{
	glGenBuffers(1, &VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
}

void Buffer::AttribDivisor(unsigned int index, unsigned int divisor)
{
	glVertexAttribDivisor(index, divisor);
}

void Buffer::InitializePlane()
{
	glGenVertexArrays(1, &VertexArray);
	glGenBuffers(1, &VertexBuffer);
	glBindVertexArray(VertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Buffer::UnbindVertexBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Buffer::BindVertexBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
}

void Buffer::LinkAttribute(unsigned int layout, unsigned int numComponents, unsigned int type, unsigned int stride, void* offset)
{
	glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
}

void Buffer::Bind()
{
	glBindVertexArray(VertexArray);
}

void Buffer::Unbind()
{
	glBindVertexArray(0);
}

void Buffer::Delete()
{
	glDeleteBuffers(1, &VertexBuffer);
	glDeleteBuffers(1, &IndexBuffer);
	glDeleteVertexArrays(1, &VertexArray);
}

// =================================================================================================== \\


// =============================================[Class RHI]============================================ \\

//Initializes the GLAD library to load the OpenGL functions.
void Core::Wrapper::WrapperRHI::InitializeAPI()
{
	if (!gladLoadGLLoader((GLADloadproc)WrapperWindow::GetProcAddress)) {
		PrintError("Failed to initialize GLAD");
		return;
	}
#ifndef PANDOR_GAME
	glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	glLineWidth(5.f);
#endif
}

//Enables debugging output for OpenGL error messages.
void Core::Wrapper::WrapperRHI::EnableDebugOutput()
{
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(DebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCullFace(GL_BACK);
}

//Creates a vertex array object (VertexArray) by linking vertex attributes to vertex data.
void Core::Wrapper::WrapperRHI::CreateBuffer(Buffer*& buffer, float* vertices, size_t verticesSize, unsigned int* indices, size_t indicesSize)
{
	buffer = new Buffer(vertices, verticesSize, indices, indicesSize);
	buffer->Bind();

	buffer->LinkAttribute(0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
	buffer->LinkAttribute(1, 2, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	buffer->LinkAttribute(2, 3, GL_FLOAT, 8 * sizeof(float), (void*)(5 * sizeof(float)));

	buffer->Unbind();
}

void Core::Wrapper::WrapperRHI::DepthActive()
{
	glEnable(GL_DEPTH_TEST);
}

void Core::Wrapper::WrapperRHI::DepthDisable()
{
	glDisable(GL_DEPTH_TEST);
}

void Core::Wrapper::WrapperRHI::StencilActive()
{
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void Core::Wrapper::WrapperRHI::UseStencil()
{
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilMask(0xFF);
}

void Core::Wrapper::WrapperRHI::DrawStencil(Resources::Shader* shaderData, Vector4 Color)
{
	if (shaderData->HasBeenSent())
	{
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);
		ShaderSendVec3(shaderData->GetLocation("OutlineColor"), Color);
	}
}

void Core::Wrapper::WrapperRHI::MaskStencil()
{
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0, 0xFF);
}

void Core::Wrapper::WrapperRHI::DisableStencil()
{
	glDisable(GL_STENCIL_TEST);
}

void Core::Wrapper::WrapperRHI::DepthRange(Vector2 r)
{
	glDepthRange(r.x, r.y);
}

void Core::Wrapper::WrapperRHI::CameraData(Resources::Shader*& shaderData, const char* name, Math::Matrix4 MVP)
{
	if (shaderData->HasBeenSent())
		glUniformMatrix4fv(glGetUniformLocation(shaderData->ID, name), 1, GL_FALSE, &MVP[0][0]);
}
// =================================================================================================== \\


// ===============================================[Debug]============================================= \\

//Callback function called when a debug message is generated by OpenGL
void Core::Wrapper::WrapperRHI::DebugOutput(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, int length, const char* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131154) return;

	std::string log = "";
	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             log += "Source: API\n"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   log += "Source: Window System\n"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: log += "Source: Shader Compiler\n"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     log += "Source: Third Party\n"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     log += "Source: Application\n"; break;
	case GL_DEBUG_SOURCE_OTHER:           log += "Source: Other\n"; break;
	}
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               log += "Type: Error\n"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: log += "Type: Deprecated Behaviour\n"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  log += "Type: Undefined Behaviour\n"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         log += "Type: Portability\n"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         log += "Type: Performance\n"; break;
	case GL_DEBUG_TYPE_MARKER:              log += "Type: Marker\n"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          log += "Type: Push Group\n"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           log += "Type: Pop Group\n"; break;
	case GL_DEBUG_TYPE_OTHER:               log += "Type: Other\n"; break;
	}

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         LOG(Debug::LogType::L_ERROR, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
	case GL_DEBUG_SEVERITY_MEDIUM:       LOG(Debug::LogType::L_WARNING, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
	case GL_DEBUG_SEVERITY_LOW:          LOG(Debug::LogType::L_WARNING, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: LOG(Debug::LogType::L_INFO, "Debug message (%d): %s\n%s", id, message, log.c_str()); break;
	}
}

void Core::Wrapper::WrapperRHI::ViewPort(int x, int y, int sizeX, int sizeY)
{
	glViewport(x, y, sizeX, sizeY);
}

const char* Core::Wrapper::WrapperRHI::GetVersion()
{
	return (const char*)glGetString(GL_VERSION);
}

const char* Core::Wrapper::WrapperRHI::GetVendor()
{
	return (const char*)glGetString(GL_VENDOR);
}

const char* Core::Wrapper::WrapperRHI::GetRenderer()
{
	return (const char*)glGetString(GL_RENDERER);
}

void Core::Wrapper::WrapperRHI::DrawElements(int count)
{
	glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0);
}

void Core::Wrapper::WrapperRHI::DrawArrays(size_t start, size_t count, bool wireframe, bool cullface)
{
	Core::App::Get().drawCall += 1;
	Core::App::Get().verticeCount += count;
	Core::App::Get().triangleCount += count / 3;
	int polygonMode;
	// Enable Wire frame.
	if (!cullface) {
		glDisable(GL_CULL_FACE);
		glGetIntegerv(GL_POLYGON_MODE, &polygonMode);
	}
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, (GLsizei)start, (GLsizei)count);

	if (!cullface) {
		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, polygonMode);
	}

	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Core::Wrapper::WrapperRHI::ClearColorAndBuffer(Vector4 clearColor)
{
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w); //0.2f, 0.3f, 0.3f, 1.0f
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Core::Wrapper::WrapperRHI::ClearColor(Vector4 clearColor)
{
	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w); //0.2f, 0.3f, 0.3f, 1.0f
	glClear(GL_COLOR_BUFFER_BIT);
}

int Core::Wrapper::WrapperRHI::ShaderGetLocation(unsigned int& ID, const char* name)
{
	return glGetUniformLocation(ID, name);
}

void Core::Wrapper::WrapperRHI::PushToGPU(unsigned char* data, int x, int y)
{
	glFlush();
	glFinish();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Core::Wrapper::WrapperRHI::PixelStore(unsigned int type, int value)
{
	glPixelStorei(type, value);
}

void Core::Wrapper::WrapperRHI::GenerateTexture(unsigned int& tex)
{
	glGenTextures(1, &tex);
}

void Core::Wrapper::WrapperRHI::FontTexture(unsigned int width, unsigned int rows, unsigned char* buffer)
{
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		width,
		rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		buffer
	);
	// set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Core::Wrapper::WrapperRHI::ActivateTexture(unsigned int index /*= 0*/)
{
	glActiveTexture(GL_TEXTURE0 + index);
}

void Core::Wrapper::WrapperRHI::UpdateTexture(unsigned int& ID, int filter /*= PR_NEAREST*/, int wrap /*= PR_REPEAT*/)
{
	glBindTexture(GL_TEXTURE_2D, ID);

	// Set the texture filter mode to nearest neighbor
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Core::Wrapper::WrapperRHI::DrawInstance(size_t first, size_t count, size_t number)
{
	glDrawArraysInstanced(GL_TRIANGLES, (GLint)first, (GLsizei)count, (GLsizei)number);
}

Core::Wrapper::WrapperRHI::RenderBuffer::RenderBuffer()
{
}

Core::Wrapper::WrapperRHI::RenderBuffer::~RenderBuffer()
{
	if (frameBuffer != 0)
		glDeleteFramebuffers(1, &frameBuffer);
	if (renderBuffer != 0)
		glDeleteRenderbuffers(1, &renderBuffer);
}

void Core::Wrapper::WrapperRHI::RenderBuffer::UnBind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Core::Wrapper::WrapperRHI::RenderBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
}

void Core::Wrapper::WrapperRHI::RenderBuffer::Generate(unsigned int& texture, const Math::Vector2& size, unsigned int texUnit /*= 0*/)
{
	glBindTextureUnit(texUnit, texture);

	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	// Set texture to resourcesManager.
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		PrintLog("Framebuffer %d Complete !", frameBuffer);
	}
	else
		PrintError("Framebuffer %d Not Complete !", frameBuffer);

	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)size.x, (GLsizei)size.y); // use a single render buffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Core::Wrapper::WrapperRHI::RenderBuffer::Resize(const Math::Vector2& windowSize)
{
	if (windowSize.x * windowSize.y != 0) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)windowSize.x, (GLsizei)windowSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)windowSize.x, (GLsizei)windowSize.y);
	}
}

void Core::Wrapper::WrapperRHI::RenderBuffer::Read(int width, int height, unsigned char*& data)
{
	Bind();
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

// =================================================================================================== \\


// ==============================================[Line]================================================ \\
 
#include <Resources/ResourcesManager.h>
#include <Core/App.h>
std::unique_ptr<Line> Core::Wrapper::WrapperRHI::Line::m_line = std::make_unique<Line>(5.f);

Core::Wrapper::WrapperRHI::Line::Line()
{

}

Core::Wrapper::WrapperRHI::Line::Line(float lineWidth)
{
	this->LineWidth = lineWidth;
}

Core::Wrapper::WrapperRHI::Line::~Line() { m_shader = nullptr; }

void Core::Wrapper::WrapperRHI::Line::Initialize()
{
	m_shader = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Shader>(ENGINEPATH"Shaders/unlit");
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) + 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &_point1);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float), 3 * sizeof(float), &_point2);


	// position attribute
	glVertexAttribPointer(0U, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0U);

	m_initialized = true;
}

void Core::Wrapper::WrapperRHI::Line::Draw(Math::Vector3 p1, Math::Vector3 p2)
{
	if (!m_initialized)
		Initialize();
	if (!m_shader || !m_shader->HasBeenSent())
	{
		return;
	}
	// Bind Position
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * sizeof(float), &p1);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float), 3 * sizeof(float), &p2);

	//glDepthRange(0, 0.01);
	float defaultWidth;
	glGetFloatv(GL_LINE_WIDTH, &defaultWidth);
	glLineWidth(LineWidth);

	m_shader->Use();
	auto MVP = Core::App::Get().sceneManager->GetCurrentScene()->GetVP();

	glUniformMatrix4fv(m_shader->GetLocation("MVP"), 1, GL_TRUE, &MVP[0][0]);
	glUniform4f(m_shader->GetLocation("ourColor"), Color.x, Color.y, Color.z, Color.w);
	glUniform1i(m_shader->GetLocation("enableTexture"), false);

	// Draw vertices
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_LINES, 0, 9);
	glBindVertexArray(0);

	glLineWidth(defaultWidth);
	//glDepthRange(0.01, 1);
}

void Core::Wrapper::WrapperRHI::Line::DrawCube(const Math::Vector3& position, const Math::Vector3& extent, const Vector4& color /*= Vector4(1, 1, 1, 1)*/)
{
	this->Color = color;
	Math::Vector3 corner[8];
	corner[0] = position - extent / 2;
	corner[1] = position + extent / 2;
	corner[2] = position + Vector3(-extent.x / 2, extent.y / 2, extent.z / 2);
	corner[3] = position + Vector3(extent.x / 2, -extent.y / 2, extent.z / 2);
	corner[4] = position + Vector3(extent.x / 2, extent.y / 2, -extent.z / 2);
	corner[5] = position + Vector3(-extent.x / 2, extent.y / 2, -extent.z / 2);
	corner[6] = position + Vector3(extent.x / 2, -extent.y / 2, -extent.z / 2);
	corner[7] = position + Vector3(-extent.x / 2, -extent.y / 2, extent.z / 2);

	// Top plane
	Draw(corner[0], corner[7]);
	Draw(corner[7], corner[3]);
	Draw(corner[3], corner[6]);
	Draw(corner[6], corner[0]);

	// Bottom plane
	Draw(corner[1], corner[4]);
	Draw(corner[4], corner[5]);
	Draw(corner[5], corner[2]);
	Draw(corner[2], corner[1]);

	//Other
	Draw(corner[0], corner[5]);
	Draw(corner[7], corner[2]);
	Draw(corner[3], corner[1]);
	Draw(corner[6], corner[4]);

}

void Core::Wrapper::WrapperRHI::Line::DrawCone(const Math::Vector3& position, const Math::Quaternion& rotation, float topRadius, float angle, float height /*= 25.f*/, const Vector4& color /*= Vector4(1, 1, 1, 1)*/)
{
	Color = color;
	auto forward = rotation * Math::Vector3::Forward();
	float h = 25.f;
	angle = angle * DEG2RAD;
	float radius = topRadius + h * std::tanf(angle);
	DrawCircle(position, forward, topRadius, 32, Color);
	DrawCircle(position + forward * h, forward, radius, 32, Color);
	Draw(position + rotation * Vector3::Up() * topRadius, position + rotation * (Vector3::Up() * radius + Vector3::Forward() * h));
	Draw(position + rotation * -Vector3::Up() * topRadius, position + rotation * (-Vector3::Up() * radius + Vector3::Forward() * h));
	Draw(position + rotation * -Vector3::Right() * topRadius, position + rotation * (-Vector3::Right() * radius + Vector3::Forward() * h));
	Draw(position + rotation * Vector3::Right() * topRadius, position + rotation * (Vector3::Right() * radius + Vector3::Forward() * h));
}

void Line::ReloadShader()
{
	m_shader = Resources::ResourcesManager::Get()->GetUnlitShader();
}

void Line::DrawCircle(const Math::Vector3& position, const Math::Vector3& direction, float radius, int segment /*= 64*/, const Vector4& color /*= Vector4(1, 1, 1, 1)*/)
{
	Color = color;
	float angle = 0.f; 
	Vector3 right = 0;
	if (direction == Vector3::Up() || direction == -Vector3::Up())
	{
		right = Vector3::Right(); // If direction is up or down, use right as the perpendicular vector
	}
	else
	{
		right = Vector3::Up().Cross(direction).GetNormalized(); // vector to the right of the direction
	}
	Vector3 startPoint = position + right * radius;
	Vector3 previousPoint = startPoint;

	for (int i = 1; i <= segment; i++)
	{
		angle = i * 360.f / segment;
		Vector3 point = position + Quaternion::AngleAxis(angle, direction) * (right * radius);
		Draw(previousPoint, point);
		previousPoint = point;
	}
	Draw(previousPoint, startPoint);

}

// =================================================================================================== \\


// =========================================[ShadowMapping]============================================ \\

void Core::Wrapper::WrapperRHI::ShadowMap::InitShadow()
{
	glGenFramebuffers(1, &m_buffer);

	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	glBindFramebuffer(GL_FRAMEBUFFER, m_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Core::Wrapper::WrapperRHI::ShadowMap::BeginShadowMapGeneration()
{
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, m_buffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	stencilDepthTest = false;
}

void Core::Wrapper::WrapperRHI::ShadowMap::EndShadowMapGeneration(unsigned int width, unsigned int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	stencilDepthTest = true;
}

void Core::Wrapper::WrapperRHI::ShadowMap::Active(int index)
{
	WrapperRHI::ActivateTexture(index);
}

void Core::Wrapper::WrapperRHI::ShadowMap::Bind()
{
	WrapperRHI::TextureBind(m_shadowMap, GL_TEXTURE_2D);
}



// =================================================================================================== \\
