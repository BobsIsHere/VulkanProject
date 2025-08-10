#include "Window.h"

Window::Window(Camera* pCamera) :
    m_Window{},
	m_pCamera{ pCamera }
{
}

Window::~Window()
{
}

void Window::Initialize(const int width, const int height, const char* title)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(m_Window, this);

    glfwSetFramebufferSizeCallback(m_Window, FramebufferResizeCallback);

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        auto* wnd{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (wnd && wnd->m_pCamera)
        {
            wnd->m_pCamera->ProcessKeyboardInput(key, scancode, action, mods);
        }
    });

    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos)
    {
        auto* wnd{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (wnd && wnd->m_pCamera)
        {
            wnd->m_pCamera->ProcessMouseMovement(window, xpos, ypos);
        }
    });

    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
    {
        auto* wnd{ static_cast<Window*>(glfwGetWindowUserPointer(window)) };
        if (wnd && wnd->m_pCamera)
        {
            wnd->m_pCamera->ProcessMouseEvent(window, button, action, mods);
        }
    });
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
