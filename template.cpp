#include "template.h"

#include <wiiuse/wpad.h>
#include "mgdl-input-wii.h"
#include <string>
#include <cstdlib>

#include <ogc/lwp_watchdog.h>

#include "const.h"
#include "pointer_png.h"
#include "font8x16_png.h"
#include "sample3_ogg.h"
#include "blipSelect_wav.h"
#include "fly_png.h"

#include "random_sammakko_png.h"
#include "random_sammakko_super_isku_png.h"
#include "wide_bg_png.h"
#include "kieli_png.h"
#include "pahaa_png.h"
#include "rouskis_wav.h"
#include "logo_png.h"

static u64 deltaTimeStart = 0;
static u64 programStart = 0;

Template::Template()
{

}

void Template::Init()
{
    frogScale = 0.5f;
    frogSit.LoadImageBuffer(random_sammakko_png, random_sammakko_png_size, gdl::Nearest, gdl::RGBA8);
    frogLick.LoadImageBuffer(random_sammakko_super_isku_png, random_sammakko_super_isku_png_size, gdl::Nearest, gdl::RGBA8);

    pahaa.LoadImageBuffer(pahaa_png, pahaa_png_size, gdl::Nearest, gdl::RGBA8);
    kieli.LoadImageBuffer(kieli_png, kieli_png_size, gdl::Nearest, gdl::RGBA8);

    gdl::SpriteSetConfig pahaacf = pahaa_sprites.CreateConfig(4, 582/4, 144);
    pahaa_sprites.LoadSprites(pahaacf, &pahaa);
    pahaa_timer = 0;
    pahaa_interval = 0.2;
    pahaa_frame = 0;

    pond.LoadImageBuffer(wide_bg_png, wide_bg_png_size, gdl::Nearest, gdl::RGBA8);
    logo.LoadImageBuffer(logo_png, logo_png_size, gdl::Nearest, gdl::RGBA8);

    flySnack.LoadImageBuffer(fly_png, fly_png_size, gdl::Nearest, gdl::RGBA8);
    gdl::SpriteSetConfig flyCfg = flySnackSprites.CreateConfig(2, 360/2, 158);
    flySnackSprites.LoadSprites(flyCfg, &flySnack);
    
    pointerImage.LoadImageBuffer(pointer_png, pointer_png_size, gdl::Nearest, gdl::RGBA8);


    ibmFontImage.LoadImageBuffer(font8x16_png, font8x16_png_size, gdl::Nearest, gdl::RGBA8);
    ibmFont.BindSheet(ibmFontImage, 8, 16, ' ');
    
    blip.LoadSound(blipSelect_wav, blipSelect_wav_size);
    sampleMusic.LoadFromBuffer(sample3_ogg, sample3_ogg_size);

    menu = gdl::MenuCreator(&ibmFont, 1.5f, 1.2f);

    // settime((u64)0); // Setting time to 0 crashes Dolphin!
    deltaTimeStart = gettime();
    programStart = deltaTimeStart;
    // Initialize randomness
    std::srand(deltaTimeStart);

    fly = Snack(&flySnackSprites, glm::vec2(gdl::ScreenCenterX, gdl::ScreenYres/5));
    tongueHitBoxSize = 60.0f;
    tongueHitBoxOffset = glm::vec2(-350, -165);

    rouskis.LoadSound(rouskis_wav, rouskis_wav_size);

    currentState = GameState::StartScreen;
}

void Template::Update()
{
    u64 now = gettime();
    elapsed=ticks_to_millisecs(now - programStart);
    deltaTime = (double)(now - deltaTimeStart) / (double)(TB_TIMER_CLOCK * 1000); // division is to convert from ticks to seconds
    deltaTimeF = (float)deltaTime;
    deltaTimeStart = now;

    switch(currentState)
    {
        case StartScreen:
            UpdateStartScreen();
            break;
        case GameLoop:
            UpdateGameLoop();
            break;
    };
}

void Template::UpdateGameLoop()
{
    glm::vec2 cursorPosInScreen = { gdl::WiiInput::GetCursorPosition().x , gdl::WiiInput::GetCursorPosition().y };

    frogState.pos += frogState.velocity * deltaTimeF;
    frogState.velocity += glm::vec2(0.f, -9.81f) * deltaTimeF;

    if (frogState.pos.y <= groundY) {
        frogState.pos.y = groundY;
        frogState.velocity.y = 0.f;
    }

    bool frogOnGround = frogState.pos.y <= groundY && frogState.velocity.y <= 0.f;

    if (frogOnGround) {
        glm::vec2 frogWalkDesirePos = screenToWorld(cursorPosInScreen);
        glm::vec2 frogWalkDiff = frogWalkDesirePos - frogState.pos;
        if (glm::abs(frogWalkDiff.x) > 0.1f) {
            frogState.velocity = glm::vec2((frogWalkDiff * frog_walk_speed).x, 1.5f);
        }

        if (gdl::WiiInput::ButtonHeld(WPAD_BUTTON_A)) {
            frogState.velocity = glm::vec2(0.f, 6.f);
        }
    }

    fly.Update(deltaTime);

    // Check if the fly is caught
    bool tongueOut = gdl::WiiInput::ButtonHeld(WPAD_BUTTON_A);
    if (tongueOut && glm::length(fly.position - GetTongueHitboxCenter()) < tongueHitBoxSize/2)
    {
        // Catch the fly
        fly.position = glm::vec2(gdl::ScreenCenterX, -100);
        rouskis.Play(1.0f, 100.0f);
    }
}

void Template::UpdatePahaaAnimaatio()
{
    pahaa_timer += deltaTime;
    if (pahaa_timer >= pahaa_interval)
    {
        pahaa_timer = 0;
        pahaa_frame += 1;
        if (pahaa_frame > 3)
        {
            pahaa_frame = 0;
        }
    }
}

