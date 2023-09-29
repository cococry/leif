#pragma once

#include "../leif.h"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <vector>
#include <random>
#include <iostream>

class RandomNumberGenerator {
public:
    RandomNumberGenerator() : random_device(), random_engine(random_device()) 
    {
        std::cout << "Init random.\n";
    }

    int RandInt(int min, int max) {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(random_engine);
    }

private:
    std::random_device random_device;
    std::mt19937 random_engine;
};

struct PlayerData {
    float Speed;
    vec2s Pos;
    uint32_t Scale;
    uint32_t Margin;
    uint32_t CellCount;
    float Width;
};

enum CollectableType {
    CollectableTypeProtein = 0,
    CollectableTypeVirus,
    CollectableTypeMax
};
struct Collectable {
   CollectableType Type;
   vec2s Pos;
   float Speed;
   uint32_t Scale;
   Collectable(uint32_t scale = 50);
};

struct GameState {
    uint32_t ProteinCount;
    float SpawnTimer, SpawnRate;
};

class Game {
    public:
        Game(uint32_t winWidth, uint32_t winHeight);
        ~Game();

        void Render(); 
        void RenderUI();
        void Update(); 
        
        static uint32_t WinWidth;
        static uint32_t WinHeight;

        static PlayerData Player;

        inline GLFWwindow* GetWin() const {
            return mWin;
        }
        inline bool Running() const {
            return !glfwWindowShouldClose(mWin);
        }
        static RandomNumberGenerator RNG;
    private:
        void RenderPlayer();
        void UpdatePlayer();

        void RenderCollectables();
        void UpdateCollectables();

        LfTexture mCellTex{};
        LfTexture mProteinTex{};
        LfTexture mBgTex{};
        LfTexture mVirusTex{};
        LfTexture mHealthTex{};
    
        GLFWwindow* mWin;

        float mDeltaTime, mLastTime;

        GameState mState;
        
        LfFont mButtonFont;

        std::vector<Collectable> mCollectables;

};
