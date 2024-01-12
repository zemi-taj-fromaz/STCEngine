#include "Layer.h"

#include "Platform/Vulkan/AppVulkanImpl.h"

Layer::Layer(const std::string& name) : m_DebugName(name)
{
}


bool Layer::poll_inputs(GLFWwindow* window, float deltaTime)
{

    float cameraSpeed = 100.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        m_Camera.update_position(cameraSpeed * deltaTime * m_Camera.Front);
    }


    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        m_Camera.update_position(cameraSpeed * deltaTime * m_Camera.Right);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        m_Camera.update_position(-cameraSpeed * deltaTime * m_Camera.Front);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        m_Camera.update_position(-cameraSpeed * deltaTime * m_Camera.Right);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        m_Camera.update_position(cameraSpeed * deltaTime * m_Camera.Up);
    }


    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        m_Camera.update_position(-cameraSpeed * deltaTime * m_Camera.Up);
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        return false;
    }

    return true;
}

void Layer::set_callbacks(GLFWwindow* window)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
        app->set_frame_buffer_resized();
        //TODO
        //app->set_frame_b

        });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
        app->set_field_of_view(static_cast<float>(yoffset));
        //app->setScrollCallback();
        });


    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<AppVulkanImpl*>(glfwGetWindowUserPointer(window));
            glm::vec2 mousePosition = app->get_mouse_position();

            app->set_mouse_position({ static_cast<float>(xpos), static_cast<float>(ypos) });

            float xoffset = static_cast<float>(xpos) - mousePosition.x;
            float yoffset = mousePosition.y - static_cast<float>(ypos); // reversed since y-coordinates range from bottom to top

            const float sensitivity = 0.05f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            app->process_mouse_movement(xoffset, yoffset);
            // app->process_mouse_movement(0.0f, 0.0f);
        });

}

