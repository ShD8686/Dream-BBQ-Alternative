#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "shaderClass.h"
#include "Texture.h"

class Mesh
{
public:
    Mesh(std::vector<float> vertices, std::vector<GLuint> indices, glm::vec3 color = glm::vec3(0.8f));

    void SetTexture(const Texture& texture);
    void Draw(Shader& shader, const glm::mat4& model);
    void Delete(bool deleteTexture = false);

    static Mesh Box(const glm::vec3& size, const glm::vec3& color);                            
    static Mesh Plane(float sizeX, float sizeZ, float uvTiles, const glm::vec3& color);          
    static Mesh Disc(float radius, int segments, const glm::vec3& color);                           

private:
    std::vector<float> vertices;
    std::vector<GLuint> indices;
    VAO VAO1;
    VBO VBO1;
    EBO EBO1;
    Texture tex;
    bool hasTexture = false;
    glm::vec3 color;
};
