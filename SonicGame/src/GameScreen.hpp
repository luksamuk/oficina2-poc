#pragma once

#include <oficina2/canvas.hpp>
#include <oficina2/render.hpp>
#include <oficina2/entity.hpp>

using namespace oficina;

class PlayerCharacter : public oficina::ofEntity
{
private:
    ofAnimator sprite;
public:
    void init();
    void load();
    void unload();
    void update(float dt);
    void draw(glm::mat4 vp);
};

class GameScreen : public oficina::ofCanvas
{
private:
    PlayerCharacter player;
    glm::vec2       cameraPosition;
public:
    void init();
    void load();
    void unload();
    void update(float dt);
    void draw();
};
