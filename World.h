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
    void Clear();
    void PlugFountainWithFish();
    void BuildDarkStairsScene();
    void BuildNextScene();
    // Передать шейдеру положения ламп (strength 0 = выключить подсветку)
    void UploadLights(Shader& shader, float strength) const;
    // Устройство "якорь": дворец перестаёт отдаляться и остаётся на месте (z), у него появляется стена
    void SetPalaceAnchor(float z);
    bool PalaceDoorNear(const glm::vec3& p) const;
    // Лампочка на столбе-"виселице", качается над точкой (x, z) и светит как подвижный точечный источник
    void AddSwingLamp(float x, float z);
    float GetDarkStairsHeight(float x, float z) const;
    float GetFloorHeight(float x, float z) const;

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
        glm::vec3 rotation = glm::vec3(0.0f);
    };
    struct FlowerInstance
    {
        glm::vec3 pos;
        glm::vec3 scale;
        int colorIndex;
    };
    // Фонарь-подсказка: столб, рука в сторону target (x, z), лампа и точечный источник света
    void AddLamp(const glm::vec3& basePos, const glm::vec2& target);
    // Цепочка фонарей от точки from до точки to (обе как x, z). Фонари чередуются слева/справа от тропы
    void AddLampTrail(const glm::vec2& from, const glm::vec2& to, float spacing = 12.0f, float sideOffset = 3.2f);

    glm::vec3 viewPos = glm::vec3(0.0f); // последняя позиция игрока (для отсечения далёких объектов)
    std::vector<Mesh> flowerMeshes;
    std::vector<FlowerInstance> flowers;
    bool inPalaceScene = false;
    bool palaceAnchored = false;
    float palaceAnchorZ = 0.0f;
    std::vector<glm::vec3> lampLights;
    bool swingActive = false;
    glm::vec3 swingPivot = glm::vec3(0.0f);
    float swingLen = 1.5f;
    float swingTime = 0.0f;
    glm::vec3 SwingBulbPos() const;
    float SwingAngle() const;
    std::unique_ptr<Mesh> swingCord, swingBulb;
    float palaceZ = 0.0f;
    std::vector<Prop> palaceProps;
    bool inDarkStairs = false;
    std::unique_ptr<Model> map;
    std::vector<size_t> waterStreamIndices;
    std::unique_ptr<Mesh> floor;
    std::vector<Prop> props;
    std::vector<AABB> colliders;
};
