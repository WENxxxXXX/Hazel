#pragma once
#include <glm/glm.hpp>
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Core/Base.h"

struct aiMesh;

namespace Hazel
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 norm;
		glm::vec2 texcoord;
		glm::vec4 color;

		Vertex() : position(0.0f), norm(0.0f), texcoord(0.0f), color(0.0f) {}
		Vertex(glm::vec3 _position, glm::vec3 _norm, glm::vec2 _texcoord, glm::vec4 _color, int _EntityID)
			: position(_position), norm(_norm), texcoord(_texcoord), color(_color) {}
	};

	class Model
	{
	public:
		Model();
		Model(const std::string meshPath);
		~Model();

		void SetVerticesAndIndices(const std::vector<Vertex>& vertices, 
			const std::vector<unsigned int>& indices);
		Ref<VertexArray> GetVertexArray() { return m_VAO; }
		Ref<IndexBuffer> GetIndexBuffer() { return m_EBO; }

	private:
		void ProcessMesh(aiMesh* mesh);

		std::vector<Vertex> m_vertices;
		std::vector<unsigned int> m_indices;
		// VAO VBO EBO
		Ref<VertexArray> m_VAO;
		Ref<VertexBuffer> m_VBO;
		Ref<IndexBuffer> m_EBO;
	};
}