#include "NPC.h"
#include <cmath>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

NPC::NPC(std::string n, glm::vec3 pos, glm::vec3 color)
    : name(std::move(n)),
    position(pos),
    body(std::make_shared<Mesh>(Mesh::Box(glm::vec3(0.6f, 1.8f, 0.6f), color))),
    nose(std::make_shared<Mesh>(Mesh::Box(glm::vec3(0.16f, 0.16f, 0.3f), glm::vec3(1.0f, 0.9f, 0.6f))))
{
}

void NPC::SetModel(std::shared_ptr<Model> m, float scale)
{
    model = std::move(m);
    modelScale = scale;
}

void NPC::FaceTowards(const glm::vec3& target, float dt)
{
    glm::vec3 d = target - position;
    if (std::abs(d.x) < 1e-4f && std::abs(d.z) < 1e-4f) return;

    float targetYaw = glm::degrees(std::atan2(d.x, d.z));
    float diff = std::fmod(targetYaw - yaw + 540.0f, 360.0f) - 180.0f;   
    yaw += diff * std::min(1.0f, 8.0f * dt);
}

void NPC::MakeFish()
{
    body->Delete();
    nose->Delete();
    body = std::make_shared<Mesh>(Mesh::Box(glm::vec3(0.28f, 0.22f, 0.65f), glm::vec3(0.35f, 0.70f, 0.95f)));
    nose = std::make_shared<Mesh>(Mesh::Box(glm::vec3(0.18f, 0.16f, 0.22f), glm::vec3(0.95f, 0.55f, 0.20f)));
    height = 0.25f;
    radius = 0.35f;
}

void NPC::MakeDevice()
{
    body->Delete();
    nose->Delete();
    body = std::make_shared<Mesh>(Mesh::Box(glm::vec3(0.50f, 0.90f, 0.50f), glm::vec3(0.22f, 0.24f, 0.30f)));
    nose = std::make_shared<Mesh>(Mesh::Box(glm::vec3(0.32f, 0.20f, 0.10f), glm::vec3(0.30f, 1.50f, 0.55f)));
    height = 0.90f;
    radius = 0.40f;
    noseZ = 0.27f;
}

void NPC::Update(float dt, const glm::vec3& playerPos)
{
    if (!active) return;
    if (talking)
    {
        FaceTowards(playerPos, dt);
        return;
    }
    if (patrol.empty()) return;

    glm::vec3 d = patrol[patrolIndex] - position;
    d.y = 0.0f;
    float dist = glm::length(d);

    if (dist < 0.1f)
    {
        patrolIndex = (patrolIndex + 1) % patrol.size();
        return;
    }
    d /= dist;
    position += d * walkSpeed * dt;
    FaceTowards(position + d, dt * 2.0f);
}

void NPC::Draw(Shader& shader)
{
    if (!active) return;
    glm::mat4 base = glm::translate(glm::mat4(1.0f), position);
    base = glm::rotate(base, glm::radians(yaw), glm::vec3(0, 1, 0));

    if (model)
    {
        model->Draw(shader, glm::scale(base, glm::vec3(modelScale)));
        return;
    }
    body->Draw(shader, glm::translate(base, glm::vec3(0.0f, height * 0.5f, 0.0f)));
    nose->Draw(shader, glm::translate(base, glm::vec3(0.0f, height * 0.85f, noseZ)));
}

AABB NPC::Bounds() const
{
    return AABB{ position + glm::vec3(-radius, 0.0f, -radius),
                 position + glm::vec3(radius, height, radius) };
}

void NPC::Delete()
{
    if (!model)
    {
        body->Delete();
        nose->Delete();
    }
}
