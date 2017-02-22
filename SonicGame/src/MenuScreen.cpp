#include "MenuScreen.hpp"
#include <oficina2/input.hpp>
#include <string>
#include "GameScreen.hpp"
using namespace oficina;

void MenuScreen::init()
{
    mvp = glm::ortho(0.0f, 640.0f, -360.0f, 0.0f, 1.0f, 10.0f)
	* glm::lookAt(glm::vec3(0.0f, 0.0f, -1.2f),
		      glm::vec3(0.0f, 0.0f, 0.0f),
		      glm::vec3(0.0f, -1.0f, 0.0f));
}

void MenuScreen::load()
{
    font.init(ofTexturePool::load("res/fonts/levelselect.png"), glm::uvec2(10u), true);
}

void MenuScreen::unload()
{
    font.unload();
}

void MenuScreen::update(float dt)
{
    // Selection management
    if(ofStickMovedTowards(
	   ofStickLeft | ofStickVert | ofStickPositive))
	sel++;
    else
	if(ofStickMovedTowards(
	       ofStickLeft | ofStickVert | ofStickNegative))
	    sel--;
    if(sel < minsel) sel = maxsel;
    else if(sel > maxsel) sel = minsel;
}

void MenuScreen::draw()
{
}


void MainMenu::init()
{
    MenuScreen::init();
    std::string ctrlMove = "Arrow Keys",
	ctrlJmp = "S",
	ctrlSpr = "W",
	ctrlPs  = "Enter";
    
    menuText << "Sonic The Hedgehog Engine v0.1 alpha" << std::endl
	     << "Powered by Oficina Framework v2" << std::endl
	     << "by luksamuk" << std::endl << std::endl
	     << "This engine is proof-of-concept only, " << std::endl
	     << "and shall not be used for commercial purposes." << std::endl << std:: endl
	     << "Controls" << std::endl
	     << "========" << std::endl
	     << ctrlMove << ": Move" << std::endl
	     << ctrlJmp << ": Jump" << std::endl
	     << ctrlSpr << ": Change super state" << std::endl
	     << ctrlPs << ": Pause game" << std::endl
	     << "F1: Cycle debugger state (inactive/watcher/repl)" << std::endl
	     << "F2: Toggle Repl input (Repl visible)" << std::endl
	     << "Enter: Evaluate Repl command (Repl visible)" << std::endl
	     << "Shift + Enter: Input new line (Repl visible)" << std::endl
	     << std::endl << std::endl
	     << "Repl commands" << std::endl
	     << "============" << std::endl
	     << "(setsuper! state): Change super state" << std::endl
	     << "                   (\"state\" must be #t or #f)" << std::endl
	     << "(canvas-list): List currently loaded scenes" << std::endl << std::endl
	     << "* PRESS START *";
}

void MainMenu::load()
{
    MenuScreen::load();
}

void MainMenu::unload()
{
    MenuScreen::unload();
}

void MainMenu::update(float dt)
{
    MenuScreen::update(dt);
    if(ofButtonTap(ofPadStart)) {
        remove();
        ofCanvasManager::add(new GameScreen, 0, "Sonic Engine");
    }
}

void MainMenu::draw()
{
    glm::vec2 raster(100.0f, 50.0f);
    font.write(menuText.str(), raster, mvp);
}
