#include "hzpch.h"
#include "Hazel/Renderer/OrthoGraphicCameraController.h"

#include "Hazel/Core/Input.h"
#include "Hazel/Core/KeyCodes.h"

namespace Hazel {


	OrthoGraphicCameraController::OrthoGraphicCameraController(float aspectRatio, bool rotation)
		:m_AspectRatio(aspectRatio), m_Rotation(rotation), m_Camera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top),
		m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel })
	{
	}

	void OrthoGraphicCameraController::OnUpdate(Timestep& ts)
	{
		HZ_PROFILE_FUNCTION();

		if (m_Rotation) {
			if (Input::IsKeyPressed(HZ_KEY_Q))
				m_CameraRotation -= m_CameraRotationSpeed * ts;
			else if (Hazel::Input::IsKeyPressed(HZ_KEY_E))
				m_CameraRotation += m_CameraRotationSpeed * ts;

			// ���Ƕȿ����� 180��~ -180����(��360��)���ڲ�Ӱ�������������¼��ټ���ɱ��������ڲ��ϸ����е��½Ƕȹ���
			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			m_Camera.SetRotation(m_CameraRotation);
		}

		//if (Input::IsKeyPressed(HZ_KEY_A))
		//	m_CameraPosition.x += m_CameraTranslationSpeed * ts;
		//else if (Hazel::Input::IsKeyPressed(HZ_KEY_D))
		//	m_CameraPosition.x -= m_CameraTranslationSpeed * ts;

		//if (Input::IsKeyPressed(HZ_KEY_W))
		//	m_CameraPosition.y -= m_CameraTranslationSpeed * ts;
		//else if (Hazel::Input::IsKeyPressed(HZ_KEY_S))
		//	m_CameraPosition.y += m_CameraTranslationSpeed * ts;

		if (Input::IsKeyPressed(HZ_KEY_A)) {
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_D)) {
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		if (Input::IsKeyPressed(HZ_KEY_W)) {
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}
		else if (Hazel::Input::IsKeyPressed(HZ_KEY_S)) {
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
		}

		m_Camera.SetPosition(m_CameraPosition);

		m_CameraTranslationSpeed = m_ZoomLevel;

	}

	void OrthoGraphicCameraController::OnEvent(Event& e)
	{
		HZ_PROFILE_FUNCTION();

		EventDispatcher dispathcer(e);
		dispathcer.Dispatch<MouseScrolledEvent>(HZ_BIND_EVENT_FN(OrthoGraphicCameraController::OnMouseScrolled));
		dispathcer.Dispatch<WindowResizeEvent>(HZ_BIND_EVENT_FN(OrthoGraphicCameraController::OnWindowResized));
	}

	bool OrthoGraphicCameraController::OnMouseScrolled(MouseScrolledEvent e)
	{
		HZ_PROFILE_FUNCTION();

		m_ZoomLevel -= e.GetYOffset() * 0.5f;//Offset ����ǰ����ʱͨ��Ϊ������������ʱͨ��Ϊ����
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };	//�����������ڵ�����ռ�ı߽磬�Ա���¾���
		m_Camera.SetProjectionMatrix(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
		return false;//���������Ҫһֱ��ѯ���ʷ��� false������ֹ���¼��ķַ�������
	}

	bool OrthoGraphicCameraController::OnWindowResized(WindowResizeEvent e)
	{
		HZ_PROFILE_FUNCTION();

		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();//���ûص��Ŀ�߱�
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjectionMatrix(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
		return false;
	}

}