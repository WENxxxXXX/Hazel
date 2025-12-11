#pragma once
#include "Hazel/Renderer/EditorCamera.h"
#include "Hazel/Renderer/Shader.h"
#include "Hazel/Renderer/VertexArray.h"
#include "Hazel/Renderer/FrameBuffer.h"
#include <Hazel/Scene/Component.h>

namespace Hazel
{
	class Renderer3D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& viewMatrix);
		static void BeginScene(const EditorCamera& camera);
		static void EndScene();

		static void DrawIndexed(const MaterialComponent& material, const glm::mat4& transform,
			const Ref<VertexArray>& vertexArray, uint32_t indexCount, int entityID);

		static void CompositePass(const Ref<FrameBuffer>& framebuffer);
		static void ResolveOIT(const Ref<FrameBuffer>& framebuffer);
	};
}