#pragma once

#include <mgdl-wii.h>
#include "frog.h"
#include "snack.h"

enum GameState
{
    StartScreen,
    GameLoop
};

class Template
{
    gdl::Image ibmFontImage;
    gdl::FFont ibmFont;
    gdl::Image pointerImage;
    gdl::Image logo;
    gdl::Image foreground_grass;

    gdl::Music pelimusa;
    gdl::Music woodwind;
    gdl::Sound slurps;
    gdl::Sound rouskis;

    double deltaTime;
    float deltaTimeF;
    int elapsedMS;

    // Frogs
    gdl::Image frogSit;
    gdl::Image pond;
    gdl::Image pahaa;
    gdl::Image kieli_frames[3];
    gdl::SpriteSet pahaa_sprites;
    gdl::SpriteSet kieli_sprites;
    double pahaa_timer;
    double pahaa_interval;
    int pahaa_frame;

    double kieli_timer;
    double kieli_interval;
    int kieli_frame;
    int kieli_animation_direction;

    FrogState frogState;
    float groundY = 0.f;
    float worldToScreenScale = 480.f / 4.f;
    glm::vec2 renderOffset = glm::vec2(320.f, 480.f * 0.75f);

    // Offset from frog position to where the hitbox is
    glm::vec2 tongueHitCenterOffset;
    float tongueHitRadius;
    float frogRollRadians;

    // Jump buffering timing, in milliseconds
    int jumpPressedTime = 0;
    int jumpBufferTime = 0;

    // Snacks
    gdl::Image flySnack;
    gdl::SpriteSet flySnackSprites;
    Snack fly;

    public:
        Template();
        void Init();
        void Update();
        void Draw();

    private:
        void DrawInputInfo(int x, int y);
        void DrawTimingInfo(int x, int y, float scale);
        void DrawPahaaAnimaatio(int x, int y);
        void UpdatePahaaAnimaatio();

        glm::vec2 screenToWorld(glm::vec2 p_screen);
        glm::vec2 worldToScreen(glm::vec2 p_world);
        glm::vec2 GetTongueHitCenter();
        glm::vec2 GetFrogRenderPos();
        float frogScale;

        GameState currentState;
        void ChangeGameState(GameState newState);

        void UpdateGameLoop();
        void DrawGameLoop();

        void UpdateStartScreen();
        void DrawStartScreen();

        // Drawing and updating the frog with current animation
        void ChangeFrogAnimation(FrogAnimation newAnimation);
        void DrawFrog();
        void UpdateFrog();
        void UpdateLickAnimation();
};
