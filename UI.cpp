#include "UI.h"
#include <fstream>
#include <imgui.h>
#include <cmath>
#include <algorithm>
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

void UI::DrawVignette(float intensity, float r, float g, float b)
{
    if (intensity <= 0.001f) return;
    if (intensity > 1.0f) intensity = 1.0f;

    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImDrawList* dl = ImGui::GetBackgroundDrawList(); // под таймером и текстом, но поверх 3D-сцены

    const int R = (int)(r * 255.0f), G = (int)(g * 255.0f), B = (int)(b * 255.0f);
    const ImU32 edge = IM_COL32(R, G, B, (int)(intensity * 190.0f));
    const ImU32 none = IM_COL32(R, G, B, 0);
    const float bx = s.x * 0.28f, by = s.y * 0.30f;

    dl->AddRectFilledMultiColor(ImVec2(0.0f, 0.0f), ImVec2(s.x, by), edge, edge, none, none); // сверху
    dl->AddRectFilledMultiColor(ImVec2(0.0f, s.y - by), ImVec2(s.x, s.y), none, none, edge, edge); // снизу
    dl->AddRectFilledMultiColor(ImVec2(0.0f, 0.0f), ImVec2(bx, s.y), edge, none, none, edge); // слева
    dl->AddRectFilledMultiColor(ImVec2(s.x - bx, 0.0f), ImVec2(s.x, s.y), none, edge, edge, none); // справа
}

void UI::DrawFade(float r, float g, float b, float alpha)
{
    if (alpha <= 0.001f) return;
    if (alpha > 1.0f) alpha = 1.0f;

    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImU32 col = IM_COL32((int)(r * 255.0f), (int)(g * 255.0f), (int)(b * 255.0f), (int)(alpha * 255.0f));
    ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(0.0f, 0.0f), s, col);
}

void UI::DrawBSOD(const std::string& text, bool happy)
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(s);
    ImGui::SetNextWindowBgAlpha(0.0f);

    ImGui::Begin("##bsod", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);

    ImGui::SetCursorPos(ImVec2(s.x * 0.12f, s.y * 0.22f));
    ImGui::SetWindowFontScale(happy ? 9.0f : 4.5f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), happy ? ":)" : ":(");

    ImGui::SetCursorPos(happy ? ImVec2(s.x * 0.12f, s.y * 0.80f) : ImVec2(s.x * 0.12f, s.y * 0.42f));
    ImGui::SetWindowFontScale(3.5f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", text.c_str());

    ImGui::SetWindowFontScale(1.0f);
    ImGui::End();
}

// ---------- Реплики авторов ----------

// Время печати одного символа: после конца фразы ("." перед пробелом/концом) и запятой небольшая пауза
static float CharCost(const std::string& s, size_t i)
{
    char c = s[i];
    char nx = (i + 1 < s.size()) ? s[i + 1] : ' ';
    float t = 0.095f;                                   // медленная печать
    if (c == '.' && nx != '.') t += 0.60f;
    else if (c == '.') t += 0.28f;                      // между точками многоточия
    else if (c == ',') t += 0.30f;
    else if (c == '!' || c == '?') t += 0.50f;
    return t;
}

static std::string JoinSpans(const StoryLine& line)
{
    std::string all;
    for (const StorySpan& sp : line.spans) all += sp.text;
    return all;
}

float UI::StoryTypeDuration(const StoryLine& line)
{
    std::string all = JoinSpans(line);
    float t = 0.0f;
    for (size_t i = 0; i < all.size(); ++i) t += CharCost(all, i);
    return t;
}

static float Hash01(float x)
{
    float v = std::sin(x * 127.1f) * 43758.5453f;
    return v - std::floor(v);
}

// Сдвиг уже добавленных вершин (от индекса v0): нужен для наклона и "срезов" глитча
template <class F>
static void ShiftVerts(ImDrawList* dl, int v0, F fn)
{
    for (int i = v0; i < dl->VtxBuffer.Size; ++i)
        fn(dl->VtxBuffer[i].pos);
}

