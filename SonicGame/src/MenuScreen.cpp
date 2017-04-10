#include "MenuScreen.hpp"
#include <oficina2/oficina.hpp>
#include <oficina2/input.hpp>
#include <oficina2/benchmark.hpp>
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
    if(sel < minsel) sel = minsel;
    else if(sel > maxsel) sel = maxsel;
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
	     << "Powered by Oficina Framework" << std::endl
         << "v" OF_VERSION_STRING << std::endl
	     << "by luksamuk" << std::endl << std::endl
	     << "This engine is proof-of-concept only, " << std::endl
	     << "and shall not be used for commercial purposes." << std::endl << std:: endl
	     << "* PRESS START *" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl
         << "BLABLABLABLABLABLABLABLABLABLABLA" << std::endl;
    ofBenchmarkStart(2.0f);
    maxsel = 5;
}

void MainMenu::load()
{
    MenuScreen::load();
    title = ofTexturePool::load("res/levels/title/title.png");
    titleblack = ofTexturePool::load("res/levels/title/title_black.png");
    titleRend.init(title);
    titleBlackRend.init(titleblack);

    float quadVerts[] = {
        0.0f,     0.0f, 0.0f,
        640.0f,   0.0f, 0.0f,
        640.0f, 360.0f, 0.0f,
        0.0f,   360.0f, 0.0f
    };

    quad = ofPrimitiveRenderer::makePrimitive(ofQuad, 4, sizeof(quadVerts), quadVerts);
}

void MainMenu::unload()
{
    MenuScreen::unload();
    delete quad;
    titleRend.unload();
    titleBlackRend.unload();
    ofTexturePool::unload(title);
    ofTexturePool::unload(titleblack);
    ofBenchmarkEnd();
}

void MainMenu::update(float dt)
{
    // Beginning and option select
    if(transstType == 0u) {
        if(sizestart < 1.0f)
            sizestart += 0.02f;
        else {
            sizestart = 1.0f;
            if(transst < 1.0f)
                transst += 0.09f;
            else transst = 1.0f;
        }

        if(transst >= 1.0f) {
            if(!menuSelect) {
                if(ofButtonTap(ofPadStart)) {
                    menuTextSelect = sel;
                    menuSelect = true;
                }
            }
            else
            {
                if(ofButtonTap(ofPadStart)) {
                    transstType = 1u;
                }
                // VERY VERY ROUGH
                if(ofStickMovedTowards(ofStickLeft | ofStickHoriz | ofStickPositive))
                    sel++;
                else if(ofStickMovedTowards(ofStickLeft | ofStickHoriz | ofStickNegative))
                    sel--;
                menuTextSelect = ((sel > 0) && (sel < 6)) ? sel : menuTextSelect;

                if(sel != oldSel) {
                    menu_move = (sel < oldSel) ? -1 : 1;
                    menu_pos.x += 30.0f * menu_move;
                    oldSel = sel;
                }

                if(menu_move)
                {
                    menu_pos.x += 0.03f * menu_move;
                    if(menu_move == -1)
                    {
                        if(menu_pos.x <= -510.0f)
                            menu_pos.x = 510.0f;
                        //if(menu_pos.x < )
                    }
                    else if(menu_move == 1)
                    {
                        if(menu_pos.x >= 510.0f)
                            menu_pos.x = -510.0f;
                    }
                }
            }
        }
    }
    else if(transstType == 1u) {
        transst -= 0.09f;
        if(transst < 0.0f) {
            transst = 0.0f;
            switch(sel) {
            case 1u: // New Game
                remove();
                ofCanvasManager::add(new GameScreen, 0, "Sonic Engine");
                break;
            case 2u: // Level Select
                remove();
                break;
            case 3u: // Level Editor
                remove();
                break;
            case 4u: // Options
                remove();
                break;
            case 5u: ofSoftStop(); break;
            };
        }
    }

    title_model = glm::translate(glm::mat4(), glm::vec3(-0.03f, 0.0f, 0.0f)) *
                    glm::scale(glm::mat4(),
                    glm::vec3(sizestart * 0.8f, sizestart * 0.8f, 1.0f));

    ofBenchmarkUpdateCall();
    MenuScreen::update(dt);
}

void MainMenu::draw()
{
    glm::vec2 titlepos(640.0f, 360.0f);
    titlepos /= 2.0f;
    if(transstType == 0u)
        ofPrimitiveRenderer::draw(quad, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - transst), mvp);
    titleRend.render(titlepos, title_model * mvp, 0u, glm::vec4(1.0f, 1.0f, 1.0f, transst));
    titleBlackRend.render(titlepos, title_model * mvp, 0u, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f - transst));

    if(sizestart >= 1.0f) {
        glm::vec2 raster(640.0f, 360.0f);
        raster /= 2.0f;
        raster.x -= font.measure(menuOptions[menuTextSelect]).x / 2.0f;
        raster.y += 100.0f;
        font.write(menuOptions[menuTextSelect], raster, mvp, glm::vec4(glm::vec3(1.0f), transst));
    }

    font.write("Powered by Oficina Framework v" OF_VERSION_STRING,
                glm::vec2(-310.0f, 540.0f - font.getGlyphSize().y),
                glm::scale(glm::mat4(), glm::vec3(0.5f, 0.5f, 1.0f)) * mvp,
                glm::vec4(glm::vec3(1.0f), 0.5f));

    if(transstType == 1u)
        ofPrimitiveRenderer::draw(quad, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f - transst), mvp);
}
