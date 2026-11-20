#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Mesh.h"
#include "AABB.h"

class Model
{
public:
    std::vector<Mesh> meshes;
    std::vector<AABB> colliders;

    bool Load(const std::string& path, float scale = 1.0f, bool pixelated = false);
    void Draw(Shader& shader, const glm::mat4& model = glm::mat4(1.0f));
    void Delete();

private:
    std::unordered_map<std::string, Texture> textureCache;
    bool LoadTexture(const std::string& path, bool pixelated, Texture& out);
};
