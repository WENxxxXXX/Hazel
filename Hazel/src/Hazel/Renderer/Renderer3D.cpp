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
	};
	static Renderer3DData s_Data;

	void Renderer3D::Init()
	{
		s_Data.m_model = CreateRef<Model>();
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
		s_Data.m_model->SetVerticesAndIndices(vertices, indices);

		s_Data.shaderMaps[ShaderType::BlinnPhong] = Shader::Create("assets/shaders/Blinn-phong.glsl");
	}

	void Renderer3D::BeginScene(const EditorCamera& camera)
	{
		glm::mat4 viewProjectionMatrix = camera.GetViewProjection();
		glm::vec3 cameraPos = camera.GetPosition();

		s_Data.shaderMaps[ShaderType::BlinnPhong]->Bind();
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetMat4("u_ViewProjection", viewProjectionMatrix);
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("viewPos", cameraPos);

		//s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("material.ambient", glm::vec3(1.0f, 1.0f, 0.0f));
		//s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("material.diffuse", glm::vec3(1.0f, 1.0f, 0.0f));
		//s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("material.specular", glm::vec3(1.0f));
		//s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat("material.shininess", 32.0f);

		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.direction", glm::vec3(0.1f, 0.2f, -1.0f));
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.ambient", glm::vec3(0.1f));
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.diffuse", glm::vec3(1.0f));
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("dirLight.specular", glm::vec3(10.0f));
	}

	void Renderer3D::EndScene()
	{
		s_Data.shaderMaps[ShaderType::BlinnPhong]->Bind();
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetMat4("u_Transform", glm::mat4(1.0f));
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetInt("u_entityID", 11);
		RendererCommand::DrawIndexed(s_Data.m_model->GetVertexArray(), 
			s_Data.m_model->GetIndexBuffer()->GetCount());
	}

	void Renderer3D::DrawIndexed(const MaterialComponent& material, const glm::mat4& transform,
		const Ref<VertexArray>& vertexArray, uint32_t indexCount, int entityID)
	{
		//s_Data.shaderMaps[ShaderType::BlinnPhong]->Bind();
		//s_Data.shaderMaps[ShaderType::BlinnPhong]->SetMat4("u_Transform", transform);
		//s_Data.shaderMaps[ShaderType::BlinnPhong]->SetInt("u_entityID", entityID);
		s_Data.shaderMaps[material.shaderType]->Bind();
		s_Data.shaderMaps[material.shaderType]->SetMat4("u_Transform", transform);
		s_Data.shaderMaps[material.shaderType]->SetInt("u_entityID", entityID);

		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("material.ambient", material.ambient);
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("material.diffuse", material.diffuse);
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat3("material.specular", material.specular);
		s_Data.shaderMaps[ShaderType::BlinnPhong]->SetFloat("material.shininess", material.shininess);
		RendererCommand::DrawIndexed(vertexArray, indexCount);
	}


}
