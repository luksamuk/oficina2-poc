#pragma once

#include <oficina2/canvas.hpp>
#include <oficina2/render.hpp>
#include <sstream>

class MenuScreen : public oficina::ofCanvas
{
protected:
    oficina::ofFont font;
    ofdword         sel = 1u,
	            maxsel = 3u,
	            minsel = 1u;
    glm::mat4 mvp;
public:
    void init();
    void load();
    void unload();
    void update(float dt);
    void draw();
};


class MainMenu : public MenuScreen
{
private:
    std::stringstream menuText;
    oficina::ofTexture title,
                       titleblack;
    oficina::ofTextureRenderer titleRend, titleBlackRend;
    float sizestart = 0.02f;
    float transst   = 0.0f;
    glm::mat4 title_model;
    glm::mat4 menu_model;
    glm::vec2 menu_pos;
    ofsbyte   menu_move = 0;
    oficina::ofPrimitive* quad = nullptr;
    ofbyte transstType = 0u;
    std::string menuOptions[6] = {
        "Press Start Button",
        "     New Game    >",
        "<  Level Select  >",
        "<  Level Editor  >",
        "<     Options    >",
        "<      Quit       "
    };
    bool menuSelect = false;
    ofbyte menuTextSelect = 0u;
    ofbyte oldSel = 1u;
public:
    void init();
    void load();
    void unload();
    void update(float dt);
    void draw();
};
