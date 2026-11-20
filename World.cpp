#include "World.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <glm/gtc/matrix_transform.hpp>

void World::Clear()
{
    Delete();
    floor.reset();
    map.reset();
    props.clear();
    palaceProps.clear();
    flowerMeshes.clear();
    flowers.clear();
    colliders.clear();
    npcs.clear();
    waterStreamIndices.clear();
    inDarkStairs = false;
    inPalaceScene = false;
    palaceAnchored = false;
    palaceAnchorZ = 0.0f;
    lampLights.clear();
    palaceZ = 0.0f;
}

void World::BuildDemo()
{
    Clear();

    const float halfSize = 60.0f;
    const float fullSize = halfSize * 2.0f;

    glm::vec3 floorColor(0.92f, 0.94f, 0.97f);
    floor = std::make_unique<Mesh>(Mesh::Plane(fullSize, fullSize, halfSize, floorColor));

    glm::vec3 wallColor(0.84f, 0.87f, 0.91f);
    props.push_back(Prop{ Mesh::Box(glm::vec3(134.0f, 1.5f, 10.0f), wallColor), glm::vec3(0.0f, 2.8f, -60.5f), glm::vec3(45.0f, 0.0f, 0.0f) });
    props.push_back(Prop{ Mesh::Box(glm::vec3(134.0f, 1.5f, 10.0f), wallColor), glm::vec3(0.0f, 2.8f,  60.5f), glm::vec3(-45.0f, 0.0f, 0.0f) });
    props.push_back(Prop{ Mesh::Box(glm::vec3(10.0f, 1.5f, 134.0f), wallColor), glm::vec3(-60.5f, 2.8f, 0.0f), glm::vec3(0.0f, 0.0f, -45.0f) });
    props.push_back(Prop{ Mesh::Box(glm::vec3(10.0f, 1.5f, 134.0f), wallColor), glm::vec3(60.5f, 2.8f, 0.0f), glm::vec3(0.0f, 0.0f,  45.0f) });

    colliders.push_back(AABB{ glm::vec3(-62.0f, 0.0f, -62.0f), glm::vec3(62.0f, 12.0f, -57.5f) });
    colliders.push_back(AABB{ glm::vec3(-62.0f, 0.0f,  57.5f), glm::vec3(62.0f, 12.0f,  62.0f) });
    colliders.push_back(AABB{ glm::vec3(-62.0f, 0.0f, -62.0f), glm::vec3(-57.5f, 12.0f,  62.0f) });
    colliders.push_back(AABB{ glm::vec3(57.5f, 0.0f, -62.0f), glm::vec3(62.0f, 12.0f,  62.0f) });

    const float px = -14.0f, pz = -12.0f;
    const glm::vec3 pyramidYellow(0.95f, 0.80f, 0.15f);
    const int tiers = 18;
    const float tierH = 0.55f;
    for (int i = 0; i < tiers; ++i)
    {
        float w = 18.0f - i * 0.95f;
        float y = tierH * 0.5f + i * tierH;
        AddBox(glm::vec3(px, y, pz), glm::vec3(w, tierH, w), pyramidYellow);
    }

    AddBox(glm::vec3(px, 1.425f, pz + 7.65f), glm::vec3(2.1f, 2.85f, 3.0f), pyramidYellow);
    AddBox(glm::vec3(px, 1.25f, pz + 9.18f), glm::vec3(1.45f, 2.45f, 0.08f), glm::vec3(0.20f, 0.13f, 0.08f));
    AddBox(glm::vec3(px + 0.52f, 1.15f, pz + 9.25f), glm::vec3(0.12f, 0.12f, 0.08f), glm::vec3(0.95f, 0.80f, 0.20f), false);

    const float fx = 24.0f, fz = -30.0f;
    const glm::vec3 stoneColor(0.70f, 0.73f, 0.78f);
    const glm::vec3 waterColor(0.25f, 0.62f, 0.90f);
    const glm::vec3 splashColor(0.50f, 0.82f, 0.98f);

    AddBox(glm::vec3(fx, 0.35f, fz + 2.8f), glm::vec3(6.0f, 0.7f, 0.4f), stoneColor);
    AddBox(glm::vec3(fx, 0.35f, fz - 2.8f), glm::vec3(6.0f, 0.7f, 0.4f), stoneColor);
    AddBox(glm::vec3(fx - 2.8f, 0.35f, fz), glm::vec3(0.4f, 0.7f, 5.2f), stoneColor);
    AddBox(glm::vec3(fx + 2.8f, 0.35f, fz), glm::vec3(0.4f, 0.7f, 5.2f), stoneColor);
    AddBox(glm::vec3(fx, 0.08f, fz), glm::vec3(5.4f, 0.16f, 5.4f), stoneColor);
    AddBox(glm::vec3(fx, 0.48f, fz), glm::vec3(5.2f, 0.08f, 5.2f), waterColor);

    AddBox(glm::vec3(fx, 1.0f, fz), glm::vec3(0.9f, 2.0f, 0.9f), stoneColor);
    AddBox(glm::vec3(fx, 1.85f, fz), glm::vec3(2.6f, 0.2f, 2.6f), stoneColor);
    AddBox(glm::vec3(fx, 2.075f, fz + 1.2f), glm::vec3(2.6f, 0.25f, 0.2f), stoneColor);
    AddBox(glm::vec3(fx, 2.075f, fz - 1.2f), glm::vec3(2.6f, 0.25f, 0.2f), stoneColor);
    AddBox(glm::vec3(fx - 1.2f, 2.075f, fz), glm::vec3(0.2f, 0.25f, 2.2f), stoneColor);
    AddBox(glm::vec3(fx + 1.2f, 2.075f, fz), glm::vec3(0.2f, 0.25f, 2.2f), stoneColor);
    AddBox(glm::vec3(fx, 2.06f, fz), glm::vec3(2.2f, 0.08f, 2.2f), waterColor);
    AddBox(glm::vec3(fx, 2.40f, fz), glm::vec3(0.45f, 0.9f, 0.45f), stoneColor);

    waterStreamIndices.push_back(props.size());
    AddBox(glm::vec3(fx, 3.10f, fz), glm::vec3(0.22f, 0.5f, 0.22f), splashColor, false);
    waterStreamIndices.push_back(props.size());
    AddBox(glm::vec3(fx, 1.325f, fz + 1.35f), glm::vec3(0.5f, 1.65f, 0.12f), splashColor, false);
    waterStreamIndices.push_back(props.size());
    AddBox(glm::vec3(fx, 1.325f, fz - 1.35f), glm::vec3(0.5f, 1.65f, 0.12f), splashColor, false);
    waterStreamIndices.push_back(props.size());
    AddBox(glm::vec3(fx + 1.35f, 1.325f, fz), glm::vec3(0.12f, 1.65f, 0.5f), splashColor, false);
    waterStreamIndices.push_back(props.size());
    AddBox(glm::vec3(fx - 1.35f, 1.325f, fz), glm::vec3(0.12f, 1.65f, 0.5f), splashColor, false);

    AddLampTrail(glm::vec2(2.0f, 3.0f),    glm::vec2(17.0f, -22.0f));         
    AddLampTrail(glm::vec2(-4.0f, 14.0f),  glm::vec2(-39.0f, 40.0f));        
    AddLampTrail(glm::vec2(-5.0f, 3.0f),   glm::vec2(-13.0f, 0.5f));
    AddLamp(glm::vec3(29.5f, 0.0f, -27.0f), glm::vec2(24.0f, -30.0f));
    AddLamp(glm::vec3(-43.5f, 0.0f, 49.0f), glm::vec2(-46.0f, 46.0f));
    AddLamp(glm::vec3(-49.0f, 0.0f, 43.0f), glm::vec2(-46.0f, 46.0f));
}

