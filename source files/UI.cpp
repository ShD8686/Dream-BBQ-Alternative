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

void UI::Init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    // Шрифт с кириллицей. Лучше положить свой в Resources/fonts/font.ttf
    const char* candidates[] = {
        "Resources/fonts/font.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf"
    };
    for (const char* c : candidates)
    {
        if (FileExists(c))
        {
            io.Fonts->AddFontFromFileTTF(c, 22.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
            break;
        }
    }

    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 8.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void UI::BeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::EndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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
