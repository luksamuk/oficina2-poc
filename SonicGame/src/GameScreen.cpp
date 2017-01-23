#include "GameScreen.hpp"
#include <oficina2/oficina.hpp>
#include <oficina2/io.hpp>
#include <oficina2/input.hpp>
#include <oficina2/benchmark.hpp>
#include <oficina2/ofscheme.hpp>
using namespace oficina;

glm::mat4 vp;

ofAnimator* anim;
bool m_super = false,
     m_oldsuper = false;

void PlayerCharacter::init() {
    script.init("GameCharacter", this);
    anim = &sprite;
}

void PlayerCharacter::load() {
    sonic = ofTexturePool::load("res/sonic.png");
    super = ofTexturePool::load("res/supersonic.png");
    sprite.init(sonic, glm::uvec2(60, 60), false);
    sprite.setPosition(glm::vec2(0.0f, -16.0f));

    ofdword stopped[]  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    			   0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 3, 4, 4 };
    ofdword walking[]  = { 5, 6, 7, 8, 9, 10 };
    ofdword running[]  = { 11, 12, 13, 14 };
    ofdword rolling[]  = { 15, 16, 17, 16, 19, 16, 21, 16 };
    ofdword skidding[] = { 23 };
    ofdword peelout[]  = { 24, 25, 26, 27 };
    ofdword pushing[]  = { 28, 29, 30, 31 };
    ofdword crouch[]   = { 32 };
    ofdword lookup[]   = { 33 };
    ofdword death[]    = { 34 };

    sprite.reg("Idle", 30, stopped, 8.0f, true, 26);
    sprite.reg("Walk",  6, walking, 16.0f, true);
    sprite.reg("Run",   4, running, 4.0f, true);
    sprite.reg("Roll",  8, rolling, 8.0f, true);
    sprite.reg("Skid",  1, skidding, 1.0f);
    sprite.reg("Peel",  4, peelout, 2.0f, true);
    sprite.reg("Push",  4, pushing, 16.0f, true);
    sprite.reg("Crouch", 1, crouch, 1.0f);
    sprite.reg("LookUp", 1, lookup, 1.0f);
    sprite.reg("Dead", 1, death, 1.0f);

    sprite.SetAnimation("Idle");
    //sprite.SetAnimation("Walk");
    //sprite.SetAnimation("Run");
    

    script.regFunc("animation-set!",
            [] (scheme* scm, pointer args) -> pointer
            {
                if(args != scm->NIL)
                {
                    if(scm->vptr->is_string(pair_car(args)))
                    {
                        std::string animName = scm->vptr->string_value(pair_car(args));
                        anim->SetAnimation(animName);
                    }
                    else ofLog(ofLogErr, OFLOG_CYN "SCM: " OFLOG_RESET
                            "Animation name must be string.\n");
                }
                return scm->NIL;
            });

    script.regFunc("animation-setspd!",
            [] (scheme* scm, pointer args) -> pointer
            {
                if(args != scm->NIL)
                {
                	pointer p = scheme_eval(scm, pair_car(args));
                    if(scm->vptr->is_number(p))
                    {
                        float animspd = scm->vptr->rvalue(pair_car(args));
                        anim->SetAnimationSpeed(animspd);
                    }
                    else ofLog(ofLogErr, OFLOG_CYN "SCM: " OFLOG_RESET
                            "Animation speed must be number.\n");
                }
                return scm->NIL;
            });

    script.regFunc("animation-spd?",
            [] (scheme* scm, pointer args) -> pointer
            {
            	return scm->vptr->mk_real(scm, anim->GetAnimationSpeed());
            });

    script.regFunc("animation-defspd?",
            [] (scheme* scm, pointer args) -> pointer
            {
            	return scm->vptr->mk_real(scm, anim->GetDefaultAnimationSpeed());
            });

    script.regFunc("animation-setrunning!",
		   [] (scheme* scm, pointer args) -> pointer
		   {
		       if(args != scm->NIL)
		       {
			   if(pair_car(args) == scm->T || pair_car(args) == scm->F)
			   {
			       anim->SetAnimationRunning((pair_car(args) == scm->T));
			   }
			   else ofLog(ofLogErr, OFLOG_CYN "SCM: " OFLOG_RESET
				      "Animation running state must be #t or #f.\n");
		       }
		       else ofLog(ofLogErr, OFLOG_CYN "SCM: " OFLOG_RESET
				  "Must provide arguments for running animation\n");
		       return scm->NIL;
		   });

    script.regFunc("setsuper!",
		   [] (scheme* scm, pointer args) -> pointer
		   {
		       if(args != scm->NIL)
		       {
			   if(pair_car(args) == scm->T || pair_car(args) == scm->F)
			   {
			       m_super = (pair_car(args) == scm->T);
			   }
			   else ofLog(ofLogErr, OFLOG_CYN "SCM: " OFLOG_RESET
				      "Super state must be #t or #f.\n");
		       }
		       else ofLog(ofLogErr, OFLOG_CYN "SCM: " OFLOG_RESET
				  "Must provide arguments for super state\n");
		       return scm->NIL;
		   });

    script.load("res/GameCharacter.scm");
}