void DrawTextDouble(const char* text, short x, short y, float scale, gdl::FFont* font, u32 color)
{
    font->DrawText(text, x-font->GetWidth(text)*scale/2+4, y+4, scale, gdl::Color::Black);
    font->DrawText(text, x-font->GetWidth(text)*scale/2, y, scale, color);

}

void Template::Draw()
{
    switch(currentState)
    {
        case StartScreen:
            DrawStartScreen();
            break;
        case GameLoop:
            DrawGameLoop();
            break;
    };
}

void Template::DrawGameLoop()
{
    // Draw Image

    pond.Put(gdl::ScreenCenterX, gdl::ScreenCenterY, gdl::Color::White,
             gdl::AlignmentModes::Centered, gdl::AlignmentModes::Centered, 1.0f, 0.0f);

    frogRollRadians = gdl::WiiInput::GetRoll();
    glm::vec2 frogRenderPos = worldToScreen(frogState.pos);
    if (gdl::WiiInput::ButtonHeld(WPAD_BUTTON_A)) 
    {
        float sitWh = frogSit.Xsize() /2;
        float lickW = frogLick.Xsize();
        float lickCenterX = lickW - sitWh;
        frogLick.Put(
            frogRenderPos.x, frogRenderPos.y,
            gdl::Color::White, lickCenterX, gdl::Centered, frogScale, frogRollRadians / PI * 180.f
        );
    }
    else
    {
        frogSit.Put(
            frogRenderPos.x, frogRenderPos.y,
            gdl::Color::White, gdl::Centered, gdl::Centered, frogScale, frogRollRadians / PI * 180.0f
        );
    }

    fly.Draw(&ibmFont);

    // Draw tongue catch hitbox
    glm::vec2 hb = GetTongueHitboxCenter();
    gdl::DrawBox(hb.x - tongueHitBoxSize/2, hb.y - tongueHitBoxSize/2, hb.x + tongueHitBoxSize/2, hb.y + tongueHitBoxSize/2, gdl::Color::Red);

    // Input
    short top = 32;
    short left = 32;
    DrawInputInfo(left, top);
}

void Template::DrawPahaaAnimaatio()
{
    pahaa_sprites.Put(0, 0, pahaa_frame, gdl::Color::White, 0, 0, 1.0f);
}

void Template::DrawInputInfo(int x, int y)
{
    // Draw cursor
    gdl::vec2 cp = gdl::WiiInput::GetCursorPosition();

    pointerImage.Put(cp.x,cp.y,gdl::Color::White, 0, 0, 1.0f);
}

glm::vec2 Template::GetTongueHitboxCenter()
{
    glm::vec2 frogRenderPos = GetFrogRenderPos();
    // Rotate offset
    float rad = frogRollRadians;// + PI;
    float cosRoll = cos(rad);
    float sinRoll = sin(rad);
    float rx = cosRoll * tongueHitBoxOffset.x - sinRoll * tongueHitBoxOffset.y;
    float ry = sinRoll * tongueHitBoxOffset.x + cosRoll *tongueHitBoxOffset.y;
    glm::vec2 rotatedHitCenter = glm::vec2(rx, ry);

    glm::vec2 hitboxCenter = glm::vec2(frogRenderPos.x, frogRenderPos.y) + rotatedHitCenter * frogScale;
    return hitboxCenter;
}

glm::vec2 Template::GetFrogRenderPos()
{
    glm::vec2 frogRenderPos = worldToScreen(frogState.pos);
    return frogRenderPos;
}

glm::vec2 Template::screenToWorld(glm::vec2 p_screen) {
    return p_screen / glm::vec2(worldToScreenScale, -worldToScreenScale) - renderOffset / worldToScreenScale;
}

glm::vec2 Template::worldToScreen(glm::vec2 p_world) {
    return p_world * glm::vec2(worldToScreenScale, -worldToScreenScale) + renderOffset;
}

void Template::UpdateStartScreen()
{
    if (gdl::WiiInput::ButtonPress(WPAD_BUTTON_A) || gdl::WiiInput::ButtonPress(WPAD_BUTTON_B))
    {
        currentState = GameState::GameLoop;
    }
}

void Template::DrawStartScreen()
{
    float infoScale = 3.0f;
    const char* info = "Press A or B";
    const char* info2 = "to start!";

    pond.Put(gdl::ScreenCenterX, gdl::ScreenCenterY, gdl::Color::White,
             gdl::AlignmentModes::Centered, gdl::AlignmentModes::Centered, 1.0f, 0.0f);

    logo.Put(gdl::ScreenCenterX, gdl::ScreenCenterY - ibmFont.GetHeight()*infoScale,
             gdl::Color::White, gdl::Centered, gdl::Centered, 1.0f, 0.0f);

    frogSit.Put(gdl::ScreenCenterX + frogSit.Xsize()*frogScale*2,
                gdl::ScreenYres - frogSit.Ysize()*frogScale,
                gdl::Color::White, 0, 0, frogScale, 0.0f);

    DrawTextDouble(info,
                   gdl::ScreenCenterX,
                   gdl::ScreenCenterY + gdl::ScreenYres/6 + ibmFont.GetHeight() * infoScale,
                   infoScale,
                   &ibmFont, gdl::Color::LightRed);
    DrawTextDouble(info2,
                   gdl::ScreenCenterX,
                   gdl::ScreenCenterY + gdl::ScreenYres/6 + ibmFont.GetHeight() * 2.0f * infoScale,
                   infoScale,
                   &ibmFont, gdl::Color::LightRed);
}
