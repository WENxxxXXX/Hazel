#pragma once

#include "Hazel/Core/Timestep.h"

#include "Hazel/Events/MouseEvent.h"
#include "Hazel/Events/ApplicationEvent.h"

#include "Hazel/Renderer/OrthoGraphicCamera.h"

namespace Hazel {

	struct OrthoGraphicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }		//���������x�����ӽǵĿ��
		float GetHeight() { return Top - Bottom; }		//���������y�����ӽǵĸ߶�
	};

	class OrthoGraphicCameraController
	{
	public:
		OrthoGraphicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep& ts);
		void OnEvent(Event& e);

		OrthoGraphicCamera& GetCamera() { return m_Camera; }
		const OrthoGraphicCamera& GetCamera() const { return m_Camera; }
		
		void SetZoomLevel(float level) { m_ZoomLevel = level; }
		float GetZoomLevel() const { return m_ZoomLevel; }

		// ��ȡ�������ǰλ�á����������״̬��Left, Right, Bottom, Top��
		const OrthoGraphicCameraBounds& GetBounds() const { return m_Bounds; }
	private:
		bool OnMouseScrolled(MouseScrolledEvent e);
		bool OnWindowResized(WindowResizeEvent e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		// ��¼�����״̬�������䴢������ݳ�ʼ������������������������û�з��ص�ǰ���ݵĺ�������ʹ��Bounds����¼����㡣
		OrthoGraphicCameraBounds m_Bounds;
		// ��ʼ�� camera ʱ������ʹ���� bounds ���丳ֵ�����Ա������������� bounds��������ִ���
		OrthoGraphicCamera m_Camera;

		bool m_Rotation;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f;
		float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
	};

}