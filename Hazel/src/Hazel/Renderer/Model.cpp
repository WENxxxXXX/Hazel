#include "hzpch.h"
#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

Hazel::Model::Model()
{
}

Hazel::Model::Model(const std::string meshPath)
{
	// 使用assimp库加载模型
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(meshPath, 
		aiProcess_Triangulate |           // 将所有面转换为三角形
		aiProcess_FlipUVs |               // 翻转UV坐标
		aiProcess_CalcTangentSpace |      // 计算切线和副法线
		aiProcess_GenSmoothNormals |      // 生成平滑法线
		aiProcess_JoinIdenticalVertices); // 合并相同顶点

	// 检查场景是否加载成功
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		HZ_CORE_ERROR("Assimp: {}", importer.GetErrorString());
		return;
	}

	// 处理模型的所有网格
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		ProcessMesh(mesh);
	}
}

void Hazel::Model::ProcessMesh(aiMesh* mesh)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	// 处理所有顶点
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		// 位置
		vertex.position = glm::vec3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		);

		// 法线
		if (mesh->HasNormals())
		{
			vertex.norm = glm::vec3(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			);
		}
		else
		{
			vertex.norm = glm::vec3(0.0f);
		}

		// 纹理坐标
		if (mesh->mTextureCoords[0])
		{
			vertex.texcoord = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}
		else
		{
			vertex.texcoord = glm::vec2(0.0f);
		}

		// 颜色（如果存在）
		if (mesh->mColors[0])
		{
			vertex.color = glm::vec4(
				mesh->mColors[0][i].r,
				mesh->mColors[0][i].g,
				mesh->mColors[0][i].b,
				mesh->mColors[0][i].a
			);
		}
		else
		{
			vertex.color = glm::vec4(1.0f);
		}

		vertices.push_back(vertex);
	}

	// 处理所有索引
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// 设置顶点和索引
	SetVerticesAndIndices(vertices, indices);

	HZ_CORE_INFO("Model loaded successfully: vertices={}, indices={}", vertices.size(), indices.size());
}

Hazel::Model::~Model()
{
}

void Hazel::Model::SetVerticesAndIndices(const std::vector<Vertex>& vertices, 
	const std::vector<unsigned int>& indices)
{
	m_vertices = vertices;
	m_indices = indices;

	m_VAO = VertexArray::Create();
	m_VAO->Bind();

	m_VBO = VertexBuffer::Create(vertices.size() * sizeof(Vertex));
	m_VBO->SetData((void*)m_vertices.data(), vertices.size() * sizeof(Vertex));
	BufferLayout squareLayout =
	{
		{ShaderDataType::Float3, "a_Position"},
		{ShaderDataType::Float3, "a_Norm"},
		{ShaderDataType::Float2, "a_TexCoord"},
		{ShaderDataType::Float4, "a_Color"}
	};
	m_VBO->SetLayout(squareLayout);

	m_EBO = IndexBuffer::Create(m_indices.data(), m_indices.size());

	m_VAO->SetIndexBuffer(m_EBO);
	m_VAO->AddVertexBuffer(m_VBO);
}
