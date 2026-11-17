#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "shaderClass.h"
#include "Player.h"
#include "World.h"
#include "UI.h"

enum class State { Playing, Dialogue };

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

    Shader shader("default.vert", "default.frag");

    // ---------- Мир ----------
    World world;
    world.BuildDemo();
    // Когда появится своя карта из Blender:
    // world.LoadMap("Resources/map/map.obj", 1.0f);
    // world.SetFloorTexture("Resources/textures/grass.png");

    NPC cook("Повар", glm::vec3(3.0f, 0.0f, -5.0f), glm::vec3(0.9f, 0.4f, 0.2f));
    cook.dialogue = {
        { "Повар", "О, новый посетитель! Добро пожаловать на наш шашлычный двор." },
        { "Повар", "Мясо почти готово. Загляни позже, а пока осмотрись." },
        { "Повар", "И не трогай мангал, он ещё горячий!" }
    };
    world.AddNPC(cook);

    NPC guard("Охранник", glm::vec3(-3.0f, 0.0f, 6.0f), glm::vec3(0.2f, 0.4f, 0.9f));
    guard.patrol = { glm::vec3(-3, 0, 6), glm::vec3(5, 0, 6), glm::vec3(5, 0, 12), glm::vec3(-3, 0, 12) };
    guard.dialogue = {
        { "Охранник", "Стой. Здесь территория закрыта." },
        { "Охранник", "Шучу, проходи." }
    };
    world.AddNPC(guard);

    Player player(W, H, glm::vec3(0.0f, 0.0f, 8.0f));
    gPlayer = &player;

    UI ui;
    ui.Init(window);

    // ---------- Игровой цикл ----------
    State state = State::Playing;
    NPC* talkingTo = nullptr;
    int lineIndex = 0;
    bool prevE = false, prevEsc = false;
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
        bool ePressed = eDown && !prevE;
        bool escPressed = escDown && !prevEsc;
        prevE = eDown;
        prevEsc = escDown;

        // ---------- Update ----------
        player.Update(window, dt, world, state == State::Playing);
        world.Update(dt, player.Position);

        NPC* target = nullptr;
        if (state == State::Playing)
        {
            if (escPressed) glfwSetWindowShouldClose(window, true);

            target = world.FindInteractable(player.EyePosition(), player.camera.Orientation, 2.5f);
            if (target && ePressed && !target->dialogue.empty())
            {
                talkingTo = target;
                talkingTo->StartTalking();
                lineIndex = 0;
                state = State::Dialogue;
                target = nullptr;
            }
        }
        else // Dialogue
        {
            bool finish = escPressed;
            if (ePressed)
            {
                lineIndex++;
                if (lineIndex >= (int)talkingTo->dialogue.size()) finish = true;
            }
            if (finish)
            {
                talkingTo->StopTalking();
                talkingTo = nullptr;
                state = State::Playing;
            }
        }

        // ---------- Render ----------
        glClearColor(0.53f, 0.74f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Activate();
        player.camera.Matrix(shader, "camMatrix");
        glUniform3f(glGetUniformLocation(shader.ID, "lightDir"), -0.4f, -1.0f, -0.3f);
        world.Draw(shader);

        ui.BeginFrame();
        ui.DrawCrosshair();
        ui.DrawFPS(1.0f / (dt > 0.0001f ? dt : 0.0001f));
        if (target)
            ui.DrawPrompt("[E] Говорить: " + target->name);
        if (state == State::Dialogue && talkingTo)
        {
            const DialogueLine& line = talkingTo->dialogue[lineIndex];
            ui.DrawDialogue(line.speaker, line.text, lineIndex, (int)talkingTo->dialogue.size());
        }
        ui.EndFrame();

        glfwSwapBuffers(window);
    }

    ui.Shutdown();
    world.Delete();
    shader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
