#pragma once

#include "Hazel.h"

namespace Hazel
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(Hazel::Timestep ts) override;
		void OnImGuiRender() override;
		void OnEvent(Hazel::Event& event) override;
	private:
		Hazel::Ref<Hazel::Texture2D> m_Texture, m_Emoji;
		Hazel::Ref<Hazel::Texture2D> m_SpriteSheet, m_SpriteSheetRole;
		Hazel::Ref<Hazel::SubTexture2D> m_Role;

		std::unordered_map<char, Hazel::Ref<Hazel::SubTexture2D>> m_TilesMap;

		Hazel::Ref<Hazel::FrameBuffer> m_Framebuffer;

		Hazel::OrthoGraphicCameraController m_CameraController;

		glm::vec4 m_QuadColor = { 0.5412f, 0.1686f, 0.8863f, 1.0f };
	};
}
