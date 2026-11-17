#pragma once
#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Model.h"
#include "NPC.h"
#include "AABB.h"

class World
{
public:
    // Вариант 1: тестовая карта из коробок (работает без единого ассета)
    void BuildDemo();
    void SetFloorTexture(const std::string& path, bool pixelated = false);

    // Вариант 2: настоящая карта из .obj (объекты "col_*" станут коллизиями)
    bool LoadMap(const std::string& path, float scale = 1.0f, bool pixelated = false);

    void AddBox(const glm::vec3& center, const glm::vec3& size, const glm::vec3& color, bool solid = true);
    void AddCollider(const AABB& box) { colliders.push_back(box); }
    void AddNPC(const NPC& npc) { npcs.push_back(npc); }

    void Update(float dt, const glm::vec3& playerPos);
    void Draw(Shader& shader);
    void Delete();

    // Пересекается ли "коробка игрока" со стенами или NPC
    bool Collides(const glm::vec3& feetPos, float radius, float height) const;

    // NPC, на которого смотрит игрок и который достаточно близко (или nullptr)
    NPC* FindInteractable(const glm::vec3& eyePos, const glm::vec3& lookDir, float maxDist = 2.5f);

    std::vector<NPC> npcs;

private:
    struct Prop
    {
        Mesh mesh;
        glm::vec3 position;
    };

    std::unique_ptr<Model> map;
    std::unique_ptr<Mesh> floor;
    std::vector<Prop> props;
    std::vector<AABB> colliders;
};
