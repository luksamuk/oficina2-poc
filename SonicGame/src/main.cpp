#include <oficina2/oficina.hpp>
#include "GameScreen.hpp"
#include "MenuScreen.hpp"

int main(int argc, char** argv)
{
    //oficina::ofLogSetLevel(ofLogWarn);
    oficina::ofInit();
    //oficina::ofSetWindowSize(1280, 720);
    //oficina::ofSetViewportSize(1280, 720);
    oficina::ofMapDefaultsP1();
    //oficina::ofCanvasManager::dbg_ChangeState();
    //oficina::ofCanvasManager::add(new GameScreen, 0, "Level");
    oficina::ofCanvasManager::add(new MainMenu, 1, "MainMenu");
    oficina::ofGameLoop();
    oficina::ofQuit();
	return 0;
}
