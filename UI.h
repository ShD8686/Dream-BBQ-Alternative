#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct ImGuiContext;

struct PhoneQuest
{
    std::string text;
    bool completed = false;
};

// Интерфейс на Dear ImGui: прицел, подсказка "Нажмите E", окно диалога, окно телефона.
class UI
{
public:
    void DrawFade(float r, float g, float b, float alpha);
    void Init(GLFWwindow* window);
    void BeginFrame();
    void EndFrame();
    void Shutdown();
    void DrawTimer(float secondsLeft);
    void DrawBSOD(const std::string& text);

    void InitPhone(GLFWwindow* phoneWindow);
    void DrawPhoneWindow(GLFWwindow* phoneWindow, float dt, const std::vector<PhoneQuest>& quests);
    void ShutdownPhone(GLFWwindow* phoneWindow);

    void DrawCrosshair();
    void DrawPrompt(const std::string& text);
    void DrawDialogue(const std::string& speaker, const std::string& text, int lineIndex, int lineCount);
    void DrawFPS(float fps);

private:
    ImGuiContext* mainCtx = nullptr;
    ImGuiContext* phoneCtx = nullptr;
};