#pragma once
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Интерфейс на Dear ImGui: прицел, подсказка "Нажмите E", окно диалога.
class UI
{
public:
    void Init(GLFWwindow* window);
    void BeginFrame();
    void EndFrame();
    void Shutdown();

    void DrawCrosshair();
    void DrawPrompt(const std::string& text);
    void DrawDialogue(const std::string& speaker, const std::string& text, int lineIndex, int lineCount);
    void DrawFPS(float fps);
};
