#include "UI.h"
#include <fstream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

static bool FileExists(const char* path)
{
    std::ifstream f(path);
    return f.good();
}

static void LoadCyrillicFont(ImGuiIO& io, float size)
{
    const char* candidates[] = {
        "Resources/fonts/font.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };
    for (const char* c : candidates)
    {
        if (FileExists(c))
        {
            io.Fonts->AddFontFromFileTTF(c, size, nullptr, io.Fonts->GetGlyphRangesCyrillic());
            break;
        }
    }
}

void UI::DrawTimer(float secondsLeft)
{
    if (secondsLeft < 0.0f) secondsLeft = 0.0f;
    ImVec2 s = ImGui::GetIO().DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, 16.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGui::Begin("##timer", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);

    ImGui::SetWindowFontScale(1.8f);
    ImVec4 col = (secondsLeft <= 10.0f)
        ? ImVec4(1.0f, 0.18f, 0.18f, 1.0f)   // Red on last 10 seconds
        : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);    // White otherwise

    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.1f", secondsLeft);
    ImGui::TextColored(col, "%s", buf);
    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

void UI::DrawFade(float r, float g, float b, float alpha)
{
    if (alpha <= 0.001f) return;
    if (alpha > 1.0f) alpha = 1.0f;

    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImU32 col = IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(alpha * 255.0f));
    ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0.0f, 0.0f), s, col);
}

void UI::DrawBSOD(const std::string& text)
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(s);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("##bsod", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);

    ImGui::SetCursorPos(ImVec2(s.x * 0.12f, s.y * 0.22f));
    ImGui::SetWindowFontScale(4.5f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ":(");

    ImGui::SetCursorPos(ImVec2(s.x * 0.12f, s.y * 0.42f));
    ImGui::SetWindowFontScale(3.5f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", text.c_str());

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

void UI::Init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    mainCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext(mainCtx);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    LoadCyrillicFont(io, 22.0f);

    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 8.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::BeginFrame()
{
    ImGui::SetCurrentContext(mainCtx);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::EndFrame()
{
    ImGui::SetCurrentContext(mainCtx);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Shutdown()
{
    ImGui::SetCurrentContext(mainCtx);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(mainCtx);
    mainCtx = nullptr;
}

void UI::InitPhone(GLFWwindow* phoneWindow)
{
    if (!phoneWindow) return;

    GLFWwindow* prevWin = glfwGetCurrentContext();
    glfwMakeContextCurrent(phoneWindow);

    phoneCtx = ImGui::CreateContext();
    ImGui::SetCurrentContext(phoneCtx);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    LoadCyrillicFont(io, 20.0f);

    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");

    ImGui::SetCurrentContext(mainCtx);
    glfwMakeContextCurrent(prevWin);
}

void UI::DrawPhoneWindow(GLFWwindow* phoneWindow, float dt, const std::vector<PhoneQuest>& quests)
{
    if (!phoneWindow || !phoneCtx) return;

    GLFWwindow* prevWin = glfwGetCurrentContext();
    glfwMakeContextCurrent(phoneWindow);
    ImGui::SetCurrentContext(phoneCtx);

    int w = 320, h = 560, fbW = 320, fbH = 560;
    glfwGetWindowSize(phoneWindow, &w, &h);
    glfwGetFramebufferSize(phoneWindow, &fbW, &fbH);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? (float)fbW / w : 1.0f, h > 0 ? (float)fbH / h : 1.0f);
    io.DeltaTime = (dt > 0.0001f) ? dt : (1.0f / 60.0f);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    // Outer phone frame inside the 320x560 OS window
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
    ImGui::SetNextWindowSize(ImVec2((float)w - 20.0f, (float)h - 20.0f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18.0f, 16.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.13f, 0.17f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.33f, 0.40f, 1.0f));

    ImGui::Begin("##phone_screen", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 winSize = ImGui::GetWindowSize();

    // Top speaker notch
    float notchW = 84.0f;
    dl->AddRectFilled(
        ImVec2(winPos.x + (winSize.x - notchW) * 0.5f, winPos.y + 8.0f),
        ImVec2(winPos.x + (winSize.x + notchW) * 0.5f, winPos.y + 18.0f),
        IM_COL32(20, 22, 28, 255), 5.0f
    );

    // Status bar
    ImGui::SetCursorPosY(24.0f);
    ImGui::TextDisabled("12:45");
    ImGui::SameLine(winSize.x - 95.0f);
    ImGui::TextDisabled("5G  100%%");
    ImGui::Separator();
    ImGui::Spacing();

    // App title
    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), "QUESTS");
    ImGui::Spacing();

    if (quests.empty())
    {
        ImGui::TextDisabled("No tasks yet.\nTalk to NPCs!");
    }
    else
    {
        for (const PhoneQuest& q : quests)
        {
            if (q.completed)
                ImGui::TextColored(ImVec4(0.45f, 0.85f, 0.50f, 1.0f), "[x]");
            else
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.25f, 1.0f), "[ ]");

            ImGui::SameLine();
            if (q.completed)
                ImGui::TextWrapped("%s", q.text.c_str());
            else
                ImGui::TextWrapped("%s", q.text.c_str());

            ImGui::Spacing();
        }
    }

    // Bottom hint + home bar
    ImGui::SetCursorPosY(winSize.y - 44.0f);
    ImGui::TextDisabled("[TAB] Hide phone");

    float barW = 100.0f;
    dl->AddRectFilled(
        ImVec2(winPos.x + (winSize.x - barW) * 0.5f, winPos.y + winSize.y - 14.0f),
        ImVec2(winPos.x + (winSize.x + barW) * 0.5f, winPos.y + winSize.y - 9.0f),
        IM_COL32(190, 195, 210, 200), 3.0f
    );

    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    ImGui::Render();
    glViewport(0, 0, fbW, fbH);
    glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(phoneWindow);

    ImGui::SetCurrentContext(mainCtx);
    glfwMakeContextCurrent(prevWin);
}

