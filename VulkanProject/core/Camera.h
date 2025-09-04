#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera final
{
public:
	Camera(glm::vec3 startPos);

	glm::mat4 Update();

	void ProcessKeyboardInput(int key, int scancode, int action, int mods);
	void ProcessMouseMovement(GLFWwindow* window, double xpos, double ypos);
	void ProcessMouseEvent(GLFWwindow* window, int button, int action, int mods);

private:
	glm::vec2 m_LastMousePosition;
	glm::vec3 m_CameraPosition;
	glm::vec3 m_CameraForward{ 0.f, 0.f, -1.f };
	glm::vec3 m_CameraUp{ 0.f, 1.f, 0.f };
	glm::vec3 m_CameraRight{ 1.f, 0.f, 0.f };

	float m_Yaw;
	float m_Pitch;

	const float m_MovementSpeed = 50.f;
	const float m_MouseSensitivity = 0.1f;
	const float m_MaxAngle = 89.f;
};