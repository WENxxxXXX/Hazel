#include <Hazel.h>
#include "Hazel/Core/EntryPoint.h"
#include "Sandbox2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "imgui/imgui.h"
#include <Hazel/Renderer/Renderer.h>

class ExampleLayer : public Hazel::Layer
{
public:
	ExampleLayer()
		: Layer("Exampler Layer"), m_CameraController(1280.0f / 720.0f, true)
	{
		float vertices[3 * 7] = {
				-0.5f, -0.5f, 0.0f, 1.0f, 0.2f, 0.8f, 1.0f,
				 0.5f, -0.5f, 0.0f, 1.0f, 0.3f, 0.8f, 1.0f,
				 0.0f,  0.5f, 0.0f, 1.0f, 0.8f, 0.2f, 1.0f
		};
		unsigned int indices[3] = { 0, 1, 2 };

		Hazel::Ref<Hazel::VertexBuffer> vertexBuffer;
		Hazel::Ref<Hazel::IndexBuffer> indexBuffer;
		vertexBuffer.reset(Hazel::VertexBuffer::Create(vertices, sizeof(vertices)));
		indexBuffer.reset(Hazel::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

		m_VertexArray = Hazel::VertexArray::Create();
		Hazel::BufferLayout layout =
		{
			{ Hazel::ShaderDataType::Float3, "a_Position" },
			{ Hazel::ShaderDataType::Float4, "a_Color" }
		};
		vertexBuffer->SetLayout(layout);// when we set the layout, we store the layout data in OpenGLBuffer.m_Layout by "SetLayout()" function, then delete layout.

		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(indexBuffer);

		std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			out vec3 v_Position;
			out vec4 v_Color;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position =  u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";
		std::string fragmentSrc = R"(
			#version 330 core

			in vec3 v_Position;
			in vec4 v_Color;
			layout(location = 0) out vec4 a_Color;

			void main()
			{
				a_Color = vec4(v_Position * 0.5 + 0.5, 1.0);
				a_Color = v_Color;
			}
		)";

		m_Shader = Hazel::Shader::Create("TriangleShader", vertexSrc, fragmentSrc);

		// -------------- Square rendering ----------------
		float squareVertices[5 * 4] =
		{
			- 0.5f, -0.5f, -0.1f, 0.0f, 0.0f,
			 0.5f, -0.5f, -0.1f, 1.0f, 0.0f,
			 0.5f,  0.5f, -0.1f, 1.0f, 1.0f,
			-0.5f,  0.5f, -0.1f, 0.0f, 1.0f
		};
		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };

		m_SquareVA = Hazel::VertexArray::Create();

		Hazel::Ref<Hazel::VertexBuffer> squareVB;
		squareVB.reset(Hazel::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		Hazel::Ref<Hazel::IndexBuffer> squareIB;
		squareIB.reset(Hazel::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));

		Hazel::BufferLayout squareLayout =
		{
			{Hazel::ShaderDataType::Float3, "a_Position"},
			{Hazel::ShaderDataType::Float2, "a_TexCoord"}
		};
		squareVB->SetLayout(squareLayout);
		m_SquareVA->AddVertexBuffer(squareVB);
		m_SquareVA->SetIndexBuffer(squareIB);

