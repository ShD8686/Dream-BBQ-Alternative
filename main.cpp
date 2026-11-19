#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "shaderClass.h"
#include "Player.h"
#include "World.h"
#include "UI.h"
#include "Audio.h"

enum class State { Playing, Dialogue };
enum class Scene { SnowWorld, DarkStairs, NextScene, BSOD };
enum class FadeState { None, FadeOut, FadeIn };

struct Quest
{
    std::string id;
    std::string text;
    bool unlocked = false;
    bool completed = false;
};

static Player* gPlayer = nullptr;

void framebufferSizeCallback(GLFWwindow*, int w, int h)
{
    if (w == 0 || h == 0) return;
    glViewport(0, 0, w, h);
    if (gPlayer) { gPlayer->camera.width = w; gPlayer->camera.height = h; }
}

int main()
{
    const int W = 1280, H = 720;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(W, H, "Dream BBQ Alternative 2", NULL, NULL);
    if (!window)
    {
        std::cout << "Не удалось создать окно" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL();
    glViewport(0, 0, W, H);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    // 2nd OS window for the phone (hidden at startup)
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* phoneWindow = glfwCreateWindow(320, 560, "Phone", NULL, window);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    if (phoneWindow)
    {
        glfwMakeContextCurrent(phoneWindow);
        glfwSwapInterval(0);
        glfwMakeContextCurrent(window);
    }

    Shader shader("default.vert", "default.frag");

    // ---------- Audio ----------
    Audio audio;
    audio.Init();
    audio.PlayMusic("Resources/audio/music_snow.mp3", 0.55f);
    audio.PlayAmbient("Resources/audio/ambient_fog_wind.mp3", 0.45f);

    // ---------- Мир ----------
    World world;
    world.BuildDemo();

    std::vector<Quest> quests = {
        { "clear_fog",  "Make fog disappear", false, false },
        { "bring_fish", "Bring fish to NPC",  false, false }
    };
    bool phoneOpen = false;
    bool hasFish = false;
    bool fishDelivered = false;
    bool fogCleared = false;
    std::vector<DialogueLine> customDialogue;
    bool darkIntroShown = false;
    Scene scene = Scene::SnowWorld;

    FadeState fadeState = FadeState::None;
    float fadeAlpha = 0.0f;
    glm::vec3 fadeColor(0.0f);
    Scene pendingScene = Scene::SnowWorld;

    float palaceTimer = 45.0f;
    int lastCountdownSec = 11;
    float bsodTimer = 0.0f;
    float stepDistance = 0.0f;

    // 1. NPC near fountain
    NPC cook("Повар", glm::vec3(19.5f, 0.0f, -26.5f), glm::vec3(0.9f, 0.4f, 0.2f));
    cook.givesQuestId = "bring_fish";
    cook.dialogue = {
        { "Why is this?", "Well hello, my dear friend! You probably wondering why there is so many fog, yes?" },
        { "Weirdo", "As you can see behind me there is a fountaion, that have heated water" },
        { "Weirdo", "In this cold place it's starting to vaporise." },
        { "Weirdo", "If you just could bring me a fish, I could think about how we could solve that." },
        { "Weirdo", "Give a man a bread and you feed him for a day; give man a fish and and he solves all your problem or something." }
    };
    world.AddNPC(cook);

    // 2. NPC near pyramid
    NPC guard("Охранник", glm::vec3(-11.5f, 0.0f, -0.5f), glm::vec3(0.2f, 0.4f, 0.9f));
    guard.givesQuestId = "clear_fog";
    guard.patrol = { glm::vec3(-17.0f, 0.0f, -0.5f), glm::vec3(-11.0f, 0.0f, -0.5f) };
    guard.dialogue = {
        { "Bouncer", "Wait wait wait, face control first. I can't just allow anyone to come to this mega Halloween Party!" },
        { "Bouncer", "However with that thick fog it's not like I could see your face anyway. Probably today is not your lucky day" }
    };
    world.AddNPC(guard);

    // 3. Collectable Fish NPC hidden near a far corner
    NPC fish("Рыба", glm::vec3(-46.0f, 0.0f, 46.0f));
    fish.MakeFish();
    fish.dialogue = {
        { "Fish", "Just imagine, I just got my bachelors diploma and now I can start to build my future! Hey, what are you d..." }
    };
    world.AddNPC(fish);

    Player player(W, H, glm::vec3(0.0f, 0.0f, 8.0f));
    gPlayer = &player;

    UI ui;
    ui.Init(window);
    if (phoneWindow) ui.InitPhone(phoneWindow);

    // ---------- Игровой цикл ----------
    State state = State::Playing;
    NPC* talkingTo = nullptr;
    int lineIndex = 0;
    bool prevE = false, prevEsc = false, prevTab = false;
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;
        if (dt > 0.05f) dt = 0.05f;

        glfwPollEvents();

        bool eDown = glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS;
        bool escDown = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
        bool tabDown = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
        bool ePressed = eDown && !prevE;
        bool escPressed = escDown && !prevEsc;
        bool tabPressed = tabDown && !prevTab;
        prevE = eDown;
        prevEsc = escDown;
        prevTab = tabDown;

        if (tabPressed && phoneWindow && scene != Scene::BSOD)
        {
            phoneOpen = !phoneOpen;
            audio.PlaySFX("Resources/audio/sfx_phone_toggle.wav");
            if (!phoneOpen && glfwGetWindowAttrib(phoneWindow, GLFW_VISIBLE))
                glfwHideWindow(phoneWindow);
        }

        // ---------- Fade Transition Update ----------
        const float fadeSpeed = 0.5f;
        if (fadeState == FadeState::FadeOut)
        {
            fadeAlpha += fadeSpeed * dt;
            if (fadeAlpha >= 1.0f)
            {
                fadeAlpha = 1.0f;
                fadeState = FadeState::FadeIn;

                if (pendingScene == Scene::DarkStairs)
                {
                    scene = Scene::DarkStairs;
                    world.BuildDarkStairsScene();
                    player.Position = glm::vec3(0.0f, 0.0f, 22.0f);
                    player.groundY = 0.0f;

                    bool hasExitQuest = false;
                    for (const Quest& q : quests)
                        if (q.id == "exit") hasExitQuest = true;
                    if (!hasExitQuest)
                        quests.push_back({ "exit", "Exit", true, false });
                    phoneOpen = true;

                    audio.PlaySFX("Resources/audio/sfx_new_quest.mp3");
                    audio.PlayMusic("Resources/audio/music_dark_stairs.mp3", 0.60f);
                }
                else if (pendingScene == Scene::NextScene)
                {
                    for (Quest& q : quests)
                        if (q.id == "exit") q.completed = true;

                    scene = Scene::NextScene;
                    world.BuildNextScene();
                    player.Position = glm::vec3(0.0f, 0.0f, 110.0f);
                    player.groundY = 0.0f;
                    palaceTimer = 40.0f;
                    lastCountdownSec = 11;

                    quests.push_back({ "palace", "Get to the palace in time", true, false });
                    phoneOpen = true;

                    audio.PlaySFX("Resources/audio/sfx_new_quest.mp3");
                    audio.PlayMusic("Resources/audio/music_palace_run.mp3", 0.65f);
                }
            }
        }
        else if (fadeState == FadeState::FadeIn)
        {
            fadeAlpha -= fadeSpeed * dt;
            if (fadeAlpha <= 0.0f)
            {
                fadeAlpha = 0.0f;
                fadeState = FadeState::None;

                // Automatically start dialogue once inside the dark room
                if (scene == Scene::DarkStairs && !darkIntroShown)
                {
                    darkIntroShown = true;
                    customDialogue = {
                        { "???", "..." },
                        { "???", "... ..." },
                        { "???", "... ... ..." },
                        { "???", "You're really a stubborn isn't it? Do you really want to know what awaits you next?" },
                        { "???", "Your patience will be tested, don't act like we didn't warned you." }
                    };
                    talkingTo = nullptr;
                    lineIndex = 0;
                    state = State::Dialogue;
                    audio.SetMusicVolume(0.25f);
                    audio.PlaySFX("Resources/audio/sfx_dialogue_next.wav");
                }
            }
        }

        // ---------- Update ----------
        if (scene == Scene::BSOD)
        {
            bsodTimer += dt;
            if (bsodTimer >= 3.0f)
                glfwSetWindowShouldClose(window, true);
        }
        else
        {
            bool canControl = (state == State::Playing && fadeState == FadeState::None);
            glm::vec3 prevPos = player.Position;
            player.Update(window, dt, world, canControl);

            if (canControl && std::abs(player.Position.y - player.groundY) < 0.05f)
            {
                float moved = glm::length(glm::vec2(player.Position.x - prevPos.x, player.Position.z - prevPos.z));
                stepDistance += moved;
                if (stepDistance >= 1.9f)
                {
                    stepDistance = 0.0f;
                    if (scene == Scene::SnowWorld)
                        audio.PlaySFX("Resources/audio/sfx_step_snow.wav");
                    else if (scene == Scene::DarkStairs)
                        audio.PlaySFX("Resources/audio/sfx_step_stone.wav");
                    else if (scene == Scene::NextScene)
                        audio.PlaySFX("Resources/audio/sfx_step_grass.wav");
                }
            }

            if (scene == Scene::DarkStairs && fadeState == FadeState::None)
            {
                // Walk into white exit rectangle -> Fade to White -> Palace Scene
                if (std::abs(player.Position.x) < 1.8f && player.Position.z < -23.8f && player.Position.y >= 4.2f)
                {
                    fadeState = FadeState::FadeOut;
                    fadeAlpha = 0.0f;
                    fadeColor = glm::vec3(1.0f, 1.0f, 1.0f); // White fade
                    pendingScene = Scene::NextScene;
                    audio.PlaySFX("Resources/audio/sfx_exit_white.wav");
                }
            }
            else if (scene == Scene::NextScene && fadeState == FadeState::None)
            {
                palaceTimer -= dt;

                int sec = (int)std::ceil(palaceTimer);
                if (palaceTimer <= 10.0f && sec > 0 && sec < lastCountdownSec)
                {
                    lastCountdownSec = sec;
                    audio.PlaySFX("Resources/audio/sfx_timer_tick.wav");
                }

                if (palaceTimer <= 0.0f)
                {
                    palaceTimer = 0.0f;
                    scene = Scene::BSOD;
                    bsodTimer = 0.0f;
                    phoneOpen = false;
                    if (phoneWindow && glfwGetWindowAttrib(phoneWindow, GLFW_VISIBLE))
                        glfwHideWindow(phoneWindow);

                    audio.StopMusic();
                    audio.StopAmbient();
                    audio.PlaySFX("Resources/audio/sfx_bsod.mp3");
                }
            }

            world.Update(dt, player.Position);
        }

        NPC* target = nullptr;
        if (scene != Scene::BSOD && fadeState == FadeState::None)
        {
            if (state == State::Playing)
            {
                if (escPressed) glfwIconifyWindow(window);

                target = world.FindInteractable(player.EyePosition(), player.camera.Orientation, 3.0f);
                if (target && ePressed && !target->dialogue.empty())
                {
                    if (target->name == "Повар")
                    {
                        if (fishDelivered)
                            target->dialogue = { { "Weirdo", "May I help you?." } };
                        else if (hasFish)
                            target->dialogue = {
                                { "Weirdo", "Yeaaaah, now we do something!" },
                                { "Weirdo", "Like a fish to water" }
                        };
                    }
                    else if (target->name == "Охранник")
                    {
                        if (fogCleared)
                            target->dialogue = {
                                { "Bouncer", "Now I can see you." },
                                { "Охранник", "No, I still don't know who you are? We we been studying together? Ok anyway, you may enter" }
                        };
                    }

                    talkingTo = target;
                    talkingTo->StartTalking();
                    lineIndex = 0;
                    state = State::Dialogue;
                    target = nullptr;

                    audio.SetMusicVolume(0.25f);
                    if (talkingTo->name == "Рыба")
                        audio.PlaySFX("Resources/audio/sfx_fish_gulp.mp3");
                    else if (talkingTo->name == "Повар")
                        audio.PlaySFX("Resources/audio/sfx_talk_cook.wav");
                    else
                        audio.PlaySFX("Resources/audio/sfx_talk_guard.wav");
                }
            }
            else // Dialogue
            {
                const auto& lines = talkingTo ? talkingTo->dialogue : customDialogue;
                bool completedDialogue = false;
                bool finish = escPressed;
                if (ePressed)
                {
                    lineIndex++;
                    if (lineIndex >= (int)lines.size())
                    {
                        finish = true;
                        completedDialogue = true;
                    }
                    else
                    {
                        audio.PlaySFX("Resources/audio/sfx_dialogue_next.wav");
                    }
                }
                if (finish)
                {
                    NPC* finishedNpc = talkingTo;
                    talkingTo = nullptr;
                    state = State::Playing;
                    audio.SetMusicVolume(0.55f);

                    if (finishedNpc)
                    {
                        std::string npcName = finishedNpc->name;
                        std::string questId = finishedNpc->givesQuestId;
                        finishedNpc->StopTalking();

                        if (completedDialogue)
                        {
                            if (!questId.empty())
                            {
                                for (Quest& q : quests)
                                {
                                    if (q.id == questId && !q.unlocked)
                                    {
                                        q.unlocked = true;
                                        phoneOpen = true;
                                        audio.PlaySFX("Resources/audio/sfx_new_quest.mp3");
                                    }
                                }
                            }

                            if (npcName == "Рыба")
                            {
                                finishedNpc->active = false;
                                hasFish = true;
                            }
                            else if (npcName == "Повар" && hasFish && !fishDelivered)
                            {
                                fishDelivered = true;
                                fogCleared = true;
                                world.PlugFountainWithFish();
                                for (Quest& q : quests)
                                {
                                    q.unlocked = true;
                                    q.completed = true;
                                }

                                audio.StopAmbient();
                                audio.PlaySFX("Resources/audio/sfx_fountain_plug.mp3");
                                audio.PlaySFX("Resources/audio/sfx_fog_clear.wav");
                            }
                            else if (npcName == "Охранник" && fogCleared)
                            {
                                fadeState = FadeState::FadeOut;
                                fadeAlpha = 0.0f;
                                fadeColor = glm::vec3(0.0f, 0.0f, 0.0f);
                                pendingScene = Scene::DarkStairs;
                                audio.PlaySFX("Resources/audio/sfx_portal_dark.mp3");
                            }
                        }
                    }
                }
            }
        }

        // ---------- Render Main Window ----------
        if (scene == Scene::BSOD)
        {
            glClearColor(0.0f, 0.47f, 0.84f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ui.BeginFrame();
            ui.DrawBSOD("Freedom");
            ui.EndFrame();
        }
        else
        {
            if (scene == Scene::DarkStairs)
                glClearColor(0.02f, 0.02f, 0.03f, 1.0f);
            else if (scene == Scene::NextScene)
                glClearColor(0.86f, 0.90f, 0.94f, 1.0f);
            else if (fogCleared)
                glClearColor(0.53f, 0.76f, 0.96f, 1.0f);
            else
                glClearColor(0.86f, 0.90f, 0.94f, 1.0f);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.Activate();
            player.camera.Matrix(shader, "camMatrix");

            if (scene == Scene::DarkStairs)
                glUniform3f(glGetUniformLocation(shader.ID, "lightDir"), 0.0f, -0.3f, -1.0f);
            else
                glUniform3f(glGetUniformLocation(shader.ID, "lightDir"), -0.4f, -1.0f, -0.3f);

            float fog = 0.0f;
            if (scene == Scene::SnowWorld && !fogCleared) fog = 0.032f;
            else if (scene == Scene::NextScene)          fog = 0.017f; // Thicker fog, palace faintly visible
            glUniform1f(glGetUniformLocation(shader.ID, "fogDensity"), fog);

            glm::vec3 camPos = player.EyePosition();
            glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camPos.x, camPos.y, camPos.z);

            world.Draw(shader);

            ui.BeginFrame();
            ui.DrawCrosshair();
            ui.DrawFPS(1.0f / (dt > 0.0001f ? dt : 0.0001f));
            if (scene == Scene::NextScene)
                ui.DrawTimer(palaceTimer);
            if (target)
            {
                if (target->name == "Рыба")
                    ui.DrawPrompt("[E] Взять: Рыба");
                else
                    ui.DrawPrompt("[E] Говорить: " + target->name);
            }
            if (state == State::Dialogue)
            {
                const auto& lines = talkingTo ? talkingTo->dialogue : customDialogue;
                if (lineIndex < (int)lines.size())
                {
                    const DialogueLine& line = lines[lineIndex];
                    ui.DrawDialogue(line.speaker, line.text, lineIndex, (int)lines.size());
                }
            }
            if (fadeAlpha > 0.001f)
                ui.DrawFade(fadeColor.r, fadeColor.g, fadeColor.b, fadeAlpha);
            ui.EndFrame();
        }

        glfwSwapBuffers(window);

        // ---------- Render Phone Window ----------
        if (phoneWindow && scene != Scene::BSOD)
        {
            if (phoneOpen && !glfwGetWindowAttrib(phoneWindow, GLFW_VISIBLE))
            {
                glfwShowWindow(phoneWindow);
                glfwFocusWindow(window);
            }

            if (glfwGetWindowAttrib(phoneWindow, GLFW_VISIBLE))
            {
                if (glfwWindowShouldClose(phoneWindow))
                {
                    glfwHideWindow(phoneWindow);
                    glfwSetWindowShouldClose(phoneWindow, GLFW_FALSE);
                    phoneOpen = false;
                }
                else
                {
                    std::vector<PhoneQuest> visibleQuests;
                    for (const Quest& q : quests)
                        if (q.unlocked) visibleQuests.push_back({ q.text, q.completed });

                    ui.DrawPhoneWindow(phoneWindow, dt, visibleQuests);
                }
            }
        }
    }

    if (phoneWindow)
    {
        ui.ShutdownPhone(phoneWindow);
        glfwDestroyWindow(phoneWindow);
    }

    audio.Shutdown();
    ui.Shutdown();
    world.Delete();
    shader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}