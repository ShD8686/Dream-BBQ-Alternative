#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "shaderClass.h"

// Камера от первого лица (мышь = обзор). Позицию задаёт Player.
class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

    int width, height;
    float fov = 70.0f;
    float nearPlane = 0.05f;
    float farPlane = 300.0f;
    float sensitivity = 0.1f;

    Camera(int width, int height, glm::vec3 position);

    // enabled = false -> мышь отслеживается, но взгляд не вращается (например, в диалоге)
    void Look(GLFWwindow* window, bool enabled);

    // Кладёт proj * view в uniform шейдера
    void Matrix(Shader& shader, const char* uniform) const;

private:
    float yaw = -90.0f;
    float pitch = 0.0f;
    double lastX = 0.0, lastY = 0.0;
    bool firstMouse = true;

    void UpdateVectors();
};