		m_SquareShader = Hazel::Shader::Create("assets/shaders/FlatColorShader.glsl");
		auto textureShader = m_ShaderLibrary.Load("assets/shaders/TextureShader.glsl");

		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->
			Bind();
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(textureShader)->
			UploadUniformInt("u_Texture", 0);
		m_EmojiTexture = Hazel::Texture2D::Create("assets/textures/emoji.png");
		m_Texture = Hazel::Texture2D::Create("assets/textures/rain.jpg");
	}

	void OnUpdate(Hazel::Timestep ts) override
	{
		// Update
		m_CameraController.OnUpdate(ts);

		// Render
		Hazel::RendererCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Hazel::RendererCommand::Clear();

		Hazel::Renderer::BeginScene(m_CameraController.GetCamera());

		glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
		//std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_SquareShader)->Bind();
		m_SquareShader->Bind();//调用虚函数
		std::dynamic_pointer_cast<Hazel::OpenGLShader>(m_SquareShader)->
			UploadUniformFloat4("u_Color", m_SquareColor);
		//m_SquareShader->UpdateUniformFloat3("u_Color", m_SquareColor);//基类没有这个函数，需要转换
		for (int y = 0; y < 20; y++) 
		{
			for (int x = 0; x < 20; x++) 
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;

				Hazel::Renderer::Submit(m_SquareShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("TextureShader");
		m_Texture->Bind();
		Hazel::Renderer::Submit(textureShader, m_SquareVA,
			glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_EmojiTexture->Bind();
		Hazel::Renderer::Submit(textureShader, m_SquareVA,
			glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		

		Hazel::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Test");
		ImGui::ColorEdit3("Square Color Edit", glm::value_ptr(m_SquareColor));
		//ImGui::ColorEdit3("Square Color Edit", &m_SquareColor[0]);//与上句等效
        const char* text = R"(
                         ...----....
                  .-'                  ''
                   .          .    .    .''.
             .' . .  .       .   .   .. ....::.
            .  .    .    .  .  .    .... ....:IA.
           '.  . .   .       .  . .. .....:.::IHHB.
         .:... . ."::"'.. .   .  . .:.. .. ..:IHIMMA
        .:::I .AHHHHHHAI::. .:...,:IIHMHHL:. . VMMMM
       :..V.:VHHHHHHHHHHB... . .:VPHHMHHHHHHHAI.:VMMI
       ::". ..HHHHMMHHHHHI.  . .:..I:MHHHHHMHV:':H:WM
       '.  ....:.HHHMMHV" .AVMHMA.:.'VHHHV:' .  :IHWV
       .:.   ...    .   ..HMMMHMMMA::.:::....  .:IHW'
     : .   .'' .: .. .  :HMMM:IMMMI::IIIPPHI::'.P:HM.
     'V:.. ... ...  .   'VMMV..VMMV :.:..:....::IHHHH
        :IP""HII:  .    . . .'V:. . . :.::IHIHHMMMMM"
        :"VI:VWMA.:      .   .. .:. ..IVHHHMMMHMMMMI
        :..VIIHMM.::.,:.,:!"I:!"I!"I!"MMMMMMHMMMMMM'
          V:HI:MA :AXXXIXII:IIIISSSSSSMMMHMHMMMMMM
             :: VSSSSSSSSSBSSSMMBSSSBB:MMHIMMI
            ::I. SSXXIIIIXSSBMBSSXXXIIMI:.IMM
            :.I:."SSSSSSSISISSXIIXSSSSI:..MMM.
              "V::...:.IHHHMMMMMMMMMMMMMHHMHP'
                 ...::.:::..:..::IIIIIHHMV"
                     "V::... . .I::IHHV"'
		)";
        ImGui::Text(text);
		ImGui::End();
	}

	void OnEvent(Hazel::Event& event) override
	{
		m_CameraController.OnEvent(event);
	}

private:
	Hazel::Ref<Hazel::Shader> m_Shader;
	Hazel::ShaderLibrary m_ShaderLibrary;
	Hazel::Ref<Hazel::VertexArray> m_VertexArray;

	Hazel::Ref<Hazel::Texture2D> m_Texture, m_EmojiTexture;

	Hazel::Ref<Hazel::Shader> m_SquareShader;
	Hazel::Ref<Hazel::VertexArray> m_SquareVA;

	glm::vec4 m_SquareColor = { 0.5412f, 0.1686f, 0.8863f, 1.0f };

	//Hazel::OrthoGraphicCamera m_Camera;
	Hazel::OrthoGraphicCameraController m_CameraController;
};

class Sandbox : public Hazel::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox()
	{

	}
};

Hazel::Application* Hazel::CreateApplication()
{
	return new Sandbox();
}