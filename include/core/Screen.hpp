#pragma once

#include <core\Common.hpp>
#include <core\Parser.hpp>
#include <core\Scene.hpp>
#include <core\Camera.hpp>
#include <core\Block.hpp>
#include <core\Timer.hpp>
#include <core\Bitmap.hpp>
#include <core\Sampler.hpp>
#include <core\Integrator.hpp>
#include <tbb\parallel_for.h>
#include <tbb\blocked_range.h>
#include <filesystem\resolver.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

NAMESPACE_BEGIN

class Shader
{
public:
	Shader(const char * VertexCode, const char * FragmentCode);

	void Use();

	void SetBool(const std::string & Name, bool Value) const;

	void SetInt(const std::string & Name, int Value) const;

	void SetFloat(const std::string & Name, float Value) const;

private:
	void CheckCompileErrors(GLuint Shader, std::string Type);

private:
	GLuint ID;
};

class Screen
{
public:
	Screen(const ImageBlock & Block);
	std::vector<const ImageBlock *> & GetRenderingBlocks();
	float & GetProgress();
	std::string & GetRenderTimeString();
	void Draw();
	void DrawUI();

private:
	void BindScreenVertexBuffer();
	void BindBlockVertexBuffer();

private:
	const ImageBlock & m_Block;
	GLFWwindow * m_pWindow = nullptr;
	GLuint m_Texture = GLuint(-1);
	GLuint m_VBO = GLuint(-1);
	GLuint m_VAO = GLuint(-1);
	GLuint m_EBO = GLuint(-1);
	std::unique_ptr<Shader> m_ScreenShader = nullptr;
	std::unique_ptr<Shader> m_BlockShader = nullptr;
	int m_Width;
	int m_Height;
	int m_BorderSize;
	float m_Scale;
	std::vector<const ImageBlock *> m_RenderingBlocks;
	float m_Progress = 0.0f;
	std::string m_RenderTimeString = "";
};

NAMESPACE_END