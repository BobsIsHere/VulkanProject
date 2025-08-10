#include <GLFW/glfw3.h>
#include "Camera.h"

Camera::Camera(glm::vec3 startPos) :
	m_CameraPosition{ startPos },
	m_LastMousePosition{ 0.f, 0.f },
	m_Yaw{ -90.f }, 
	m_Pitch{ 0.f },
	m_MovementSpeed{ 1.f },
	m_MouseSensitivity{ 0.1f }
{
}

glm::mat4 Camera::Update()
{
	//glm::vec3 newForward{ glm::normalize(glm::rotate(glm::mat4(1.0f), -m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
	//					  glm::rotate(glm::mat4(1.0f), m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
	//					  glm::vec4(m_CameraForward, 1.0f))
	//};

	//m_CameraForward = newForward;
	//m_CameraRight = glm::cross(m_CameraForward, glm::vec3(0.f, 1.f, 0.f));
	//m_CameraUp = glm::cross(m_CameraRight, m_CameraForward);

	//glm::mat4 viewMatrix{ glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraForward, m_CameraUp) };

	//return viewMatrix;

	float yawRad{ glm::radians(m_Yaw) };
	float pitchRad{ glm::radians(m_Pitch) };

	glm::vec3 newForward;
	newForward.x = cos(pitchRad) * cos(yawRad);
	newForward.y = sin(pitchRad);
	newForward.z = cos(pitchRad) * sin(yawRad);

	m_CameraForward = glm::normalize(newForward);
	m_CameraRight = glm::normalize(glm::cross(m_CameraForward, glm::vec3(0.f, 1.f, 0.f)));
	m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraForward));

	const glm::mat4 viewMatrix{ glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraForward, m_CameraUp) };
	return viewMatrix;
}

void Camera::ProcessKeyboardInput(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition += m_CameraForward * m_MovementSpeed;
	}
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition -= m_CameraForward * m_MovementSpeed;
	}
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition -= m_CameraRight * m_MovementSpeed;
	}
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition += m_CameraRight * m_MovementSpeed;
	}
}

void Camera::ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos)
{
	glm::vec2 currentMousePosition = glm::vec2(xpos, ypos);
	glm::vec2 mouseDelta = currentMousePosition - m_LastMousePosition;

	m_LastMousePosition = currentMousePosition;

	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (state == GLFW_PRESS)
	{
		m_Yaw += mouseDelta.x * m_MouseSensitivity;
		m_Pitch += mouseDelta.y * m_MouseSensitivity;

		if (m_Pitch > m_MaxAngle)
		{
			m_Pitch = m_MaxAngle;
		}
	}
}

void Camera::ProcessMouseEvent(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double xpos{};
		double ypos{};

		glfwGetCursorPos(window, &xpos, &ypos);

		m_LastMousePosition.x = static_cast<float>(xpos);
		m_LastMousePosition.y = static_cast<float>(ypos);
	}
}