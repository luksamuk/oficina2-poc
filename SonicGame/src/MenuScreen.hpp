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
public:
    void init();
    void load();
    void unload();
    void update(float dt);
    void draw();
};
