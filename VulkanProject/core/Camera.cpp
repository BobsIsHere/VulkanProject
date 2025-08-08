#include <GLFW/glfw3.h>
#include "Camera.h"

Camera::Camera(glm::vec3 startPos) :
	m_Position{ startPos },
	m_Yaw{ 0.f }, 
	m_Pitch{ 0.f },
	m_MovementSpeed{ 2.5f },
	m_MouseSensitivity{ 0.1f }
{
}

void Camera::ProcessKeyboardInput(int key, float deltaTime)
{
    glm::vec3 front{};
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = 0.f;
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front = glm::normalize(front);

    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.f, 1.f, 0.f)));

    const float velocity{ m_MovementSpeed * deltaTime };
    if (key == GLFW_KEY_W)
    {
        m_Position += front * velocity;
    }
    if (key == GLFW_KEY_S)
    {
        m_Position -= front * velocity;
    }
    if (key == GLFW_KEY_A)
    {
        m_Position -= right * velocity;
    }
    if (key == GLFW_KEY_D)
    {
        m_Position += right * velocity;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset)
{
	xoffset *= m_MouseSensitivity;
	yoffset *= m_MouseSensitivity;

	m_Yaw += xoffset;
	m_Pitch -= yoffset;

	// Clamp pitch to prevent flipping
    const float maxOrientation{ 89.f };
	if (m_Pitch > maxOrientation)
    {
        m_Pitch = maxOrientation;
    }
	if (m_Pitch < -maxOrientation)
    {
        m_Pitch = -maxOrientation;
    }
}

glm::mat4 Camera::GetViewMatrix() const
{
    glm::vec3 front{};
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front = glm::normalize(front);

	return glm::lookAt(m_Position, m_Position + front, glm::vec3(0.f, 1.f, 0.f));
}