void World::PlugFountainWithFish()
{
    for (size_t idx : waterStreamIndices)
    {
        if (idx < props.size())
            props[idx].position.y = -1000.0f;
    }

    const float fx = 24.0f, fz = -30.0f;
    AddBox(glm::vec3(fx, 3.10f, fz), glm::vec3(0.45f, 0.70f, 0.35f), glm::vec3(0.35f, 0.70f, 0.95f), false);
    AddBox(glm::vec3(fx, 3.55f, fz), glm::vec3(0.55f, 0.25f, 0.18f), glm::vec3(0.95f, 0.55f, 0.20f), false);
}

void World::BuildDarkStairsScene()
{
    Clear();
    inDarkStairs = true;

    floor = std::make_unique<Mesh>(Mesh::Plane(90.0f, 110.0f, 45.0f, glm::vec3(0.06f, 0.06f, 0.08f)));

    glm::vec3 darkWall(0.03f, 0.03f, 0.05f);
    AddBox(glm::vec3(0, 12.0f, -55.0f), glm::vec3(90, 24, 1), darkWall);
    AddBox(glm::vec3(0, 12.0f, 55.0f), glm::vec3(90, 24, 1), darkWall);
    AddBox(glm::vec3(-45, 12.0f, 0.0f), glm::vec3(1, 24, 110), darkWall);
    AddBox(glm::vec3(45, 12.0f, 0.0f), glm::vec3(1, 24, 110), darkWall);

    glm::vec3 stairColor(0.16f, 0.16f, 0.20f);
    const int steps = 16;
    const float stepH = 0.30f;
    for (int i = 0; i < steps; ++i)
    {
        float topY = (i + 1) * stepH;
        AddBox(glm::vec3(0.0f, topY * 0.5f, -4.5f - i * 1.0f), glm::vec3(7.0f, topY, 1.0f), stairColor, false);
    }

    AddBox(glm::vec3(0.0f, 2.40f, -23.0f), glm::vec3(7.0f, 4.80f, 6.0f), stairColor, false);
    AddBox(glm::vec3(0.0f, 4.80f + 1.8f, -25.5f), glm::vec3(3.0f, 3.6f, 0.25f), glm::vec3(1.0f, 1.0f, 1.0f), false);
}

