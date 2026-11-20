#include "Mesh.h"
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

Mesh::Mesh(std::vector<float> v, std::vector<GLuint> i, glm::vec3 c)
    : vertices(std::move(v)),
      indices(std::move(i)),
      VAO1(),
      VBO1(vertices.data(), (GLsizeiptr)(vertices.size() * sizeof(float))),
      EBO1(indices.data(), (GLsizeiptr)(indices.size() * sizeof(GLuint))),
      color(c)
{
    VAO1.Bind();
    VBO1.Bind();
    EBO1.Bind();

    const GLsizeiptr stride = 8 * sizeof(float);
    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, stride, (void*)0);                     
    VAO1.LinkAttrib(VBO1, 1, 3, GL_FLOAT, stride, (void*)(3 * sizeof(float)));   
    VAO1.LinkAttrib(VBO1, 2, 2, GL_FLOAT, stride, (void*)(6 * sizeof(float)));   

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();
}

void Mesh::SetTexture(const Texture& texture)
{
    tex = texture;
    hasTexture = true;
}

void Mesh::Draw(Shader& shader, const glm::mat4& model)
{
    shader.Activate();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform3f(glGetUniformLocation(shader.ID, "baseColor"), color.r, color.g, color.b);
    glUniform1i(glGetUniformLocation(shader.ID, "useTexture"), hasTexture ? 1 : 0);

    if (hasTexture)
    {
        tex.Bind(0);
        glUniform1i(glGetUniformLocation(shader.ID, "tex0"), 0);
    }

    VAO1.Bind();
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
}

void Mesh::Delete(bool deleteTexture)
{
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    if (deleteTexture) tex.Delete();
}

Mesh Mesh::Box(const glm::vec3& size, const glm::vec3& color)
{
    const glm::vec3 h = size * 0.5f;

    const glm::vec3 N[6] = { glm::vec3(1,0,0), glm::vec3(-1,0,0), glm::vec3(0,1,0), glm::vec3(0,-1,0), glm::vec3(0,0,1), glm::vec3(0,0,-1) };
    const glm::vec3 U[6] = { glm::vec3(0,1,0), glm::vec3(0,0,1),  glm::vec3(0,0,1), glm::vec3(1,0,0),  glm::vec3(1,0,0), glm::vec3(0,1,0) };
    const glm::vec3 V[6] = { glm::vec3(0,0,1), glm::vec3(0,1,0),  glm::vec3(1,0,0), glm::vec3(0,0,1),  glm::vec3(0,1,0), glm::vec3(1,0,0) };

    const float su[4] = { -1, 1, 1, -1 };
    const float sv[4] = { -1, -1, 1, 1 };
    const float uu[4] = { 0, 1, 1, 0 };
    const float vv[4] = { 0, 0, 1, 1 };

    std::vector<float> verts;
    std::vector<GLuint> idx;

    for (int f = 0; f < 6; f++)
    {
        glm::vec3 c = N[f] * h;
        glm::vec3 eu = U[f] * h;
        glm::vec3 ev = V[f] * h;
        GLuint base = (GLuint)(verts.size() / 8);

        for (int k = 0; k < 4; k++)
        {
            glm::vec3 p = c + eu * su[k] + ev * sv[k];
            verts.insert(verts.end(), { p.x, p.y, p.z, N[f].x, N[f].y, N[f].z, uu[k], vv[k] });
        }
        idx.insert(idx.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
    }
    return Mesh(verts, idx, color);
}

Mesh Mesh::Disc(float radius, int segments, const glm::vec3& color)
{
    const float twoPi = 6.2831853f;
    std::vector<float> verts = { 0, 0, 0,  0, 1, 0,  0.5f, 0.5f };    
    std::vector<GLuint> idx;

    for (int i = 0; i <= segments; ++i)
    {
        float a = twoPi * (float)i / (float)segments;
        float c = std::cos(a), s = std::sin(a);
        verts.insert(verts.end(), { radius * c, 0.0f, radius * s,  0, 1, 0,  0.5f + 0.5f * c, 0.5f + 0.5f * s });
    }
    for (int i = 1; i <= segments; ++i)
        idx.insert(idx.end(), { 0u, (GLuint)(i + 1), (GLuint)i });

    return Mesh(verts, idx, color);
}

Mesh Mesh::Plane(float sizeX, float sizeZ, float t, const glm::vec3& color)
{
    float x = sizeX * 0.5f, z = sizeZ * 0.5f;
    std::vector<float> verts = {
        -x, 0, -z,  0, 1, 0,  0, 0,
        -x, 0,  z,  0, 1, 0,  0, t,
         x, 0,  z,  0, 1, 0,  t, t,
         x, 0, -z,  0, 1, 0,  t, 0
    };
    std::vector<GLuint> idx = { 0, 1, 2, 0, 2, 3 };
    return Mesh(verts, idx, color);
}
