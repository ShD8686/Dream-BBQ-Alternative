#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>

#include "shaderClass.h"
#include "Player.h"
#include "World.h"
#include "UI.h"
#include "Audio.h"

enum class State { Playing, Dialogue };
enum class Scene { SnowWorld, DarkStairs, NextScene, BSOD, Story };
enum class StoryKind { Fail, TrueEnd };
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

int main(int argc, char** argv)
{
    // Для тестов: запуск с параметром --run сразу открывает уровень с бегом и таймером,
    // чтобы не проходить весь квест заново (Visual Studio: Свойства проекта -> Отладка -> Аргументы команды).
    // Остальные тестовые параметры: --run2 (бег уже с устройством), --stairs2 (вторая локация после провала),
    // --fail (экран авторов после провала), --bsod (счастливый синий экран), --true (финальный текст).
    bool startAtRun = false, startRun2 = false, startStairs2 = false, startFail = false, startBsod = false, startTrue = false;
    for (int i = 1; i < argc; ++i)
    {
        std::string a = argv[i];
        if (a == "--run") startAtRun = true;
        else if (a == "--run2") startAtRun = startRun2 = true;
        else if (a == "--stairs2") startStairs2 = true;
        else if (a == "--fail") startFail = true;
        else if (a == "--bsod") startBsod = true;
        else if (a == "--true") startTrue = true;
    }

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

    // ---------- Сюжет: провал, устройство, концовка ----------
    int failCount = 0;               // сколько раз игрок не добежал до дворца
    bool hasDevice = false;          // подобрано устройство: дворец перестаёт убегать
    bool deviceHintShown = false;
    bool enteringPalace = false;
    const float kPalaceStopZ = -100.0f; // где замирает дворец, когда есть устройство
    StoryKind storyKind = StoryKind::Fail;
    std::vector<StoryLine> story;
    int storyLine = 0;
    float storyLineT = 0.0f;
    float achT = -1.0f;              // время с появления ачивки (<0 = ещё не показана)

    float palaceTimer = 45.0f;
    int lastCountdownSec = 11;
    float bsodTimer = 0.0f;
    float stepDistance = 0.0f;

    // Собирает первую (снежную) локацию с нуля: карта, NPC, квесты и флаги.
    // Вызывается при старте и после провала, чтобы игрок проходил первую локацию заново.
    auto resetSnow = [&]()
    {
        world.BuildDemo();
        quests = {
            { "clear_fog",  "Make fog disappear", false, false },
            { "bring_fish", "Bring fish to NPC",  false, false }
        };
        hasFish = false;
        fishDelivered = false;
        fogCleared = false;
        phoneOpen = false;
        customDialogue.clear();

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
    };
    resetSnow();

    Player player(W, H, glm::vec3(0.0f, 0.0f, 8.0f));
    gPlayer = &player;

    UI ui;
    ui.Init(window);
    if (phoneWindow) ui.InitPhone(phoneWindow);

    auto N = [](const char* t) { return StorySpan{ t, SpanStyle::Normal }; };
    auto FR = [](const char* t) { return StorySpan{ t, SpanStyle::Freedom }; };
    auto TR = [](const char* t) { return StorySpan{ t, SpanStyle::True }; };
    auto hidePhone = [&]()
    {
        phoneOpen = false;
        if (phoneWindow && glfwGetWindowAttrib(phoneWindow, GLFW_VISIBLE))
            glfwHideWindow(phoneWindow);
    };
    State state = State::Playing;
    auto startStory = [&](StoryKind kind)
    {
        storyKind = kind;
        story.clear();
        if (kind == StoryKind::Fail)
        {
            // Каждая фраза после многоточия - отдельный экран: появилась, прочиталась, исчезла, пауза в темноте
            if (failCount <= 1)
            {
                story.push_back({ { N("Your device ran into a problem and needs to restart.") }, 1.5f });
                story.push_back({ { N("Stop cide: EXCEPTION_ANCHOR_NOT_FOUND_OR_EXIST.") }, 1.5f });
                story.push_back({ { N("0% complete") }, 3.0f, 1.0f });
                story.push_back({ { N("43% complete") }, 3.0f, 1.0f });
                story.push_back({ { N("75% complete") }, 3.0f, 1.0f });
                story.push_back({ { N("99% complete") }, 3.0f, 1.0f });
                story.push_back({ { N("100% complete") }, 3.0f, 1.0f });
            }
            else // повторный провал без устройства: подсказка
            {
                story.push_back({ { N("Skill issues ")}, 2.5f });
                story.push_back({ { N("Maybe you should look behind you...") }, 3.0f, 1.0f });
            }
        }
        else
        {
            story.push_back({ { N("Oh, it's you again!") }, 2.0f });
            story.push_back({ { N("So you know thos party kinda sucks") }, 1.8f });
            story.push_back({ { N("We probably have to find better things to do") }, 1.5f });
            StoryLine prize{ { N("Here is your prize") }, 3.5f };
            prize.achievement = true;                       // на этой строке выезжает ачивка
            story.push_back(prize);
            story.push_back({ { N("Thanks for playing!") }, 4.0f, 0.0f });
        }
        storyLine = 0;
        storyLineT = (kind == StoryKind::Fail) ? -10.0f : 0.0f; // после провала ~10 с чёрного экрана до первых слов
        achT = -1.0f;
        scene = Scene::Story;
        state = State::Playing;
        hidePhone();
        audio.StopMusic();
        audio.StopAmbient();
    };

    // ---------- Игровой цикл ----------
    if (startStairs2)
    {
        failCount = 1;
        fadeState = FadeState::FadeOut;
        fadeAlpha = 1.0f;
        fadeColor = glm::vec3(0.0f);
        pendingScene = Scene::DarkStairs;
    }
    if (startFail) { failCount = 1; startStory(StoryKind::Fail); }
    if (startTrue) startStory(StoryKind::TrueEnd);
    if (startBsod) { scene = Scene::BSOD; bsodTimer = 0.0f; }
    if (startRun2) hasDevice = true;
    if (startAtRun)
    {
        // Чёрный/белый экран сразу, а смена сцены произойдёт в первом кадре тем же путём, что и в игре
        fadeState = FadeState::FadeOut;
        fadeAlpha = 1.0f;
        fadeColor = glm::vec3(1.0f);
        pendingScene = Scene::NextScene;
    }

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

        if (tabPressed && phoneWindow && scene != Scene::BSOD && scene != Scene::Story)
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

                if (pendingScene == Scene::SnowWorld)
                {
                    // Игрок начинает заново с первой локации
                    scene = Scene::SnowWorld;
                    resetSnow();
                    hidePhone();
                    player.Position = glm::vec3(0.0f, 0.0f, 8.0f);
                    player.groundY = 0.0f;
                    player.camera.SetLook(-90.0f, 0.0f);
                    audio.PlayMusic("Resources/audio/music_snow.mp3", 0.55f);
                    audio.PlayAmbient("Resources/audio/ambient_fog_wind.mp3", 0.45f);
                }
                else if (pendingScene == Scene::DarkStairs)
                {
                    scene = Scene::DarkStairs;
                    world.BuildDarkStairsScene();
                    player.Position = glm::vec3(0.0f, 0.0f, 22.0f);
                    player.groundY = 0.0f;
                    player.camera.SetLook(-90.0f, 0.0f); // лицом к лестнице, устройство окажется сзади

                    // После провала: на второй локации сзади игрока появляется устройство
                    if (failCount > 0 && !hasDevice)
                    {
                        NPC device("Device", glm::vec3(0.0f, 0.0f, 26.0f), glm::vec3(0.2f));
                        device.MakeDevice();
                        device.yaw = 180.0f; // индикатором к месту, где стоит игрок
                        device.dialogue = {
                            { "Device", "A small box, warm to the touch. A tiny green light is blinking." },
                            { "Device", "Something is written on the side: \"ANCHOR. Keeps things in one place.\"" },
                            { "Device", "You take it with you." }
                        };
                        world.AddNPC(device);
                    }

                    bool hasExitQuest = false;
                    for (Quest& q : quests)
                    {
                        if (q.id == "exit") { hasExitQuest = true; q.completed = false; }
                    }
                    quests.erase(std::remove_if(quests.begin(), quests.end(),
                        [](const Quest& q) { return q.id == "palace"; }), quests.end());
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
                    player.camera.SetLook(-90.0f, 0.0f); // смотрим вдоль дороги, куда бежать
                    palaceTimer = 40.0f;
                    lastCountdownSec = 11;
                    enteringPalace = false;
                    if (hasDevice) world.SetPalaceAnchor(kPalaceStopZ); // устройство держит дворец на месте

                    quests.erase(std::remove_if(quests.begin(), quests.end(),
                        [](const Quest& q) { return q.id == "palace"; }), quests.end());
                    quests.push_back({ "palace", hasDevice ? "Enter the palace" : "Get to the palace in time", true, false });
                    phoneOpen = true;

                    audio.PlaySFX("Resources/audio/sfx_new_quest.mp3");
                    audio.PlayMusic("Resources/audio/music_palace_run.mp3", 0.65f);
                }
                else if (pendingScene == Scene::BSOD)
                {
                    // Вошли во дворец: счастливый синий экран
                    scene = Scene::BSOD;
                    bsodTimer = 0.0f;
                    hidePhone();
                    audio.StopMusic();
                    audio.StopAmbient();
                    audio.PlaySFX("Resources/audio/sfx_bsod.mp3");
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
                else if (scene == Scene::DarkStairs && failCount > 0 && !hasDevice && !deviceHintShown)
                {
                    deviceHintShown = true;
                    customDialogue = { { "???", "Behind you." } };
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
            if (bsodTimer >= 5.0f)          // ~5 секунд счастливого синего экрана, потом слово авторов
                startStory(StoryKind::TrueEnd);
        }
        else if (scene == Scene::Story)
        {
            if (storyLine < (int)story.size())
            {
                storyLineT += dt;
                if (achT >= 0.0f) achT += dt;

                const StoryLine& L = story[storyLine];
                const float typeD = UI::StoryTypeDuration(L);
                const float total = typeD + L.hold + 0.8f + L.gap;

                // E после того, как строка допечаталась, пропускает паузу
                if (ePressed && storyLineT > typeD + 0.4f && storyLineT < typeD + L.hold)
                    storyLineT = typeD + L.hold;

                // Ачивка выезжает, когда допечатана строка "Here is your prize"
                if (L.achievement && achT < 0.0f && storyLineT >= typeD)
                {
                    achT = 0.0f;
                    audio.PlaySFX("Resources/audio/sfx_achievement.mp3");
                }

                if (storyLineT >= total)
                {
                    storyLine++;
                    storyLineT = 0.0f;
                    if (storyLine >= (int)story.size())
                    {
                        if (storyKind == StoryKind::Fail)
                        {
                            // Игра начинается заново с первой локации (экран уже чёрный)
                            fadeState = FadeState::FadeOut;
                            fadeAlpha = 1.0f;
                            fadeColor = glm::vec3(0.0f);
                            pendingScene = Scene::SnowWorld;
                        }
                        else
                        {
                            glfwSetWindowShouldClose(window, true);
                        }
                    }
                }
            }
        }
        else
        {
            bool canControl = (state == State::Playing && fadeState == FadeState::None);
            player.runFx = (scene == Scene::NextScene); // эффекты бега только на уровне с таймером
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
            else if (scene == Scene::NextScene && fadeState == FadeState::None && !hasDevice && !enteringPalace)
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
                    failCount++;
                    startStory(StoryKind::Fail); // чёрный экран, слова авторов
                }
            }

            world.Update(dt, player.Position);
        }

        NPC* target = nullptr;
        const bool cinematic = (scene == Scene::BSOD || scene == Scene::Story);
        bool canEnter = false;
        if (scene == Scene::NextScene && state == State::Playing && fadeState == FadeState::None &&
            !enteringPalace && world.PalaceDoorNear(player.Position))
        {
            canEnter = true;
            if (ePressed)
            {
                enteringPalace = true;
                fadeState = FadeState::FadeOut;
                fadeAlpha = 0.0f;
                fadeColor = glm::vec3(1.0f);
                pendingScene = Scene::BSOD;
                audio.PlaySFX("Resources/audio/sfx_exit_white.wav");
            }
        }
        if (!cinematic && fadeState == FadeState::None)
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
                            else if (npcName == "Device")
                            {
                                finishedNpc->active = false;
                                hasDevice = true;
                                audio.PlaySFX("Resources/audio/sfx_new_quest.mp3");
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
            ui.DrawBSOD("Something wong!", true);
            ui.EndFrame();
        }
        else if (scene == Scene::Story)
        {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ui.BeginFrame();
            if (storyLine < (int)story.size())
            {
                const StoryLine& L = story[storyLine];
                const float typeD = UI::StoryTypeDuration(L);
                float fin = std::min(1.0f, storyLineT / 0.35f);
                float fout = storyLineT > typeD + L.hold ? 1.0f - (storyLineT - typeD - L.hold) / 0.8f : 1.0f;
                ui.DrawStoryLine(L, storyLineT, std::max(0.0f, std::min(fin, fout)));
            }
            if (achT >= 0.0f)
                ui.DrawAchievement("True ending?", achT);
            if (fadeAlpha > 0.001f)
                ui.DrawFade(fadeColor.r, fadeColor.g, fadeColor.b, fadeAlpha);
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
            if (scene == Scene::SnowWorld && !fogCleared) fog = 0.040f; // гуще, чтобы фонари не выдавали маршрут
            else if (scene == Scene::NextScene)          fog = 0.017f; // Thicker fog, palace faintly visible
            glUniform1f(glGetUniformLocation(shader.ID, "fogDensity"), fog);

            glm::vec3 camPos = player.EyePosition();
            glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camPos.x, camPos.y, camPos.z);

            world.UploadLights(shader, (scene == Scene::SnowWorld && !fogCleared) ? 1.0f : 0.0f);
            world.Draw(shader);

            ui.BeginFrame();
            ui.DrawCrosshair();
            ui.DrawFPS(1.0f / (dt > 0.0001f ? dt : 0.0001f));
            if (scene == Scene::NextScene)
            {
                // Последние 10 секунд: чёрная пульсирующая рамка, чем ближе к нулю - тем сильнее и чаще
                if (!hasDevice && palaceTimer <= 10.0f)
                {
                    float urgency = 1.0f - palaceTimer / 10.0f;                              // 0 -> 1
                    float pulse = 0.5f + 0.5f * std::sin((float)now * (6.0f + 8.0f * urgency)); // 0..1
                    ui.DrawVignette(urgency * (0.45f + 0.55f * pulse), 0.0f, 0.0f, 0.0f);
                }
                if (!hasDevice) ui.DrawTimer(palaceTimer);
            }
            if (canEnter)
                ui.DrawPrompt("[E] Enter");
            if (target)
            {
                if (target->name == "Рыба")
                    ui.DrawPrompt("[E] Взять: Рыба");
                else if (target->name == "Device")
                    ui.DrawPrompt("[E] Pick up: Device");
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
        if (phoneWindow && scene != Scene::BSOD && scene != Scene::Story)
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