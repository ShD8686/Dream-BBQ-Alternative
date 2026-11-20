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

enum class SpanStyle { Normal, Freedom  , True    };
struct StorySpan
{
    std::string text;
    SpanStyle style = SpanStyle::Normal;
};
struct StoryLine
{
    std::vector<StorySpan> spans;
    float hold = 1.8f;            
    float gap = 3.5f;              
    bool achievement = false;       
};

class UI
{
public:
    void DrawFade(float r, float g, float b, float alpha);
    void Init(GLFWwindow* window);
    void BeginFrame();
    void EndFrame();
    void Shutdown();
    void DrawTimer(float secondsLeft);
    void DrawVignette(float intensity, float r, float g, float b);       
    void DrawBSOD(const std::string& text, bool happy = false);

    static float StoryTypeDuration(const StoryLine& line);
    void DrawStoryLine(const StoryLine& line, float t, float fade);
    void DrawAchievement(const std::string& title, float t);

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