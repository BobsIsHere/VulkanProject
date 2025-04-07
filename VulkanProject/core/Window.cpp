#include "Window.h"

Window::Window() :
    m_Window{}
{
}

Window::~Window()
{
}

void Window::Initialize(const int width, const int height, const char* title)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);
}

GLFWwindow* Window::GetWindow() const
{
	return m_Window;
}

bool Window::GetFramebufferResized() const
{
    return m_FramebufferResized;
}

void Window::SetFramebufferResized(bool frameBufferResized)
{
    m_FramebufferResized = frameBufferResized;
}

void Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized = true;
}