void UI::DrawStoryLine(const StoryLine& line, float t, float fade)
{
    if (fade <= 0.001f) return;
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImFont* font = ImGui::GetFont();
    const float time = (float)ImGui::GetTime();

    // Сколько символов уже напечатано
    std::string all = JoinSpans(line);
    int shown = 0;
    {
        float acc = 0.0f;
        for (size_t i = 0; i < all.size(); ++i)
        {
            acc += CharCost(all, i);
            if (acc <= t) shown = (int)i + 1; else break;
        }
    }

    // Размер шрифта: крупный, но строка обязана влезть в 90% ширины окна
    float fs = std::max(26.0f, s.y * 0.065f);
    auto totalWidth = [&](float size)
        {
            float w = 0.0f;
            for (const StorySpan& sp : line.spans)
                w += font->CalcTextSizeA(size, FLT_MAX, 0.0f, sp.text.c_str()).x;
            return w;
        };
    float w = totalWidth(fs);
    if (w > s.x * 0.9f) { fs *= s.x * 0.9f / w; w = totalWidth(fs); }

    float x = (s.x - w) * 0.5f;
    const float y = s.y * 0.5f - fs * 0.5f;
    int consumed = 0;

    for (const StorySpan& sp : line.spans)
    {
        float spW = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, sp.text.c_str()).x;
        int visible = std::max(0, std::min((int)sp.text.size(), shown - consumed));
        consumed += (int)sp.text.size();
        if (visible > 0)
        {
            std::string part = sp.text.substr(0, visible);
            const int a255 = (int)(fade * 255.0f);
            ImVec2 pos(x, y);

            if (sp.style == SpanStyle::Normal)
            {
                dl->AddText(font, fs, pos, IM_COL32(225, 225, 225, a255), part.c_str());
            }
            else
            {
                // FREEDOM (красный, наклонённый) и TRUE (зелёный): оба с глитчами, наклон только у красного
                const bool red = (sp.style == SpanStyle::Freedom);
                const float slope = red ? 0.28f : 0.0f;
                const float cy = pos.y + fs;
                const float slot = std::floor(time * (red ? 11.0f : 9.0f));
                const bool burst = Hash01(slot + (red ? 17.0f : 0.0f)) > (red ? 0.66f : 0.72f);
                const float flick = red ? 0.80f + 0.20f * Hash01(std::floor(time * 14.0f)) : 1.0f;

                std::string txt = part;
                if (burst && !txt.empty())
                {
                    const char* junk = "01#%@$&";
                    size_t idx = (size_t)(Hash01(slot + 3.0f) * txt.size()) % txt.size();
                    txt[idx] = junk[(int)(Hash01(slot + 5.0f) * 7.0f) % 7];
                }
                // один вызов: текст со сдвигом верхней части (срез глитча) и наклоном
                auto put = [&](ImVec2 p, ImU32 col, float cut, float dx)
                    {
                        int v0 = dl->VtxBuffer.Size;
                        dl->AddText(font, fs, p, col, txt.c_str());
                        ShiftVerts(dl, v0, [&](ImVec2& q)
                            {
                                if (cut > 0.0f && q.y < cut) q.x += dx;
                                q.x += (cy - q.y) * slope;
                            });
                    };

                const ImU32 glowCol = red ? IM_COL32(120, 0, 0, (int)(a255 * 0.30f)) : IM_COL32(20, 255, 80, (int)(a255 * 0.16f));
                const ImU32 mainCol = red ? IM_COL32((int)(235 * flick), (int)(14 * flick), (int)(14 * flick), a255)
                    : IM_COL32(40, 255, 90, a255);
                for (int k = 0; k < 4; ++k)
                {
                    float ang = k * 1.5708f + 0.7f;
                    put(ImVec2(pos.x + std::cos(ang) * 3.0f, pos.y + std::sin(ang) * 3.0f), glowCol, -1.0f, 0.0f);
                }
                if (burst)
                {
                    float dx = (Hash01(slot + 1.0f) - 0.5f) * 10.0f;
                    float cut = pos.y + fs * (0.30f + 0.4f * Hash01(slot + 2.0f));
                    put(ImVec2(pos.x + 4.0f, pos.y), IM_COL32(255, 30, 60, (int)(a255 * 0.55f)), -1.0f, 0.0f);
                    put(ImVec2(pos.x - 4.0f, pos.y + 1.0f), IM_COL32(40, 220, 255, (int)(a255 * 0.55f)), -1.0f, 0.0f);
                    put(pos, mainCol, cut, dx);
                }
                else
                {
                    put(pos, mainCol, -1.0f, 0.0f);
                }
            }
        }
        x += spW;
    }
}

