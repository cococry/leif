#include "game.hpp"
#include <glad/glad.h>
#include <iostream>
int main(int argc, char* argv[]) {
    Game game(1920, 1080);

    while(game.Running()) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(LF_RGBA(97, 118, 125, 255));
        

        // Render Game 
        game.Render();

        // Render UI 
        lf_div_begin((vec2s){0.0f, 0.0f}, (vec2s){(float)Game::WinWidth, (float)Game::WinHeight});
        game.RenderUI();
        lf_div_end();

        // Flush
        game.Update();
        lf_update();
        glfwPollEvents();
        glfwSwapBuffers(game.GetWin());
    }

    return 0;
} 
