#include "hzpch.h"
#include "Hazel/Renderer/Renderer3D.h"
#include "Hazel/Renderer/RendererCommand.h"
#include "Hazel/Renderer/Model.h"

namespace Hazel
{
	struct Renderer3DData
	{
		Ref<Model> m_model;
		std::unordered_map<ShaderType, Ref<Shader>> shaderMaps;
		Ref<Shader> compositeShader;
		Ref<Shader> linkedListOITResolveShader;
		Ref<VertexArray> screenQuadVAO;
		Ref<VertexBuffer> screenQuadVBO;
		Ref<IndexBuffer> screenQuadEBO;

		Ref<Texture2D> headPointTexture;
		Ref<PixelUnpackBuffer> headPoinntClearBuffer;
		Ref<AtomicCounterBuffer> atomicCountBuffer;
		Ref<TextureBuffer> linkedListBuffer;
	};
	static Renderer3DData s_3DData;

	void Renderer3D::Init()
	{
		s_3DData.m_model = CreateRef<Model>();
		std::vector<Vertex> vertices =
		{
			Vertex(glm::vec3(2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f),
				glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 10),
			Vertex(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f),
				glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), 11),
			Vertex(glm::vec3(-2.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f),
				glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 12),
		};
		std::vector<unsigned int> indices = { 0, 1, 2 };
		s_3DData.m_model->SetVerticesAndIndices(vertices, indices);

		s_3DData.shaderMaps[ShaderType::BlinnPhong] = Shader::Create("assets/shaders/Blinn-phong.glsl");
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT] = Shader::Create("assets/shaders/WeightedBlend_Transparent.glsl");
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build] = Shader::Create("assets/shaders/LinkedListOIT_Build.glsl");
		s_3DData.compositeShader = Shader::Create("assets/shaders/WeightedBlend_Composite.glsl");
		s_3DData.linkedListOITResolveShader = Shader::Create("assets/shaders/LinkedListOIT_Resolve.glsl");

		// s_3DData Quad
		s_3DData.screenQuadVAO = VertexArray::Create();
		s_3DData.screenQuadVAO->Bind();
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f
		};
		s_3DData.screenQuadVBO = VertexBuffer::Create(sizeof(quadVertices));
		s_3DData.screenQuadVBO->SetData((void*)quadVertices, sizeof(quadVertices));
		s_3DData.screenQuadVBO->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
			});
		s_3DData.screenQuadVAO->AddVertexBuffer(s_3DData.screenQuadVBO);
		uint32_t quadIndices[] = { 0, 1, 2, 2, 3, 0 };
		s_3DData.screenQuadEBO = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
		s_3DData.screenQuadVAO->SetIndexBuffer(s_3DData.screenQuadEBO);

		int maxWidth = RendererCommand::GetMaxMonitorSize().first;
		int maxHeight = RendererCommand::GetMaxMonitorSize().second;
		s_3DData.headPointTexture = Texture2D::Create(maxWidth, maxHeight, ImageFormat::R32UI);
		s_3DData.headPointTexture->BindImageTexture(0);

		s_3DData.headPoinntClearBuffer = PixelUnpackBuffer::Create(maxWidth * maxHeight * sizeof(uint32_t));
		void* data = s_3DData.headPoinntClearBuffer->MapBuffer();
		memset(data, 0, maxWidth * maxHeight * sizeof(uint32_t));
		s_3DData.headPoinntClearBuffer->UnmapBuffer();

		s_3DData.atomicCountBuffer = AtomicCounterBuffer::Create();
		uint32_t* initData = (uint32_t*)s_3DData.atomicCountBuffer->MapBuffer();
		initData[0] = 0;
		s_3DData.atomicCountBuffer->UnmapBuffer();

		s_3DData.linkedListBuffer = TextureBuffer::Create(maxWidth * maxHeight * 3 * 16, ImageFormat::RGBA32UI);
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		glm::mat4 viewProjectionMatrix = camera.GetViewProjection();
		glm::vec3 cameraPos = camera.GetPosition();

		s_3DData.shaderMaps[ShaderType::BlinnPhong]->Bind();
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetMat4("u_ViewProjection", viewProjectionMatrix);
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("viewPos", cameraPos);
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.direction",
			glm::inverse(camera.GetViewMatrix()) * glm::vec4(0.1f, 0.2f, -1.0f, 0.0f));
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.ambient", glm::vec3(0.1f));
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.diffuse", glm::vec3(1.0f));
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.specular", glm::vec3(1.0f));

		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->Bind();
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->SetMat4("u_ViewProjection", viewProjectionMatrix);
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->SetFloat3("viewPos", cameraPos);
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->SetFloat3("dirLight.direction", 
			glm::inverse(camera.GetViewMatrix()) * glm::vec4(0.1f, 0.2f, -1.0f, 0.0f));
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->SetFloat3("dirLight.ambient", glm::vec3(0.1f));
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->SetFloat3("dirLight.diffuse", glm::vec3(1.0f));
		s_3DData.shaderMaps[ShaderType::WeightedBlendOIT]->SetFloat3("dirLight.specular", glm::vec3(1.0f));

		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->Bind();
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->SetMat4("u_ViewProjection", viewProjectionMatrix);
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->SetFloat3("viewPos", cameraPos);
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->SetFloat3("dirLight.direction",
			glm::inverse(camera.GetViewMatrix()) * glm::vec4(0.1f, 0.2f, -1.0f, 0.0f));
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->SetFloat3("dirLight.ambient", glm::vec3(0.1f));
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->SetFloat3("dirLight.diffuse", glm::vec3(1.0f));
		s_3DData.shaderMaps[ShaderType::LinkedListOIT_Build]->SetFloat3("dirLight.specular", glm::vec3(1.0f));

		s_3DData.atomicCountBuffer->Reset(1);
		s_3DData.atomicCountBuffer->BindBase(0);
		s_3DData.headPointTexture->BindImageTexture(0);
		s_3DData.linkedListBuffer->BindImageTexture(1);

		s_3DData.headPoinntClearBuffer->Bind();
		s_3DData.headPointTexture->SetData(nullptr, RendererCommand::GetMaxMonitorSize().first * RendererCommand::GetMaxMonitorSize().second * sizeof(uint32_t));
		s_3DData.headPoinntClearBuffer->Unbind();
	}

	void Renderer3D::EndScene()
	{
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->Bind();
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetMat4("u_Transform", glm::mat4(1.0f));
		s_3DData.shaderMaps[ShaderType::BlinnPhong]->SetInt("u_entityID", 11);
		RendererCommand::DrawIndexed(s_3DData.m_model->GetVertexArray(),
			s_3DData.m_model->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::DrawIndexed(const MaterialComponent& material, const glm::mat4& transform,
		const Ref<VertexArray>& vertexArray, uint32_t indexCount, int entityID)
	{
		switch (material.shaderType)
		{
		case ShaderType::BlinnPhong:
			s_3DData.shaderMaps[material.shaderType]->Bind();
			s_3DData.shaderMaps[material.shaderType]->SetMat4("u_Transform", transform);
			s_3DData.shaderMaps[material.shaderType]->SetInt("u_entityID", entityID);

			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.ambient", material.ambient);
			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.diffuse", material.diffuse);
			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.specular", material.specular);
			s_3DData.shaderMaps[material.shaderType]->SetFloat("material.shininess", material.shininess);
			break;
		case ShaderType::WeightedBlendOIT:
			s_3DData.shaderMaps[material.shaderType]->Bind();
			s_3DData.shaderMaps[material.shaderType]->SetMat4("u_Transform", transform);
			s_3DData.shaderMaps[material.shaderType]->SetInt("u_entityID", entityID);

			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.ambient", material.ambient);
			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.diffuse", material.diffuse);
			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.specular", material.specular);
			s_3DData.shaderMaps[material.shaderType]->SetFloat("material.shininess", material.shininess);
			s_3DData.shaderMaps[material.shaderType]->SetFloat("material.alpha", material.alpha);
			break;
		case ShaderType::LinkedListOIT_Build:
			s_3DData.shaderMaps[material.shaderType]->Bind();
			s_3DData.shaderMaps[material.shaderType]->SetMat4("u_Transform", transform);
			s_3DData.shaderMaps[material.shaderType]->SetInt("u_entityID", entityID);

			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.ambient", material.ambient);
			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.diffuse", material.diffuse);
			s_3DData.shaderMaps[material.shaderType]->SetFloat3("material.specular", material.specular);
			s_3DData.shaderMaps[material.shaderType]->SetFloat("material.shininess", material.shininess);
			s_3DData.shaderMaps[material.shaderType]->SetFloat("material.alpha", material.alpha);

			//RendererCommand::MemeryBarrierTexFetch();
			RendererCommand::SetDepthMask(false);
			RendererCommand::SetColorMaski(0, false, false, false, false);
			RendererCommand::DrawIndexed(vertexArray, indexCount);
			RendererCommand::SetDepthMask(true);
			RendererCommand::SetColorMaski(0, true, true, true, true);
			return;
		}

		
		RendererCommand::DrawIndexed(vertexArray, indexCount);
	}

	void Renderer3D::CompositePass(const Ref<FrameBuffer>& framebuffer)
	{

		// Ensure texture writes are visible
		//RendererCommand::MemeryBarrierTexFetch();
		//RendererCommand::Flush();

		RendererCommand::SetDepthTest(false);
		RendererCommand::SetBlendFunc(RendererAPI::BlendFactor::SrcAlpha, RendererAPI::BlendFactor::OneMinusSrcAlpha);

		s_3DData.compositeShader->Bind();
		RendererCommand::BindTexture(framebuffer->GetColorAttachmentRendererID(2), 0);
		RendererCommand::BindTexture(framebuffer->GetColorAttachmentRendererID(3), 1);
		s_3DData.compositeShader->SetInt("u_Accum", 0);
		s_3DData.compositeShader->SetInt("u_Reveal", 1);

		//// Prevent Feedback Loop: Only write to Attachment 0
		//GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
		//glDrawBuffers(1, buffers);

		//RendererCommand::DrawIndexed(s_3DData.screenQuadVAO);
		RendererCommand::DrawIndexed(s_3DData.screenQuadVAO, s_3DData.screenQuadEBO->GetCount());

		//// Restore DrawBuffers for next pass
		//GLenum buffers4[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		//glDrawBuffers(4, buffers4);

		RendererCommand::SetDepthTest(true);
	}

	void Renderer3D::ResolveOIT(const Ref<FrameBuffer>& framebuffer)
	{
		//RendererCommand::MemeryBarrierTexFetch();
		//RendererCommand::Flush();

		RendererCommand::SetDepthTest(false);
		RendererCommand::SetBlendFunc(RendererAPI::BlendFactor::SrcAlpha, RendererAPI::BlendFactor::OneMinusSrcAlpha);

		s_3DData.linkedListOITResolveShader->Bind();
		s_3DData.headPointTexture->BindImageTexture(0);
		s_3DData.linkedListBuffer->BindImageTexture(1);

		RendererCommand::BindTexture(framebuffer->GetColorAttachmentRendererID(0), 0);
		s_3DData.linkedListOITResolveShader->SetInt("backgroundTexture", 0);

		RendererCommand::DrawIndexed(s_3DData.screenQuadVAO, s_3DData.screenQuadEBO->GetCount());

		RendererCommand::SetDepthTest(true);
	}


}
