#pragma once
#include <glad/glad.h>
#include <string>

class Texture
{
public:
    GLuint ID = 0;
    int width = 0, height = 0;

    Texture() = default;

    // pixelated = true -> чёткие пиксели (стиль PS1 / пиксель-арт)
    bool Load(const std::string& path, bool pixelated = false);

    void Bind(GLuint unit = 0) const;
    void Unbind() const;
    void Delete();
};
