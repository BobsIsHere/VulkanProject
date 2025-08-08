#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera final
{
public:
	Camera(glm::vec3 startPos);

	void ProcessKeyboardInput(int key, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset);

	glm::mat4 GetViewMatrix() const;

private:
	glm::vec3 m_Position;

	float m_Yaw;
	float m_Pitch;

	const float m_MovementSpeed = 100.f;
	const float m_MouseSensitivity = 0.001f;
};