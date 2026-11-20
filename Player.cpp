#include "Player.h"
#include <algorithm>
#include <cmath>

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
    bool sprinting = false;

    if (inputEnabled)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) move += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) move -= forward;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) move += right;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) move -= right;

        running = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        sprinting = glfwGetKey(window, sprintKey) == GLFW_PRESS;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && onGround)
        {
            velocityY = jumpSpeed;
            onGround = false;
        }
    }

    const float maxStepUp = 0.45f;
    const glm::vec3 startPos = Position;

    // --- горизонталь: ось X и Z по отдельности, чтобы скользить вдоль стен ---
    if (glm::length(move) > 0.0f)
    {
        float moveSpeed = walkSpeed;
        if (sprinting)
            moveSpeed = sprintSpeed;
        else if (running)
            moveSpeed = runSpeed;

        glm::vec3 delta = glm::normalize(move) * moveSpeed * dt;

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

    // --- эффекты бега и спринта: FOV шире + покачивание камеры ---
    if (!runFx)
    {
        camera.fov = baseFov;
        bobBlend = 0.0f;
        return;
    }

    float moved = glm::length(glm::vec2(Position.x - startPos.x, Position.z - startPos.z));
    float actualSpeed = moved / std::max(dt, 1e-4f);

    bool isSprinting = onGround && sprinting && actualSpeed > sprintSpeed * 1.5f;
    bool isRunning = !isSprinting && onGround && running && actualSpeed > runSpeed * 0.5f;

    float fovKick = 0.0f;
    if (isSprinting)
        fovKick = sprintFovKick;
    else if (isRunning)
        fovKick = runFovKick;

    float targetFov = baseFov + fovKick;
    camera.fov += (targetFov - camera.fov) * std::min(1.0f, 6.0f * dt);

    float targetBob = (onGround && actualSpeed > 0.3f) ? 1.0f : 0.0f;
    bobBlend += (targetBob - bobBlend) * std::min(1.0f, 10.0f * dt);
    bobPhase = std::fmod(bobPhase + moved * bobPerMeter, 6.2831853f);

    camera.Position += glm::vec3(0.0f, std::sin(bobPhase * 2.0f) * bobHeight * bobBlend, 0.0f)
        + right * (std::sin(bobPhase) * bobSway * bobBlend);
}