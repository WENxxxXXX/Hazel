#pragma once

#include "Hazel/Renderer/OrthoGraphicCamera.h"
#include "Hazel/Renderer/Texture.h"
#include "Hazel/Renderer/SubTexture2D.h"

namespace Hazel {

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const OrthoGraphicCamera& camera);
		static void EndScene();

		static void Flush();
		static void FlushAndReset();

		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, 
			const Ref<Texture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, 
			const Ref<Texture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, 
			const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, 
			const Ref<SubTexture2D>& subtexture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, 
			float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, 
			float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, 
			float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, 
			float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, 
			float rotation, const Ref<SubTexture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, 
			float rotation, const Ref<SubTexture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f));


		// Statistics (����ʱʹ�õ�ͳ�����ݣ������Statistics�ṹ��)
		struct Statistics {
			uint32_t DrawCalls;
			uint32_t QuadCount;

			// ����������ʱ�ټ���Vertex��Index����ʡ����
			uint32_t GetVertexCount() { return QuadCount * 4; }
			uint32_t GetIndexCount() { return QuadCount * 6; }
		};
		static void ClearStats();
		// So that we can grab data (in s_Data.Stats) where we want to use 
		static Statistics GetStats();
	};


}