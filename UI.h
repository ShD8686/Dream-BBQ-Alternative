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

// Реплика авторов на чёрном экране: набор кусков текста с разным стилем
enum class SpanStyle { Normal, Freedom /*красный, скошенный*/, True /*зелёный, с глитчами*/ };
struct StorySpan
{
    std::string text;
    SpanStyle style = SpanStyle::Normal;
};
struct StoryLine
{
    std::vector<StorySpan> spans;
    float hold = 1.8f;   // сколько секунд держать строку после того, как она допечаталась
    float gap = 3.5f;    // чёрная пауза после того, как строка исчезла, до появления следующей
    bool achievement = false; // когда строка допечатана, выезжает плашка достижения
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
    void DrawVignette(float intensity, float r, float g, float b); // цветная рамка по краям экрана (0..1)
    // happy = false: ":(" и текст по центру; happy = true: ":)" и крупная подпись внизу экрана
    void DrawBSOD(const std::string& text, bool happy = false);

    // Сколько секунд печатается строка (без паузы-удержания)
    static float StoryTypeDuration(const StoryLine& line);
    // t - секунды с начала строки, fade - общая прозрачность 0..1 (для появления/угасания)
    void DrawStoryLine(const StoryLine& line, float t, float fade);
    // Плашка достижения справа внизу; t - секунды с момента появления (сама выезжает и уезжает)
    void DrawAchievement(const std::string& title, float t);

    void InitPhone(GLFWwindow* phoneWindow);
    void DrawPhoneWindow(GLFWwindow* phoneWindow, float dt, const std::vector<PhoneQuest>& quests);
    void ShutdownPhone(GLFWwindow* phoneWindow);

    void DrawCrosshair();
    void DrawPrompt(const std::string& text);
    void DrawDialogue(const std::string& speaker, const std::string& text, int lineIndex, int lineCount);
    void DrawFPS(float fps);
    // Нижняя левая плашка: название проекта, команда и состояние переключателей (требование курса)
    void DrawHud(const std::string& line1, const std::string& line2);
    // Окно помощи по клавишам (F1 или H)
    void DrawHelp();

private:
    ImGuiContext* mainCtx = nullptr;
    ImGuiContext* phoneCtx = nullptr;
};