void UI::DrawAchievement(const std::string& title, float t)
{
    // Выезд 0.5 с, показ, уезд после 5 с
    const float inT = 0.5f, stay = 5.0f, outT = 0.5f;
    if (t < 0.0f || t > inT + stay + outT) return;
    float slide = 1.0f;
    if (t < inT) { float k = t / inT; slide = 1.0f - (1.0f - k) * (1.0f - k); }
    else if (t > inT + stay) { float k = (t - inT - stay) / outT; slide = 1.0f - k * k; }

    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImFont* font = ImGui::GetFont();

    const float w = 330.0f, h = 76.0f, margin = 24.0f;
    float x = s.x - (w + margin) * slide;
    float y = s.y - h - margin;
    ImVec2 a(x, y), b(x + w, y + h);

    dl->AddRectFilled(a, b, IM_COL32(28, 28, 32, 235), 8.0f);
    dl->AddRect(a, b, IM_COL32(90, 90, 100, 255), 8.0f, 0, 1.5f);

    // Золотая "медаль" с вопросительным знаком
    ImVec2 c(x + 40.0f, y + h * 0.5f);
    dl->AddCircleFilled(c, 24.0f, IM_COL32(214, 170, 50, 255), 32);
    dl->AddCircle(c, 24.0f, IM_COL32(255, 225, 120, 255), 32, 2.0f);
    ImVec2 q = font->CalcTextSizeA(30.0f, FLT_MAX, 0.0f, "?");
    dl->AddText(font, 30.0f, ImVec2(c.x - q.x * 0.5f, c.y - q.y * 0.5f), IM_COL32(60, 40, 5, 255), "?");

    dl->AddText(font, 15.0f, ImVec2(x + 80.0f, y + 12.0f), IM_COL32(160, 160, 170, 255), "Achievement unlocked");
    dl->AddText(font, 26.0f, ImVec2(x + 80.0f, y + 34.0f), IM_COL32(245, 245, 245, 255), title.c_str());
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

void UI::DrawHud(const std::string& line1, const std::string& line2)
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(10.0f, s.y - 10.0f), ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.35f);
    ImGui::Begin("##hud", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);
    ImGui::TextUnformatted(line1.c_str());
    ImGui::TextUnformatted(line2.c_str());
    ImGui::End();
}

void UI::DrawHelp()
{
    ImVec2 s = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(s.x * 0.5f, s.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.88f);
    ImGui::Begin("##help", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);
    ImGui::Text("CONTROLS  (game is paused while this window is open)");
    ImGui::Separator();
    ImGui::Text("W A S D      move");
    ImGui::Text("Shift        run");
    ImGui::Text("Space        jump");
    ImGui::Text("Mouse        look around");
    ImGui::Text("E            talk / pick up / enter");
    ImGui::Text("Tab          open / close the phone (quests)");
    ImGui::Separator();
    ImGui::Text("F1 or H      show / hide this help");
    ImGui::Text("P            pause / resume");
    ImGui::Text("F3           toggle atmosphere: fog + lamp point lights ON / OFF");
    ImGui::Text("Left / Right rotate the sun (light direction)");
    ImGui::Text("Esc          minimize the window");
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