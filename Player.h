#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Camera.h"
#include "World.h"

class Player
{
public:
    glm::vec3 Position;        
    Camera camera;

    float eyeHeight = 1.65f;
    float height = 1.8f;
    float radius = 0.3f;

    float walkSpeed = 3.0f;
    float runSpeed = 6.0f;
    float jumpSpeed = 5.0f;
    float gravity = 15.0f;
    float groundY = 0.0f;

    Player(int windowW, int windowH, glm::vec3 startPos);

    void Update(GLFWwindow* window, float dt, const World& world, bool inputEnabled);

    glm::vec3 EyePosition() const { return Position + glm::vec3(0.0f, eyeHeight, 0.0f); }

    bool runFx = false;             
    float baseFov = 70.0f;        
    float runFovKick = 12.0f;        
    float bobHeight = 0.05f;       
    float bobSway = 0.03f;           
    float bobPerMeter = 1.7f;            
    float sprintSpeed = 15.0f;               
    float sprintFovKick = 12.0f;        
    int sprintKey = GLFW_KEY_LEFT_CONTROL;       

private:
    float velocityY = 0.0f;
    bool onGround = true;
    float bobPhase = 0.0f;
    float bobBlend = 0.0f;
};
