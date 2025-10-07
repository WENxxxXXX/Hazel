#pragma once
#include "RendererCommand.h"

#include "OrthoGraphicCamera.h"
#include "Shader.h"

namespace Hazel
{
	class Renderer
	{
	public:
		static void BeginScene(OrthoGraphicCamera& camera);
		static void EndScene();

		static void Submit(const std::shared_ptr<Shader>& shader, 
			const std::shared_ptr<VertexArray>& vertexArray,
			const glm::mat4& transform);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData {
			glm::mat4 ViewProjectionMatrix;
		};
		static SceneData* s_SceneData;
	};
}