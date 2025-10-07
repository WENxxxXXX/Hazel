#pragma once

#include "Hazel/Renderer/RendererAPI.h"

namespace Hazel 
{

	class OpenGLRendererAPI : public RendererAPI
	{
	public:
		void Clear() override;
		void SetClearColor(const glm::vec4& color) override;

		void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};

}