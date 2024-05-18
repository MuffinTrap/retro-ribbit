#pragma once

#include <mgdl-wii.h>
#include "frog.h"
#include "snack.h"

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

    // Frogs
    gdl::Image frogSit;
    gdl::Image frogLick;
    gdl::Image pond;
    gdl::Image pahaa;
    gdl::Image kieli;
    gdl::SpriteSet pahaa_sprites;
    gdl::SpriteSet kieli_sprites;
    double pahaa_timer;
    double pahaa_interval;
    int pahaa_frame;
    double deltaTime;
    float deltaTimeF;
    float elapsed;
    FrogState frogState;
    float groundY = 0.f;
    glm::vec2 renderOffset = glm::vec2(320.f, 480.f * .75f );
    glm::vec2 worldToRenderScale = { 480.f / 4.f, -480.f / 4.f };

    // Snacks
    gdl::Image flySnack;
    gdl::SpriteSet flySnackSprites;

    public:
        Template();
        void Init();
        void Update();
        void Draw();

    private:
        void DrawMenu(int x, int y, int width);
        void DrawInputInfo(int x, int y);
        void DrawTimingInfo(int x, int y, float scale);
        void DrawSprites();
        void DrawPahaaAnimaatio();
        void UpdatePahaaAnimaatio();
};