float World::GetDarkStairsHeight(float x, float z) const
{
    if (x < -3.5f || x > 3.5f) return 0.0f;
    if (z > -4.0f || z < -26.5f) return 0.0f;
    if (z <= -20.0f) return 4.80f;

    float t = (-4.0f - z) / 16.0f;
    return std::clamp(t, 0.0f, 1.0f) * 4.80f;
}

float World::GetFloorHeight(float x, float z) const
{
    if (!inDarkStairs) return 0.0f;
    return GetDarkStairsHeight(x, z);
}

void World::BuildNextScene()
{
    Clear();
    inPalaceScene = true;

    floor = std::make_unique<Mesh>(Mesh::Plane(260.0f, 380.0f, 130.0f, glm::vec3(0.32f, 0.65f, 0.30f)));

    AddBox(glm::vec3(0.0f, 0.02f, 0.0f), glm::vec3(7.0f, 0.04f, 380.0f), glm::vec3(0.68f, 0.64f, 0.56f), false);

    colliders.push_back(AABB{ glm::vec3(-12.0f, 0.0f, -190.0f), glm::vec3(-3.5f, 10.0f, 190.0f) });
    colliders.push_back(AABB{ glm::vec3(3.5f, 0.0f, -190.0f), glm::vec3(12.0f, 10.0f, 190.0f) });
    colliders.push_back(AABB{ glm::vec3(-12.0f, 0.0f,  120.0f), glm::vec3(12.0f, 10.0f, 130.0f) });
    colliders.push_back(AABB{ glm::vec3(-12.0f, 0.0f, -188.0f), glm::vec3(12.0f, 10.0f,-180.0f) });

    const glm::vec3 flowerColors[] = {
        glm::vec3(0.95f, 0.25f, 0.30f),  
        glm::vec3(0.98f, 0.85f, 0.20f),  
        glm::vec3(0.95f, 0.50f, 0.80f),  
        glm::vec3(0.65f, 0.45f, 0.95f),  
        glm::vec3(0.95f, 0.95f, 0.98f)   
    };
    for (int c = 0; c < 5; ++c)
        flowerMeshes.push_back(Mesh::Box(glm::vec3(1.0f), flowerColors[c]));
    flowerMeshes.push_back(Mesh::Box(glm::vec3(1.0f), glm::vec3(0.18f, 0.48f, 0.18f)));     

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> distX(3.8f, 28.0f);
    std::uniform_real_distribution<float> distZ(-180.0f, 125.0f);
    std::uniform_real_distribution<float> scaleDist(0.24f, 0.46f);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> colDist(0, 4);

    for (int i = 0; i < 1000; ++i)
    {
        float x = distX(rng) * (sideDist(rng) == 0 ? -1.0f : 1.0f);
        float z = distZ(rng);
        float s = scaleDist(rng);
        int colIdx = colDist(rng);

        flowers.push_back(FlowerInstance{ glm::vec3(x, 0.12f, z), glm::vec3(0.08f, 0.24f, 0.08f), 5 });
        flowers.push_back(FlowerInstance{ glm::vec3(x, 0.27f, z), glm::vec3(s, 0.14f, s), colIdx });
    }

    const int kPaint = (int)flowerMeshes.size();
    flowerMeshes.push_back(Mesh::Box(glm::vec3(1.0f), glm::vec3(0.96f, 0.95f, 0.90f)));     
    const int kPost = (int)flowerMeshes.size();
    flowerMeshes.push_back(Mesh::Box(glm::vec3(1.0f), glm::vec3(0.52f, 0.54f, 0.60f)));     
    const int kCap = (int)flowerMeshes.size();
    flowerMeshes.push_back(Mesh::Box(glm::vec3(1.0f), glm::vec3(0.92f, 0.76f, 0.22f)));     

    for (float z = 124.0f; z > -186.0f; z -= 6.0f)
        flowers.push_back(FlowerInstance{ glm::vec3(0.0f, 0.05f, z), glm::vec3(0.28f, 0.02f, 2.6f), kPaint });

    for (float z = 120.0f; z > -186.0f; z -= 15.0f)
        for (float side : { -1.0f, 1.0f })
        {
            flowers.push_back(FlowerInstance{ glm::vec3(side * 3.75f, 0.55f, z), glm::vec3(0.32f, 1.10f, 0.32f), kPost });
            flowers.push_back(FlowerInstance{ glm::vec3(side * 3.75f, 1.16f, z), glm::vec3(0.44f, 0.14f, 0.44f), kCap });
        }

    const glm::vec3 marble(0.95f, 0.93f, 0.88f);
    const glm::vec3 gold(0.92f, 0.76f, 0.22f);
    const glm::vec3 roof(0.25f, 0.35f, 0.65f);
    const glm::vec3 gate(0.15f, 0.12f, 0.18f);

    auto addPalacePart = [&](glm::vec3 offset, glm::vec3 size, glm::vec3 color)
        {
            palaceProps.push_back(Prop{ Mesh::Box(size, color), offset });
        };

    addPalacePart(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(36.0f, 12.0f, 1.5f), marble);
    addPalacePart(glm::vec3(0.0f, 2.8f, 0.9f), glm::vec3(4.2f, 5.6f, 0.4f), gate);
    for (float cx : { -14.0f, -9.0f, -4.5f, 4.5f, 9.0f, 14.0f })
        addPalacePart(glm::vec3(cx, 6.0f, 0.9f), glm::vec3(1.1f, 12.0f, 0.6f), gold);

    addPalacePart(glm::vec3(0.0f, 12.6f, 0.0f), glm::vec3(38.0f, 1.2f, 2.0f), gold);
    addPalacePart(glm::vec3(0.0f, 16.0f, 0.0f), glm::vec3(10.0f, 6.0f, 1.6f), marble);
    addPalacePart(glm::vec3(0.0f, 20.5f, 0.0f), glm::vec3(4.0f, 3.5f, 1.6f), roof);
    addPalacePart(glm::vec3(0.0f, 23.5f, 0.0f), glm::vec3(0.8f, 3.0f, 0.8f), gold);

    addPalacePart(glm::vec3(-15.5f, 15.5f, 0.0f), glm::vec3(4.5f, 5.0f, 1.6f), marble);
    addPalacePart(glm::vec3(-15.5f, 19.2f, 0.0f), glm::vec3(5.2f, 2.5f, 1.8f), roof);
    addPalacePart(glm::vec3(15.5f, 15.5f, 0.0f), glm::vec3(4.5f, 5.0f, 1.6f), marble);
    addPalacePart(glm::vec3(15.5f, 19.2f, 0.0f), glm::vec3(5.2f, 2.5f, 1.8f), roof);

    palaceZ = 30.0f;
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

void World::AddLamp(const glm::vec3& b, const glm::vec2& target)
{
    glm::vec2 dir(target.x - b.x, target.y - b.z);
    float len = glm::length(dir);
    dir = (len > 1e-4f) ? dir / len : glm::vec2(1.0f, 0.0f);
    const float yaw = glm::degrees(std::atan2(-dir.y, dir.x));         

    const glm::vec3 metal(0.36f, 0.36f, 0.40f);
    const glm::vec3 metalDark(0.24f, 0.24f, 0.28f);
    const glm::vec3 bulb(1.45f, 1.00f, 0.42f);                

    const float poleTop = 5.1f;     
    const float armLen = 1.5f;      

    AddBox(b + glm::vec3(0.0f, 0.25f, 0.0f), glm::vec3(0.6f, 0.5f, 0.6f), metalDark);
    AddBox(b + glm::vec3(0.0f, (0.5f + poleTop) * 0.5f, 0.0f), glm::vec3(0.2f, poleTop - 0.5f, 0.2f), metal);
    AddBox(b + glm::vec3(0.0f, 1.5f, 0.0f), glm::vec3(0.34f, 0.12f, 0.34f), metalDark, false);
    AddBox(b + glm::vec3(0.0f, 3.4f, 0.0f), glm::vec3(0.30f, 0.10f, 0.30f), metalDark, false);

    auto part = [&](float along, float height, const glm::vec3& size, const glm::vec3& color)
    {
        glm::vec3 pos = b + glm::vec3(along * dir.x, height, along * dir.y);
        props.push_back(Prop{ Mesh::Box(size, color), pos, glm::vec3(0.0f, yaw, 0.0f) });
    };
    part(armLen * 0.5f, poleTop - 0.10f, glm::vec3(armLen + 0.2f, 0.14f, 0.14f), metal);    
    part(armLen,        poleTop - 0.24f, glm::vec3(1.00f, 0.16f, 0.56f), metalDark);         
    part(armLen,        poleTop - 0.35f, glm::vec3(0.90f, 0.08f, 0.45f), bulb);              

    lampLights.push_back(b + glm::vec3(dir.x * armLen, poleTop - 0.45f, dir.y * armLen));
}

void World::AddLampTrail(const glm::vec2& from, const glm::vec2& to, float spacing, float sideOffset)
{
    glm::vec2 d = to - from;
    float len = glm::length(d);
    if (len < 1e-3f) return;

    glm::vec2 dir = d / len;
    glm::vec2 perp(-dir.y, dir.x);
    int count = std::max(1, (int)std::floor(len / spacing));
    float step = len / count;                             

    for (int i = 0; i <= count; ++i)
    {
        glm::vec2 onPath = from + dir * (step * i);                     
        float side = (i % 2 == 0) ? 1.0f : -1.0f;                        
        glm::vec2 p = onPath + perp * (side * sideOffset);
        AddLamp(glm::vec3(p.x, 0.0f, p.y), onPath);                     
    }
}

void World::UploadLights(Shader& shader, float strength) const
{
    int n = (int)std::min<size_t>(lampLights.size(), 24);
    glUniform1i(glGetUniformLocation(shader.ID, "lampCount"), n);
    glUniform1f(glGetUniformLocation(shader.ID, "lampStrength"), strength);
    if (n > 0)
        glUniform3fv(glGetUniformLocation(shader.ID, "lampPos"), n, &lampLights[0].x);
}

void World::SetPalaceAnchor(float z)
{
    palaceAnchored = true;
    palaceAnchorZ = z;
    palaceZ = z;
    colliders.push_back(AABB{ glm::vec3(-19.0f, 0.0f, z - 1.2f), glm::vec3(19.0f, 14.0f, z + 1.3f) });
}

bool World::PalaceDoorNear(const glm::vec3& p) const
{
    if (!inPalaceScene || !palaceAnchored) return false;
    return std::abs(p.x) < 6.0f && p.z < palaceAnchorZ + 8.0f && p.z > palaceAnchorZ;
}

void World::Update(float dt, const glm::vec3& playerPos)
{
    for (auto& n : npcs) n.Update(dt, playerPos);
    viewPos = playerPos;

    if (inPalaceScene)
        palaceZ = palaceAnchored ? std::max(playerPos.z - 80.0f, palaceAnchorZ) : playerPos.z - 80.0f;
}

void World::Draw(Shader& shader)
{
    if (floor) floor->Draw(shader, glm::mat4(1.0f));
    if (map) map->Draw(shader, glm::mat4(1.0f));

    for (auto& p : props)
    {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), p.position);
        if (p.rotation.x != 0.0f) m = glm::rotate(m, glm::radians(p.rotation.x), glm::vec3(1, 0, 0));
        if (p.rotation.y != 0.0f) m = glm::rotate(m, glm::radians(p.rotation.y), glm::vec3(0, 1, 0));
        if (p.rotation.z != 0.0f) m = glm::rotate(m, glm::radians(p.rotation.z), glm::vec3(0, 0, 1));
        p.mesh.Draw(shader, m);
    }

    const float kFarDraw = 130.0f;
    for (const FlowerInstance& f : flowers)
    {
        if (inPalaceScene && std::abs(f.pos.z - viewPos.z) > kFarDraw) continue;

        glm::mat4 m = glm::translate(glm::mat4(1.0f), f.pos);
        m = glm::scale(m, f.scale);
        flowerMeshes[f.colorIndex].Draw(shader, m);
    }

    for (auto& p : palaceProps)
        p.mesh.Draw(shader, glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, palaceZ) + p.position));

    for (auto& n : npcs) n.Draw(shader);
}

void World::Delete()
{
    if (floor) floor->Delete(true);
    if (map) map->Delete();
    for (auto& p : props) p.mesh.Delete();
    for (auto& p : palaceProps) p.mesh.Delete();
    for (auto& fm : flowerMeshes) fm.Delete();
    for (auto& n : npcs) n.Delete();
}

bool World::Collides(const glm::vec3& feet, float radius, float height) const
{
    AABB player{ glm::vec3(feet.x - radius, feet.y + 0.05f, feet.z - radius),
                 glm::vec3(feet.x + radius, feet.y + height, feet.z + radius) };

    for (const AABB& c : colliders)
        if (AABBOverlap(player, c)) return true;

    for (const NPC& n : npcs)
        if (n.active && AABBOverlap(player, n.Bounds())) return true;

    return false;
}

NPC* World::FindInteractable(const glm::vec3& eyePos, const glm::vec3& lookDir, float maxDist)
{
    NPC* best = nullptr;
    float bestDot = 0.85f;

    glm::vec3 look = glm::normalize(lookDir);
    for (NPC& n : npcs)
    {
        if (!n.active) continue;
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