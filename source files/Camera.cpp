#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(int w, int h, glm::vec3 position)
    : Position(position), width(w), height(h)
{
    UpdateVectors();
}

void Camera::Look(GLFWwindow* window, bool enabled)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);

    if (firstMouse)
    {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    double dx = x - lastX;
    double dy = lastY - y; // инверсия: мышь вверх = смотреть вверх
    lastX = x;
    lastY = y;

    if (!enabled) return;

    yaw += (float)dx * sensitivity;
    pitch += (float)dy * sensitivity;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
    UpdateVectors();
}

void Camera::UpdateVectors()
{
    float yawR = glm::radians(yaw);
    float pitchR = glm::radians(pitch);
    Orientation = glm::normalize(glm::vec3(
        cos(yawR) * cos(pitchR),
        sin(pitchR),
        sin(yawR) * cos(pitchR)));
}

void Camera::Matrix(Shader& shader, const char* uniform) const
{
    glm::mat4 view = glm::lookAt(Position, Position + Orientation, Up);
    glm::mat4 proj = glm::perspective(glm::radians(fov), (float)width / (float)height, nearPlane, farPlane);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(proj * view));
}