#include "Player.h"

Player::Player(int w, int h, glm::vec3 startPos)
    : Position(startPos), camera(w, h, startPos)
{
    camera.Position = EyePosition();
}

void Player::Update(GLFWwindow* window, float dt, const World& world, bool inputEnabled)
{
    camera.Look(window, inputEnabled);

    glm::vec3 forward = glm::normalize(glm::vec3(camera.Orientation.x, 0.0f, camera.Orientation.z));
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.Up));

    glm::vec3 move(0.0f);
    bool running = false;

    if (inputEnabled)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;
        running = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround)
        {
            velocityY = jumpSpeed;
            onGround = false;
        }
    }

    const float maxStepUp = 0.45f;

    // --- горизонталь: ось X и Z по отдельности, чтобы скользить вдоль стен ---
    if (glm::length(move) > 0.0f)
    {
        float speed = running ? runSpeed : walkSpeed;
        glm::vec3 delta = glm::normalize(move) * speed * dt;

        Position.x += delta.x;
        if (world.Collides(Position, radius, height) ||
            world.GetFloorHeight(Position.x, Position.z) - Position.y > maxStepUp)
        {
            Position.x -= delta.x;
        }

        Position.z += delta.z;
        if (world.Collides(Position, radius, height) ||
            world.GetFloorHeight(Position.x, Position.z) - Position.y > maxStepUp)
        {
            Position.z -= delta.z;
        }
    }

    // --- вертикаль: гравитация, прыжок и плавная лестница ---
    bool wasOnGround = onGround;
    groundY = world.GetFloorHeight(Position.x, Position.z);

    velocityY -= gravity * dt;
    Position.y += velocityY * dt;

    if (Position.y <= groundY)
    {
        Position.y = groundY;
        velocityY = 0.0f;
        onGround = true;
    }
    else if (wasOnGround && velocityY <= 0.0f && (Position.y - groundY) < 0.25f)
    {
        // Smoothly step down stairs without micro-bouncing
        Position.y = groundY;
        velocityY = 0.0f;
        onGround = true;
    }
    else
    {
        onGround = false;
    }

    camera.Position = EyePosition();
}