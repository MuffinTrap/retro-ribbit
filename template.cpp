#include "template.h"

#include <wiiuse/wpad.h>
#include "mgdl-input-wii.h"
#include <string>
#include <cstdlib>

#include <ogc/lwp_watchdog.h>

#include "barb_png.h"
#include "mel_tiles_png.h"
#include "pointer_png.h"
#include "font8x16_png.h"
#include "sample3_ogg.h"
#include "blipSelect_wav.h"
#include "fly_png.h"

#include "random_sammakko_png.h"
#include "random_sammakko_super_isku_png.h"
#include "lampi_png.h"
#include "kieli_png.h"
#include "pahaa_png.h"

static u64 deltaTimeStart = 0;
static u64 programStart = 0;

Template::Template()
{

}

void Template::Init()
{
    frogSit.LoadImageBuffer(random_sammakko_png, random_sammakko_png_size, gdl::Nearest, gdl::RGBA8);
    frogLick.LoadImageBuffer(random_sammakko_super_isku_png, random_sammakko_super_isku_png_size, gdl::Nearest, gdl::RGBA8);

    pahaa.LoadImageBuffer(pahaa_png, pahaa_png_size, gdl::Nearest, gdl::RGBA8);
    kieli.LoadImageBuffer(kieli_png, kieli_png_size, gdl::Nearest, gdl::RGBA8);

    gdl::SpriteSetConfig pahaacf = pahaa_sprites.CreateConfig(4, 582/4, 144);
    pahaa_sprites.LoadSprites(pahaacf, &pahaa);
    pahaa_timer = 0;
    pahaa_interval = 0.2;
    pahaa_frame = 0;

    pond.LoadImageBuffer(lampi_png, lampi_png_size, gdl::Nearest, gdl::RGBA8);

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
}

