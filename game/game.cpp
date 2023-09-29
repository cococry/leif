#include "game.hpp"
#include <GLFW/glfw3.h>
#include <cglm/types-struct.h>
#include <iostream>
#include <glad/glad.h>
#include <functional>

uint32_t Game::WinWidth = 0;
uint32_t Game::WinHeight = 0;
RandomNumberGenerator Game::RNG;


PlayerData Game::Player{};

Collectable::Collectable(uint32_t scale) {
    Scale = scale;
    Pos.x = (float)Game::RNG.RandInt(0, Game::WinWidth - scale);
    Pos.y = (float)Game::RNG.RandInt(-900, -300);
    Type = (CollectableType)Game::RNG.RandInt(CollectableTypeProtein, CollectableTypeMax - 1);
    switch(Type) {
        case CollectableTypeProtein:
            Speed = 5.0f;
            break;
        case CollectableTypeVirus:
            Speed = 7.0f;
            break;
        default:
            Speed = 5.0f;
            break;
    }
}
void WinResizeCb(GLFWwindow* window, int32_t width, int32_t height) {
    lf_resize_display(width, height);
    glViewport(0, 0, width, height);
    Game::WinWidth = width;
    Game::WinHeight = height;
}

Game::Game(uint32_t winWidth, uint32_t winHeight) 
    : mDeltaTime(0.0f), mLastTime(0.0f) {
    WinWidth = winWidth;
    WinHeight = winHeight;
    
    // GLFW/Glad Initialization
    {
        if(!glfwInit()) {
            std::cout << "Failed to initialize GLFW.\n";
        }

        mWin = glfwCreateWindow(WinWidth, WinHeight, "Cell" ,NULL, NULL);
        if(!mWin) {
            std::cout << "Failed to create GLFW window.\n";
        }
        glfwMakeContextCurrent(mWin);
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize Glad.\n";
        }
    }
    // Leif Initialization
    {
        LfTheme theme = lf_default_theme("../game/fonts/slkscr.ttf", 48);
        theme.div_props.color = (vec4s){LF_RGBA(0, 0, 0, 0)};
        lf_init_glfw(WinWidth, WinHeight, "../game/fonts/slkscr.ttf", &theme, mWin);   
        lf_set_text_wrap(true);
        glfwSetFramebufferSizeCallback(mWin, WinResizeCb);
        glViewport(0, 0, WinWidth, WinHeight);
    }
    // Game Initialization
    {
        // Textures
        mCellTex = lf_tex_create("../game/textures/cell.png", false, LF_TEX_FILTER_NEAREST);
        mProteinTex = lf_tex_create("../game/textures/protein.png", false, LF_TEX_FILTER_NEAREST);
        mBgTex = lf_tex_create("../game/textures/wallpaper.png", false, LF_TEX_FILTER_NEAREST);
        mVirusTex = lf_tex_create("../game/textures/virus.png", false, LF_TEX_FILTER_NEAREST);
        mHealthTex = lf_tex_create("../game/textures/health.png", false, LF_TEX_FILTER_NEAREST);

        // Player 
        Player.Speed = 5.0f;
        Player.Scale = 100;
        Player.Margin = 20;
        Player.Width = Player.Scale;
        Player.Pos = (vec2s){(WinWidth - Player.Scale) / 2.0f, (float)(Game::WinHeight - Game::Player.Scale - Game::Player.Margin)};
        Player.CellCount = 1;

        mState.SpawnRate = 2.0f;
        mState.SpawnTimer = 0.0f;
        mState.ProteinCount = 0;

        mButtonFont = lf_load_font("../game/fonts/poppins.ttf", 28);
    }
}

Game::~Game() {
    lf_terminate();
    glfwDestroyWindow(mWin);
    glfwTerminate();
}

void Game::Render() {
    lf_image_render((vec2s){0, 0}, (vec4s){LF_WHITE}, (LfTexture){.id = mBgTex.id, .width = WinWidth, .height = WinHeight});
    RenderPlayer(); 
    RenderCollectables();
}
void Game::RenderUI() {
   lf_image((LfTexture){.id = mProteinTex.id, .width = 32, .height = 32});
   lf_text("%i", mState.ProteinCount);
   lf_image((LfTexture){.id = mHealthTex.id, .width = 32, .height = 32});
   lf_text("%i", Player.CellCount);

   lf_push_font(&mButtonFont);
   lf_next_line();
   if(lf_button("Upgrade Body") == LF_CLICKED && mState.ProteinCount >= 3) {
        mState.ProteinCount -= 3;
        Player.CellCount++;
        Player.Width += 20 + Player.Scale;
   } 
   lf_pop_font();
}
void Game::Update() {
    float currentTime = glfwGetTime();
    mDeltaTime = currentTime - mLastTime;
    mLastTime = currentTime;

    UpdatePlayer();
    UpdateCollectables();
}

void Game::RenderPlayer() {
    float x = Player.Pos.x;
    for(uint32_t i = 0; i < Player.CellCount; i++) {
        lf_image_render((vec2s){x, Player.Pos.y}, (vec4s){LF_WHITE}, (LfTexture){.id = mCellTex.id, .width = Player.Scale, .height = Player.Scale}); 
        x += Player.Scale + 20;
    }
}

void Game::UpdatePlayer() {
    if(lf_key_is_down(GLFW_KEY_A) && Player.Pos.x > 0) {
        Player.Pos.x -= Player.Speed;
    }
    if(lf_key_is_down(GLFW_KEY_D) && Player.Pos.x <= WinWidth - Player.Width) {
        Player.Pos.x += Player.Speed;
    }
}
void Game::RenderCollectables() {
    for(auto& collectable : mCollectables) {
        uint32_t texture;
        switch(collectable.Type) {
            case CollectableTypeProtein:
                texture = mProteinTex.id;
                break;
            case CollectableTypeVirus:
                texture = mVirusTex.id;
                break;
            default:
                texture = 0;
                break;
        }
        lf_image_render(collectable.Pos, (vec4s){LF_WHITE}, (LfTexture){.id = texture, .width = collectable.Scale, .height = collectable.Scale});
    }
}
void Game::UpdateCollectables() {
    mState.SpawnTimer += mDeltaTime;
    if(mState.SpawnTimer >= mState.SpawnRate) {
        mState.SpawnTimer = 0.0f;
        mCollectables.push_back(Collectable());
    }
    for(auto it = mCollectables.begin(); it != mCollectables.end();) {
        bool erased = false;
        it->Pos.y += it->Speed;

        if(it->Pos.y >= WinHeight) {
            it = mCollectables.erase(it);
            erased = true;
        }
        if(lf_aabb_intersects_aabb( 
                                    (LfAABB){.pos = it->Pos, .size = (vec2s){.x = (float)it->Scale, .y = (float)it->Scale}},
                                    (LfAABB){.pos = Player.Pos, .size = (vec2s){.x = Player.Width, .y = (float)Player.Scale}})) {
            it = mCollectables.erase(it);
            mState.ProteinCount++;
            erased = true; 
        }
        if(!erased) {
            it++;
        }
    }
}