void PlayerCharacter::unload() {
    sprite.unload();
    ofTexturePool::unload(sonic);
    ofTexturePool::unload(super);
    script.unload();
}

void PlayerCharacter::update(float dt) {
    //sprite.setPosition(glm::vec2(position.x, position.y));
    sprite.update(dt);
    script.update(dt);

    if(m_super != m_oldsuper) {
	m_oldsuper = m_super;
	sprite.SetAnimationTexture(m_super ? super : sonic);
	ofLog(ofLogInfo, "Changing to %s...\n", m_super ? "Super Sonic" : "Sonic");
    }

    if(ofButtonTap(ofPadBack))
    	script.load("res/GameCharacter.scm");
}

void PlayerCharacter::draw(glm::mat4 vp) {
    sprite.draw(vp * getModelMatrix(), 1.0f);
}








void GameScreen::init() {
    ofSetVSync(true);
    glm::vec2 vwprt = glm::vec2(640.0f, 360.0f);
    cameraPosition = glm::vec2(0.0f, 0.0f);
    vp  = glm::ortho(0.0f, vwprt.x, -vwprt.y, 0.0f, 1.0f, 10.0f)
            * glm::lookAt(glm::vec3(cameraPosition.x, cameraPosition.y, -1.2f),
                          glm::vec3(cameraPosition.x, cameraPosition.y, 0.0f),
                          glm::vec3(0.0f, -1.0f, 0.0f));

    player.init();
}

void GameScreen::load() {
    player.load();
    tile  = ofTexturePool::load("res/tile.png");
    tilerender.init(tile);
    //ofBenchmarkStart(2.0f);
}

void GameScreen::unload() {
    player.unload();
    tilerender.unload();
    ofTexturePool::unload(tile);
    //ofBenchmarkEnd();
}

void GameScreen::update(float dt) {
	glm::vec2 vwprt = glm::vec2(640.0f, 360.0f);

	if(player.getPosition().x >= 320.0f)
		cameraPosition.x = player.getPosition().x - 320.0f;
	else cameraPosition.x = 0.0f;

    vp  = glm::ortho(0.0f, vwprt.x, -vwprt.y, 0.0f, 1.0f, 10.0f)
            * glm::lookAt(glm::vec3(cameraPosition.x, cameraPosition.y, -1.2f),
                          glm::vec3(cameraPosition.x, cameraPosition.y, 0.0f),
                          glm::vec3(0.0f, -1.0f, 0.0f));


    player.update(dt);
    //ofBenchmarkUpdateCall();
}

void GameScreen::draw() {
    glm::vec2 pos = glm::vec2(64.0f, 308.0f);
    for(int i = 0; i < 10; i++) {
	tilerender.render(pos, vp);
	pos.x += 128.0f;
    }
    pos.y -= 128.0f;
    tilerender.render(pos, vp);
    
    player.draw(vp);
}