void UI::ShutdownPhone(GLFWwindow* phoneWindow)
{
    if (!phoneWindow || !phoneCtx) return;

    GLFWwindow* prevWin = glfwGetCurrentContext();
    glfwMakeContextCurrent(phoneWindow);
    ImGui::SetCurrentContext(phoneCtx);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext(phoneCtx);
    phoneCtx = nullptr;

    ImGui::SetCurrentContext(mainCtx);
    glfwMakeContextCurrent(prevWin);
}

void UI::DrawCrosshair()
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2(s.x * 0.5f, s.y * 0.5f), 3.0f, IM_COL32(255, 255, 255, 220));
}

void UI::DrawPrompt(const std::string& text)
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.5f + 50.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.65f);
    ImGui::Begin("##prompt", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav);
    ImGui::TextUnformatted(text.c_str());
    ImGui::End();
}

void UI::DrawDialogue(const std::string& speaker, const std::string& text, int lineIndex, int lineCount)
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y - 30.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowSize(ImVec2(s.x * 0.7f, 180.0f));
    ImGui::SetNextWindowBgAlpha(0.85f);
    ImGui::Begin("##dialogue", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "%s", speaker.c_str());
    ImGui::Separator();
    ImGui::TextWrapped("%s", text.c_str());

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 34.0f);
    const char* hint = (lineIndex + 1 >= lineCount) ? "[E] Закончить" : "[E] Далее";
    ImGui::TextDisabled("%s  (%d/%d)   [Esc] Выйти", hint, lineIndex + 1, lineCount);
    ImGui::End();
}

void UI::DrawFPS(float fps)
{
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.3f);
    ImGui::Begin("##fps", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);
    ImGui::Text("FPS: %.0f", fps);
    ImGui::End();
}