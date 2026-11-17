#include "World.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void World::BuildDemo()
{
    floor = std::make_unique<Mesh>(Mesh::Plane(60.0f, 60.0f, 30.0f, glm::vec3(0.35f, 0.6f, 0.3f)));

    // Стены по периметру
    glm::vec3 wallColor(0.7f, 0.65f, 0.55f);
    AddBox(glm::vec3(0, 1.5f, -30), glm::vec3(60, 3, 1), wallColor);
    AddBox(glm::vec3(0, 1.5f, 30), glm::vec3(60, 3, 1), wallColor);
    AddBox(glm::vec3(-30, 1.5f, 0), glm::vec3(1, 3, 60), wallColor);
    AddBox(glm::vec3(30, 1.5f, 0), glm::vec3(1, 3, 60), wallColor);

    // Домик и ящики
    AddBox(glm::vec3(-8, 1.5f, -8), glm::vec3(6, 3, 5), glm::vec3(0.75f, 0.35f, 0.3f));
    AddBox(glm::vec3(6, 0.5f, -3), glm::vec3(1, 1, 1), glm::vec3(0.6f, 0.45f, 0.25f));
    AddBox(glm::vec3(7.2f, 0.5f, -3), glm::vec3(1, 1, 1), glm::vec3(0.6f, 0.45f, 0.25f));
    AddBox(glm::vec3(6.6f, 1.5f, -3), glm::vec3(1, 1, 1), glm::vec3(0.6f, 0.45f, 0.25f));
}

void World::SetFloorTexture(const std::string& path, bool pixelated)
{
    if (!floor) return;
    Texture t;
    if (t.Load(path, pixelated)) floor->SetTexture(t);
}

bool World::LoadMap(const std::string& path, float scale, bool pixelated)
{
    map = std::make_unique<Model>();
    if (!map->Load(path, scale, pixelated))
    {
        map.reset();
        return false;
    }
    for (const AABB& c : map->colliders) colliders.push_back(c);
    return true;
}

void World::AddBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color, bool solid)
{
    props.push_back(Prop{ Mesh::Box(size, color), center });
    if (solid)
        colliders.push_back(AABB{ center - size * 0.5f, center + size * 0.5f });
}

void World::Update(float dt, const glm::vec3& playerPos)
{
    for (auto& n : npcs) n.Update(dt, playerPos);
}

void World::Draw(Shader& shader)
{
    if (floor) floor->Draw(shader, glm::mat4(1.0f));
    if (map) map->Draw(shader, glm::mat4(1.0f));
    for (auto& p : props)
        p.mesh.Draw(shader, glm::translate(glm::mat4(1.0f), p.position));
    for (auto& n : npcs) n.Draw(shader);
}

void World::Delete()
{
    if (floor) floor->Delete(true);
    if (map) map->Delete();
    for (auto& p : props) p.mesh.Delete();
    for (auto& n : npcs) n.Delete();
}

bool World::Collides(const glm::vec3& feet, float radius, float height) const
{
    // небольшой отступ снизу, чтобы пол и "земля" не считались препятствием
    AABB player{ glm::vec3(feet.x - radius, feet.y + 0.05f, feet.z - radius),
                 glm::vec3(feet.x + radius, feet.y + height, feet.z + radius) };

    for (const AABB& c : colliders)
        if (AABBOverlap(player, c)) return true;

    for (const NPC& n : npcs)
        if (AABBOverlap(player, n.Bounds())) return true;

    return false;
}

NPC* World::FindInteractable(const glm::vec3& eyePos, const glm::vec3& lookDir, float maxDist)
{
    NPC* best = nullptr;
    float bestDot = 0.85f; // ~32 градуса от центра экрана

    glm::vec3 look = glm::normalize(lookDir);
    for (NPC& n : npcs)
    {
        glm::vec3 to = n.Center() - eyePos;
        float dist = glm::length(to);
        if (dist > maxDist || dist < 1e-4f) continue;

        float d = glm::dot(to / dist, look);
        if (d > bestDot)
        {
            bestDot = d;
            best = &n;
        }
    }
    return best;
}
