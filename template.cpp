#include "template.h"

#include <wiiuse/wpad.h>
#include "mgdl-input-wii.h"
#include <string>
#include <cstdlib>

#include <ogc/lwp_watchdog.h>

#include "const.h"
#include "pointer_png.h"
#include "font8x16_png.h"
#include "slurps_wav.h"
#include "fly_png.h"

#include "random_sammakko_png.h"
#include "lick_frame1_png.h"
#include "lick_frame2_png.h"
#include "lick_frame3_png.h"
#include "wide_bg_png.h"
#include "pahaa_png.h"
#include "rouskis_wav.h"
#include "logo_png.h"
#include "pelimusa_ogg.h"
#include "woodwind_ogg.h"
#include "nurmikko_medium_png.h"

static u64 deltaTimeStart = 0;
static u64 programStart = 0;

Template::Template()
{
}

void Template::Init()
{
    frogScale = 0.5f;
    frogSit.LoadImageBuffer(random_sammakko_png, random_sammakko_png_size, gdl::Nearest, gdl::RGBA8);

    pahaa.LoadImageBuffer(pahaa_png, pahaa_png_size, gdl::Nearest, gdl::RGBA8);
    kieli_frames[0].LoadImageBuffer(lick_frame1_png, lick_frame1_png_size, gdl::Nearest, gdl::RGBA8);
    kieli_frames[1].LoadImageBuffer(lick_frame2_png, lick_frame2_png_size, gdl::Nearest, gdl::RGBA8);
    kieli_frames[2].LoadImageBuffer(lick_frame3_png, lick_frame3_png_size, gdl::Nearest, gdl::RGBA8);
    kieli_timer = 0;
    kieli_interval = 0.1;
    kieli_frame = 0;
    kieli_animation_direction = 1;

    gdl::SpriteSetConfig pahaacf = pahaa_sprites.CreateConfig(4, 582/4, 138);
    pahaa_sprites.LoadSprites(pahaacf, &pahaa);
    pahaa_timer = 0;
    pahaa_interval = 0.3;
    pahaa_frame = 0;

    pond.LoadImageBuffer(wide_bg_png, wide_bg_png_size, gdl::Nearest, gdl::RGBA8);
    logo.LoadImageBuffer(logo_png, logo_png_size, gdl::Nearest, gdl::RGBA8);
    foreground_grass.LoadImageBuffer(nurmikko_medium_png, nurmikko_medium_png_size, gdl::Nearest, gdl::RGBA8);

    flySnack.LoadImageBuffer(fly_png, fly_png_size, gdl::Nearest, gdl::RGBA8);
    gdl::SpriteSetConfig flyCfg = flySnackSprites.CreateConfig(2, 180/2, 79);
    flySnackSprites.LoadSprites(flyCfg, &flySnack);
    fly = Snack(&flySnackSprites, glm::vec2(gdl::ScreenCenterX, gdl::ScreenYres/5));
    
    pointerImage.LoadImageBuffer(pointer_png, pointer_png_size, gdl::Nearest, gdl::RGBA8);

    ibmFontImage.LoadImageBuffer(font8x16_png, font8x16_png_size, gdl::Nearest, gdl::RGBA8);
    ibmFont.BindSheet(ibmFontImage, 8, 16, ' ');
    
    slurps.LoadSound(slurps_wav, slurps_wav_size);
    rouskis.LoadSound(rouskis_wav, rouskis_wav_size);
    pelimusa.LoadFromBuffer(pelimusa_ogg, pelimusa_ogg_size);
    woodwind.LoadFromBuffer(woodwind_ogg, woodwind_ogg_size);

    tongueHitRadius = 20.0f;
    tongueHitCenterOffset = glm::vec2(-350, -165);

    jumpBufferTime = 400;

    // settime((u64)0); // Setting time to 0 crashes Dolphin!
    deltaTimeStart = gettime();
    programStart = deltaTimeStart;
    // Initialize randomness
    std::srand(deltaTimeStart);

    // Start music etc...
    ChangeGameState(GameState::StartScreen);
}

