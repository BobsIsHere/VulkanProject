#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

class Window final
{
public:
	Window();
	~Window();

	void Initialize(const int width, const int height, const char* title);
	GLFWwindow* GetWindow() const;

	bool GetFramebufferResized() const;
	void SetFramebufferResized(bool frameBufferResized);

private:
	bool m_FramebufferResized = false;
	GLFWwindow* m_Window;

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
};