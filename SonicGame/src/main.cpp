#include <oficina2/oficina.hpp>
#include "GameScreen.hpp"
#include "MenuScreen.hpp"

int main(int argc, char** argv)
{
    //oficina::ofLogSetLevel(ofLogWarn);
    oficina::ofInit({
            "wname=OFSONIC THE HEDGEHOG",
            "datad=OFSONIC THE HEDGEHOG",
            "wndsz=720p",
            "frmrt=60c",
            "vsync=on"
            });
    oficina::ofMapDefaultsP1();
    oficina::ofCanvasManager::add(new MainMenu, 1, "MainMenu");
    oficina::ofGameLoop();
    oficina::ofQuit();
	return 0;
}
