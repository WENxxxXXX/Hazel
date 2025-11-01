#pragma once

#include "Hazel.h"

#include "ParticleSystem.h"

class Sandbox2D : public Hazel::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Hazel::Timestep ts) override;
	void OnImGuiRender() override;
	void OnEvent(Hazel::Event& event) override;
private:
	Hazel::Ref<Hazel::Texture2D> m_Texture;
	Hazel::Ref<Hazel::Texture2D> m_Emoji;
	Hazel::OrthoGraphicCameraController m_CameraController;

	glm::vec4 m_QuadColor = { 0.5412f, 0.1686f, 0.8863f, 1.0f };

	ParticleProps m_Particle;
	ParticleSystem m_ParticleSystem;
};