#include <oficina2/oficina.hpp>
#include "GameScreen.hpp"

int main(int argc, char** argv)
{
    oficina::ofInit();
    //oficina::ofSetWindowSize(1280, 720);
    //oficina::ofSetViewportSize(1280, 720);
    oficina::ofMapDefaultsP1();
    oficina::ofCanvasManager::dbg_ChangeState();
    oficina::ofCanvasManager::add(new GameScreen, 0, "Level");
    oficina::ofGameLoop();
    oficina::ofQuit();
	return 0;
}