void Template::Update()
{
    u64 now = gettime();
    elapsed=ticks_to_millisecs(now - programStart);
    deltaTime = (double)(now - deltaTimeStart) / (double)(TB_TIMER_CLOCK * 1000); // division is to convert from ticks to seconds
    deltaTimeF = (float)deltaTime;
    deltaTimeStart = now;

    glm::vec2 cursorPosInScreen = { gdl::WiiInput::GetCursorPosition().x , gdl::WiiInput::GetCursorPosition().y };

    frogState.pos += frogState.velocity * deltaTimeF;
    frogState.velocity += glm::vec2(0.f, -9.81f) * deltaTimeF;

    if (frogState.pos.y <= groundY) {
        frogState.pos.y = groundY;
        frogState.velocity.y = 0.f;
    }

    bool frogOnGround = frogState.pos.y <= groundY && frogState.velocity.y <= 0.f;

    if (frogOnGround) {
        frogState.pos.x = screenToWorld(cursorPosInScreen).x;

        if (gdl::WiiInput::ButtonPress(WPAD_BUTTON_A)) {
            frogState.velocity += glm::vec2(0.f, 6.f);
        }
    }

    fly.Update(deltaTime);

    if (gdl::WiiInput::ButtonPress(WPAD_BUTTON_1))
    {
        fly.maxForce += 0.1f;

    }
    if (gdl::WiiInput::ButtonPress(WPAD_BUTTON_2))
    {
        fly.maxSpeed += 0.1f;
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

void DrawTextDouble(const char* text, short x, short y, float scale, gdl::FFont* font)
{
    font->DrawText(text, x-font->GetWidth(text)*scale/2+4, y+4, scale, gdl::Color::Black);
    font->DrawText(text, x-font->GetWidth(text)*scale/2, y, scale, gdl::Color::LightGreen);

}

void Template::Draw()
{
    // Draw Image
    /*
    barb.Put(
            gdl::ScreenCenterX - barb.Xsize()/4, 
            gdl::ScreenCenterY-barb.Ysize()/4, 
            gdl::Color::White, 
            0, 0, 
            0.5f, 0.0f);
    */

    pond.Put(0, 0, gdl::Color::White, 0, 0, 1.0f, 0.0f);
    float frogScale = 1.0f;

    gdl::vec2 cp = gdl::WiiInput::GetCursorPosition();

    gdl::vec2 frogCenter = gdl::vec2(frogSit.Xsize()/2, frogSit.Ysize()/2);

    float frogRoll = gdl::WiiInput::GetRoll();
    if (gdl::WiiInput::ButtonHeld(WPAD_BUTTON_A)) 
    {
        frogLick.Put(
            cp.x + frogCenter.x, cp.y + frogCenter.y,
            gdl::Color::White, gdl::RJustify,gdl::RJustify, frogScale, frogRoll / PI * 180.f
        );
    }
    else
    {
        frogSit.Put(
            cp.x + frogCenter.x, cp.y + frogCenter.y,
            gdl::Color::White, gdl::RJustify,gdl::RJustify, frogScale, 0.0f
        );

    }

    glm::vec2 frogRenderPos = worldToScreen(frogState.pos);
    frogSit.Put(
      frogRenderPos.x, frogRenderPos.y, gdl::Color::White, gdl::Centered, gdl::Centered, 0.1f, 0.f
    );

    fly.Draw(&ibmFont);

    // gdl::DrawBox(cp.x, cp.y, cp.x+ frogSit.Xsize() * frogScale, cp.y + frogSit.Ysize() * frogScale, gdl::Color::Red);
    float fontScale = 2.0f;

    ibmFont.Printf(10, 10, 1.0f, gdl::Color::White, "Frog size: %d", frogSit.Xsize());
    ibmFont.Printf(10, 20, 1.0f, gdl::Color::White, "Lick size: %d", frogLick.Xsize());
    ibmFont.Printf(10, 30, 1.0f, gdl::Color::White, "Pond size: %d", pond.Xsize());
    ibmFont.Printf(10, 40, 1.0f, gdl::Color::Red, "Screen size: %d x %d", gdl::ScreenXres, gdl::ScreenYres);

    DrawTextDouble("MTEK-GDL", gdl::ScreenCenterX, ibmFont.GetHeight(), fontScale, &ibmFont);
    DrawTextDouble(GDL_VERSION, gdl::ScreenCenterX, ibmFont.GetHeight() * 2 * fontScale, fontScale, &ibmFont);


    // Input
    short top = 32;
    short left = 32;
    DrawInputInfo(left, top);
    /*
    DrawMenu(left, top + 120, 120);
    DrawTimingInfo(left, gdl::ScreenYres-ibmFont.GetHeight()*4*1.5f, 1.5f);
*/

    // DrawSprites();
}

void Template::DrawPahaaAnimaatio()
{
    pahaa_sprites.Put(0, 0, pahaa_frame, gdl::Color::White, 0, 0, 1.0f);
}

static void DrawButtons(short x, short y, short size, gdl::FFont* font)
{
    // Draw button states
    gdl::Color::ColorValues active = gdl::Color::Blue;
    gdl::Color::ColorValues inactive = gdl::Color::LightBlue;
    int buttons[] = {
        WPAD_BUTTON_A, 
        WPAD_BUTTON_B,
        WPAD_BUTTON_PLUS,
        WPAD_BUTTON_MINUS,
        WPAD_BUTTON_1,
        WPAD_BUTTON_2
        };
    static std::string names[] ={ "A", "B", "+", "-", "1", "2" };
    for(int i = 0; i < 6;i++ )
    {
        gdl::Color::ColorValues c = inactive;
        if (gdl::WiiInput::ButtonHeld(buttons[i]))
        {
            c = active;
        }
        gdl::DrawBoxF(x + i * size, y, x+size+i*size,y+size,c);
        font->DrawText(names[i].c_str(), x + i*size, y, 1.0f, gdl::Color::White);
    }
}

void DrawDPad(short x, short y, short size)
{
    short box = size/3;
    short h=box/2;
    x += box + h;
    y += box + h;
    // Dpad
    int dpad_buttons[] = {
        WPAD_BUTTON_UP, 
        WPAD_BUTTON_DOWN,
        WPAD_BUTTON_LEFT,
        WPAD_BUTTON_RIGHT
    };
    gdl::vec2 directions[] = {
        gdl::vec2(0,-1), 
        gdl::vec2(0,1),
        gdl::vec2(-1,0),
        gdl::vec2(1,0)
    };
    for (int i=0;i<4;i++)
    {
        gdl::Color::ColorValues c = gdl::Color::LightRed;
        if (gdl::WiiInput::ButtonHeld(dpad_buttons[i]))
        {
            c = gdl::Color::Red;
        }
        gdl::vec2 d=directions[i];
        gdl::DrawBoxF(x+d.x*box-h, y+ d.y*box-h, x+box+d.x*box-h, y+box+d.y*box-h, c);
    }
}

void DrawJoystick(short x, short y, short size)
{
    // Draw joystick direction
    short jsize=size;
    short box = jsize/3;
    short h=box/2;
    gdl::Color::ColorValues jc = gdl::Color::Green;
    gdl::vec2 jdir = gdl::WiiInput::GetNunchukJoystickDirection(0.0f);
    short jleft= x + jsize/2 + jdir.x * box-h;
    short jtop = y + jsize/2 + jdir.y * box-h;
    gdl::DrawBox(x, y, x+jsize, y+jsize, jc);
    if (jdir.x != 0.0f && jdir.y != 0.0f)
    {
        jc = gdl::Color::LightGreen;
    }
    gdl::DrawBoxF(jleft, jtop, jleft+box, jtop+box,jc);
}

void Template::DrawInputInfo(int x, int y)
{
    // Draw cursor
    gdl::vec2 cp = gdl::WiiInput::GetCursorPosition();

    pointerImage.Put(cp.x,cp.y,gdl::Color::White, 0, 0, 1.0f);

/*
    DrawButtons(x, y, 20, &ibmFont);
    y += 20;
    DrawDPad(x, y, 60);
    x += 80;
    DrawJoystick(x, y, 60);
    */
}

void Template::DrawSprites()
{
    const gdl::Sprite* first = mel_sprites.SpriteInfo(0);
    float scale = 2.0f;
    short spriteW = first->w * scale;
    short spriteH = first->h * scale;
    short placeX = gdl::ScreenXres-spriteW;
    short placeY = 0;
    for (short i = 0; i < 4; i++)
    {
        mel_sprites.Put(placeX, placeY, i, gdl::Color::White, 0, 0, scale);
        placeY += spriteH;
    }
}

void Template::DrawTimingInfo(int x, int y, float scale)
{
    
    float ystep = ibmFont.GetHeight()*scale;
    ibmFont.Printf(x+4, y + ystep * 0+4, scale, gdl::Color::Black, "Deltatime %f", deltaTime);
    ibmFont.Printf(x, y + ystep * 0, scale, gdl::Color::LightRed, "Deltatime %f", deltaTime);

    ibmFont.Printf(x+4, y + ystep * 1+4, scale, gdl::Color::Black, "Normalized Deltatime: %f", gdl::Delta);
    ibmFont.Printf(x, y + ystep * 1, scale, gdl::Color::LightRed, "Normalized Deltatime: %f", gdl::Delta);

    ibmFont.Printf(x+4, y + ystep * 2+4, scale, gdl::Color::Black, "Elapsed milliseconds: %f", elapsed);
    ibmFont.Printf(x, y + ystep * 2, scale, gdl::Color::LightRed, "Elapsed milliseconds: %f", elapsed);

    ibmFont.Printf(x+4, y + ystep * 3+4, scale, gdl::Color::Black, "Music elapsed: %f", sampleMusic.GetElapsed());
    ibmFont.Printf(x, y + ystep * 3, scale, gdl::Color::LightRed, "Music elapsed: %f", sampleMusic.GetElapsed());
}

void Template::DrawMenu(int x, int y, int w)
{
    gdl::vec2 cp = gdl::WiiInput::GetCursorPosition();
    menu.StartMenu(x, y, w, cp.x, cp.y, gdl::WiiInput::ButtonPress(WPAD_BUTTON_A));

    menu.Text("Hi! I am menu.");
    menu.Panel(2, gdl::Color::Yellow);
    if (menu.Button("Play Ogg"))
    {
        sampleMusic.PlayMusic(true);
    }
    if (menu.Button("Play Sound"))
    {
        blip.Play(1.0f, 100.0f);
    }
    if (menu.Button("Fire Assert"))
    {
        gdl_assert(false, "Assert button pressed!");
    }
}

glm::vec2 Template::screenToWorld(glm::vec2 p_screen) {
    return p_screen / glm::vec2(worldToScreenScale, -worldToScreenScale) - renderOffset / worldToScreenScale;
}

glm::vec2 Template::worldToScreen(glm::vec2 p_world) {
    return p_world * glm::vec2(worldToScreenScale, -worldToScreenScale) + renderOffset;
}
