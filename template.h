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
    gdl::Image mel_image;
    gdl::SpriteSet mel_sprites;
    gdl::Image ibmFontImage;
    gdl::Image pointerImage;
    gdl::FFont ibmFont;
    gdl::Music sampleMusic;
    gdl::Sound blip;
    gdl::MenuCreator menu;
    gdl::Image logo;

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
    double deltaTime;
    float deltaTimeF;
    float elapsed;
    FrogState frogState;
    float groundY = 0.f;
    float worldToScreenScale = 480.f / 4.f;
    glm::vec2 renderOffset = glm::vec2(320.f, 240.f);

    // Offset from frog position to where the hitbox is
    glm::vec2 tongueHitBoxOffset;
    float tongueHitBoxSize;
    float frogRollRadians;

    // Snacks
    gdl::Image flySnack;
    gdl::SpriteSet flySnackSprites;
    Snack fly;

    gdl::Sound rouskis;

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
        glm::vec2 GetTongueHitboxCenter();
        glm::vec2 GetFrogRenderPos();
        float frogScale;

        GameState currentState;

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