void Template::ChangeGameState(GameState newState)
{
    if (newState == GameState::GameLoop)
    {
        frogState.fliesCaught = 0;
        frogState.flyCaught = false;
        woodwind.StopMusic();
        pelimusa.PlayMusic(false);
    }
    else if (newState == GameState::StartScreen)
    {
        pelimusa.StopMusic();
        woodwind.PlayMusic(false);
    }
    currentState = newState;
}

void Template::Update()
{
    u64 now = gettime();
    elapsedMS=ticks_to_millisecs(now - programStart);
    deltaTime = (double)(now - deltaTimeStart) / (double)(TB_TIMER_CLOCK * 1000); // division is to convert from ticks to seconds
    deltaTimeF = (float)deltaTime;
    deltaTimeStart = now;

    // Used in both states
    frogRollRadians = gdl::WiiInput::GetRoll();

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
    glm::vec2 cursorPosInWorld = screenToWorld(cursorPosInScreen);

    frogState.pos += frogState.velocity * deltaTimeF;
    frogState.velocity += glm::vec2(0.f, -9.81f) * deltaTimeF;

    if (frogState.pos.y <= groundY) {
        frogState.pos.y = groundY;
        frogState.velocity.y = 0.f;
    }

    bool frogOnGround = frogState.pos.y <= groundY && frogState.velocity.y <= 0.f;

    if (gdl::WiiInput::ButtonPress(WPAD_BUTTON_A) || gdl::WiiInput::ButtonHeld(WPAD_BUTTON_A)) {
        jumpPressedTime = elapsedMS;
    }

    if (frogOnGround) {
        glm::vec2 frogWalkDiff = cursorPosInWorld - frogState.pos;
        if (glm::abs(frogWalkDiff.x) > 0.1f) {
            frogState.velocity = glm::vec2((frogWalkDiff * frog_walk_speed).x, frog_jump_speed_small);
        }

        if (elapsedMS - jumpPressedTime <= jumpBufferTime)
        {
            frogState.velocity = glm::normalize(frogWalkDiff) * frog_jump_speed_high;
        }
    }
    else
    {
        if (gdl::WiiInput::ButtonHeld(WPAD_BUTTON_B)) {
            ChangeFrogAnimation(FrogAnimation::Lick);
        }
    }

    UpdateFrog();

    fly.Update(deltaTime);

    // Check if the fly is caught
    bool tongueOut = frogState.currentAnimation == FrogAnimation::Lick && kieli_frame >= 1;
    float tongueToFly = glm::length(fly.position - GetTongueHitCenter());
    if (tongueOut &&  tongueToFly < (tongueHitRadius + fly.GetCatchRadius()))
    {
        // Catch the fly
        fly.ResetToRandom();
        frogState.flyCaught = true;
    }
    // Check and loop music
    if (StatusOgg() == OGG_STATUS_EOF)
    {
        // Music has stopped, start again
        pelimusa.PlayMusic(false);
    }
    if (frogState.fliesCaught >= 5)
    {
        ChangeGameState(GameState::StartScreen);
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
            // Add fly here so that the whole animation
            // plays before game ends
            frogState.fliesCaught += 1;
            ChangeFrogAnimation(FrogAnimation::Sit);
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


    fly.Draw(&ibmFont);

    // DEBUG Draw tongue catch hitbox

    /*
    bool tongueOut = frogState.currentAnimation == FrogAnimation::Lick && kieli_frame >= 1;
    u32 color = gdl::Color::Red;
    if (tongueOut)
    {
        color = gdl::Color::Black;
    }
    // float tongueToFly = glm::length(fly.position - GetTongueHitCenter());
    glm::vec2 hb = GetTongueHitCenter();
    short tr = tongueHitRadius;
    glm::vec2 fp = fly.position;
    short fr = fly.GetCatchRadius();
    gdl::DrawBox(hb.x - tr, hb.y - tr, hb.x + tr, hb.y + tr, color);
    gdl::DrawBox(fp.x - fr, fp.y - fr, fp.x + fr, fp.y + fr, color);
    */
    // DEBUGEND

    DrawFrog();

    foreground_grass.Put(0, gdl::ScreenYres - foreground_grass.Ysize(), gdl::Color::White, 0, 0, 1.f);

    // Draw fly score
    short scoreSize = 60;
    short scoreBoxX =gdl::ScreenXres - scoreSize*1.5f;
    short scoreBoxY =gdl::ScreenYres - scoreSize*1.5f;
    gdl::DrawBoxFG(scoreBoxX, scoreBoxY, scoreBoxX + scoreSize, scoreBoxY+scoreSize,
                  gdl::Color::White, gdl::Color::White,
                   gdl::Color::LightBlue, gdl::Color::LightBlue);
    float scoreScale = 4.0f;
    ibmFont.Printf(scoreBoxX + scoreSize/2 - (ibmFont.GetWidth() * scoreScale)/2.0f + 4,
                   scoreBoxY + scoreSize/2 - (ibmFont.GetHeight() * scoreScale)/2.0f + 4,
                   scoreScale, gdl::Color::Black, "%d", 5 - frogState.fliesCaught);

    ibmFont.Printf(scoreBoxX + scoreSize/2 - (ibmFont.GetWidth() * scoreScale)/2.0f,
                   scoreBoxY + scoreSize/2 - (ibmFont.GetHeight() * scoreScale)/2.0f,
                   scoreScale, gdl::Color::LightRed, "%d", 5 - frogState.fliesCaught);

    // Input
    short top = 32;
    short left = 32;
    DrawInputInfo(left, top);


    // DEBUG jump buffering
    /*
    if (elapsedMS - jumpPressedTime <= jumpBufferTime)
    {
        ibmFont.Printf(gdl::ScreenCenterX - scoreSize - (ibmFont.GetWidth() * scoreScale)/2.0f,
                   scoreBoxY + scoreSize/2 - (ibmFont.GetHeight() * 1.0f)/2.0f,
                   2.0f, gdl::Color::LightRed, "Buffer %d", (elapsedMS - jumpPressedTime));
    }
    */
}

void Template::DrawPahaaAnimaatio(int x, int y)
{
    // Scale * 2 since the animation spritesheet
    // is half the size of other frog images
    pahaa_sprites.Put(x, y, pahaa_frame, gdl::Color::White,
                      gdl::Centered, gdl::Centered,
                      frogScale * 2.f, RadToDeg(frogRollRadians));
}

void Template::DrawInputInfo(int x, int y)
{
    // Draw cursor
    gdl::vec2 cp = gdl::WiiInput::GetCursorPosition();

    pointerImage.Put(cp.x,cp.y,gdl::Color::White, gdl::Centered, gdl::Centered, 0.25f);
}

glm::vec2 Template::GetTongueHitCenter()
{
    glm::vec2 frogRenderPos = GetFrogRenderPos();
    // Rotate offset
    float rad = frogRollRadians;
    float cosRoll = cos(rad);
    float sinRoll = sin(rad);
    float rx = cosRoll * tongueHitCenterOffset.x - sinRoll * tongueHitCenterOffset.y;
    float ry = sinRoll * tongueHitCenterOffset.x + cosRoll *tongueHitCenterOffset.y;
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
    return p_screen / glm::vec2(worldToScreenScale, -worldToScreenScale) - glm::vec2(renderOffset.x, -renderOffset.y) / worldToScreenScale;
}

glm::vec2 Template::worldToScreen(glm::vec2 p_world) {
    return p_world * glm::vec2(worldToScreenScale, -worldToScreenScale) + renderOffset;
}

void Template::UpdateStartScreen()
{
    if (gdl::WiiInput::ButtonHeld(WPAD_BUTTON_A) && gdl::WiiInput::ButtonHeld(WPAD_BUTTON_B))
    {
        ChangeGameState(GameState::GameLoop);
    }

    if (StatusOgg() == OGG_STATUS_EOF)
    {
        // Music has stopped, start again
        woodwind.PlayMusic(false);
    }
}

void Template::DrawStartScreen()
{
    float infoScale = 2.5f;
    std::string infos[3] = {
        "A: Hop B: Eat",
        "Roll the Wiimote",
        "Press A and B to start!"
    };


    pond.Put(gdl::ScreenCenterX, gdl::ScreenCenterY, gdl::Color::White,
             gdl::AlignmentModes::Centered, gdl::AlignmentModes::Centered, 1.0f, 0.0f);

    logo.Put(gdl::ScreenCenterX, gdl::ScreenCenterY - ibmFont.GetHeight()*infoScale,
             gdl::Color::White, gdl::Centered, gdl::Centered, 1.0f, 0.0f);

    frogSit.Put(gdl::ScreenCenterX + frogSit.Xsize()*frogScale*2.3f,
                gdl::ScreenYres - frogSit.Ysize()*frogScale*0.8f,
                gdl::Color::White, gdl::Centered, gdl::Centered,
                frogScale, RadToDeg(frogRollRadians));

    for (int i = 0; i < 3; i++)
    {
        DrawTextDouble(infos[i].c_str(),
            gdl::ScreenCenterX - 60,
            gdl::ScreenCenterY + gdl::ScreenYres/6 + ibmFont.GetHeight() * (float)i * infoScale,
            infoScale,
            &ibmFont, gdl::Color::LightRed);

    }
}

void Template::ChangeFrogAnimation(FrogAnimation newAnimation)
{
    if (frogState.currentAnimation == newAnimation)
    {
        return;
    }
    // Eating animation cannot be interrupted by Lick
    if (newAnimation == FrogAnimation::Lick && frogState.currentAnimation == FrogAnimation::Eat)
    {
        return;
    }
    if (newAnimation == FrogAnimation::Eat)
    {
        frogState.flyCaught = false;
        rouskis.Play(1.0f, 100.0f);
    }
    else if (newAnimation == FrogAnimation::Lick)
    {
        float pitch =  (float)rand()/(float)RAND_MAX;
        slurps.Play(0.5f + pitch, 100.0f);
    }

    frogState.currentAnimation = newAnimation;
}

void Template::UpdateFrog()
{
    switch (frogState.currentAnimation)
    {
        case FrogAnimation::Sit:
            // No animation
            break;
        case FrogAnimation::Lick:
            UpdateLickAnimation();
            break;
        case FrogAnimation::Eat:
            UpdatePahaaAnimaatio();
            break;
    };
}

void Template::UpdateLickAnimation()
{
    kieli_timer += deltaTime;
    if (kieli_timer >= kieli_interval)
    {
        kieli_timer = 0;
        kieli_frame += kieli_animation_direction;
        if (kieli_animation_direction > 0 && kieli_frame > 2)
        {
            // Start going back
            kieli_animation_direction = -1;
            kieli_frame += kieli_animation_direction;
        }
        else if (kieli_animation_direction < 0 && kieli_frame < 0 )
        {
            // End
            kieli_frame = 0;
            kieli_animation_direction = 1;
            if (frogState.flyCaught)
            {
                ChangeFrogAnimation(FrogAnimation::Eat);
            }
            else
            {
                ChangeFrogAnimation(FrogAnimation::Sit);
            }
        }
    }
}

void Template::DrawFrog()
{
    glm::vec2 frogRenderPos = worldToScreen(frogState.pos);
    switch (frogState.currentAnimation)
    {
        case FrogAnimation::Sit:
            frogSit.Put(
                frogRenderPos.x, frogRenderPos.y,
                gdl::Color::White, gdl::Centered, gdl::Centered, frogScale, RadToDeg(frogRollRadians));
        break;
        case FrogAnimation::Lick:
        {
            //ibmFont.Printf(10, 10, 1.0f, gdl::Color::Red, "Lick: timer%f frame%d", kieli_timer, kieli_frame);
            float sitWh = frogSit.Xsize() /2;
            // depending on the frame of lick animation
            float lickW = kieli_frames[kieli_frame].Xsize();
            float lickCenterX = lickW - sitWh;
            kieli_frames[kieli_frame].Put(
                frogRenderPos.x, frogRenderPos.y,
                gdl::Color::White, lickCenterX, gdl::Centered, frogScale, RadToDeg(frogRollRadians));
        }
        break;
        case FrogAnimation::Eat:
            DrawPahaaAnimaatio(frogRenderPos.x, frogRenderPos.y);
            break;
    };
}
