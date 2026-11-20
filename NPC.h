#pragma once
#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Mesh.h"
#include "Model.h"
#include "AABB.h"

struct DialogueLine
{
    std::string speaker;
    std::string text;
};

class NPC
{
public:
    std::string name;
    glm::vec3 position;
    float yaw = 0.0f;          // 0 = смотрит в +Z
    float radius = 0.4f;
    float height = 1.8f;
    bool active = true;
    void MakeFish();
    void MakeDevice();   // маленькое устройство-"якорь" со светящимся индикатором

    std::vector<DialogueLine> dialogue;
    std::string givesQuestId; // empty if this NPC doesn't give a quest
    // Патруль: если список не пуст, NPC ходит по точкам по кругу
    std::vector<glm::vec3> patrol;
    float walkSpeed = 1.2f;
    float noseZ = 0.4f;        // вынос "носа" вперёд от центра

    bool talking = false;

    // Без модели рисуется цветная "капсула-коробка" с носом (видно, куда смотрит)
    NPC(std::string name, glm::vec3 position, glm::vec3 color = glm::vec3(0.8f, 0.3f, 0.2f));

    void SetModel(std::shared_ptr<Model> m, float scale = 1.0f);

    void Update(float dt, const glm::vec3& playerPos);
    void Draw(Shader& shader);

    void StartTalking() { talking = true; }
    void StopTalking() { talking = false; }

    AABB Bounds() const;
    glm::vec3 Center() const { return position + glm::vec3(0.0f, height * 0.5f, 0.0f); }

    void Delete();

private:
    size_t patrolIndex = 0;
    std::shared_ptr<Model> model;
    float modelScale = 1.0f;
    std::shared_ptr<Mesh> body;
    std::shared_ptr<Mesh> nose;

    void FaceTowards(const glm::vec3& target, float dt);
};
