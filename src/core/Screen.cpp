#include <core\Screen.hpp>
#include <fstream>

NAMESPACE_BEGIN

Shader::Shader(const char * VertexCode, const char * FragmentCode)
{
	GLuint VS, FS;

	// vertex shader
	VS = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VS, 1, &VertexCode, NULL);
	glCompileShader(VS);
	CheckCompileErrors(VS, "Vertex");

	// fragment Shader
	FS = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FS, 1, &FragmentCode, NULL);
	glCompileShader(FS);
	CheckCompileErrors(FS, "Fragment");

	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, VS);
	glAttachShader(ID, FS);

	glLinkProgram(ID);
	CheckCompileErrors(ID, "Program");

	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(VS);
	glDeleteShader(FS);
}

void Shader::Use()
{
	glUseProgram(ID);
}

void Shader::SetBool(const std::string & Name, bool Value) const
{
	glUniform1i(glGetUniformLocation(ID, Name.c_str()), int(Value));
}

void Shader::SetInt(const std::string & Name, int Value) const
{
	glUniform1i(glGetUniformLocation(ID, Name.c_str()), Value);
}

void Shader::SetFloat(const std::string & Name, float Value) const
{
	glUniform1f(glGetUniformLocation(ID, Name.c_str()), Value);
}

void Shader::CheckCompileErrors(GLuint Shader, std::string Type)
{
	GLint Success;
	GLchar InfoLog[1024];
	if (Type != "Program")
	{
		glGetShaderiv(Shader, GL_COMPILE_STATUS, &Success);
		if (!Success)
		{
			glGetShaderInfoLog(Shader, 1024, NULL, InfoLog);
			LOG(ERROR) << "Shader complie error : " << Type << "\n" << InfoLog;
		}
	}
	else
	{
		glGetProgramiv(Shader, GL_LINK_STATUS, &Success);
		if (!Success)
		{
			glGetProgramInfoLog(Shader, 1024, NULL, InfoLog);
			LOG(ERROR) << "Program link error : " << Type << "\n" << InfoLog;
		}
	}
}

Screen::Screen(const ImageBlock & Block) : m_Block(Block), m_Scale(0.5f)
{
	if (!glfwInit())
	{
		LOG(ERROR) << "Failed to initialize GLFW";
		return;
	}

	auto Size = m_Block.GetSize();
	m_Width = Size.x();
	m_Height = Size.y();
	m_BorderSize = m_Block.GetBorderSize();

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_pWindow = glfwCreateWindow(m_Width, m_Height, "Hikari", nullptr, nullptr);

	if (m_pWindow == nullptr)
	{
		glfwTerminate();
		LOG(ERROR) << "Failed to create GLFW window!";
		return;
	}

	glfwMakeContextCurrent(m_pWindow);
	glfwSwapInterval(1); // Enable vsync

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
	ImGui_ImplOpenGL3_Init();
	ImGui::StyleColorsDark();

	if (glewInit() != GLEW_OK)
	{
		LOG(ERROR) << "Failed to initialize GLEW!";
		return;
	}
	if (!glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"))
	{
		LOG(ERROR) << "Support for necessary OpenGL extensions missing.";
		return;
	}

	const GLchar * VertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 position;\n"
		"layout(location = 1) in vec2 uv;\n"
		"out vec2 out_uv;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(position, 1.0);\n"
		"	out_uv = uv;\n"
		"}";

	const GLchar * FragmentShaderSource =
		"#version 330\n"
		"uniform sampler2D source;\n"
		"uniform float scale;\n"
		"in vec2 out_uv;\n"
		"out vec4 out_color;\n"
		"float toSRGB(float value) {\n"
		"    if (value < 0.0031308)\n"
		"        return 12.92 * value;\n"
		"    return 1.055 * pow(value, 0.41666) - 0.055;\n"
		"}\n"
		"void main() {\n"
		"    vec4 color = texture(source, out_uv);\n"
		"    color *= scale / color.w;\n"
		"    out_color = vec4(toSRGB(color.r), toSRGB(color.g), toSRGB(color.b), 1);\n"
		"}";

	// Shader
	m_Shader.reset(new Shader(VertexShaderSource, FragmentShaderSource));

	unsigned int pIndices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	float pVertices[] =
	{
		 1.0f,  1.0f, 0.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,  1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,  0.0f, 1.0f,
		-1.0f,  1.0f, 0.0f,  0.0f, 0.0f 
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pVertices), pVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndices), pIndices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	LOG(INFO) << "OpenGL initialized!Version: " << glGetString(GL_VERSION);
}

void Screen::Draw()
{
	while (!glfwWindowShouldClose(m_pWindow))
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, m_Width, m_Height);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		if (ImGui::Begin("Option", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::SliderFloat("Explosure Value", &m_Scale, 0.0f, 1.0f);
			ImGui::End();
		}

		float Scale = std::pow(2.0f, (m_Scale - 0.5f) * 20.0f);
		
		ImGui::Render();

		m_Block.Lock();
		
		glActiveTexture(GL_TEXTURE0);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, GLint(m_Block.cols()));
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA32F, m_Width, m_Height, 0, GL_RGBA, GL_FLOAT, 
			(uint8_t*)(m_Block.data()) + (m_BorderSize * m_Block.cols() + m_BorderSize) * sizeof(Color4f)
		);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glBindTexture(GL_TEXTURE_2D, m_Texture);

		m_Block.Unlock();

		m_Shader->Use();
		m_Shader->SetFloat("scale", Scale);
		m_Shader->SetInt("source", 0);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0));
		glBindVertexArray(0);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(m_pWindow);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
	glDeleteBuffers(1, &m_EBO);

	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

NAMESPACE_END
