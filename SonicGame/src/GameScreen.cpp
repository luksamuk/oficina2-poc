#include "GameScreen.hpp"
#include <oficina2/oficina.hpp>
#include <oficina2/io.hpp>
#include <oficina2/input.hpp>
#include <oficina2/benchmark.hpp>
using namespace oficina;

glm::mat4 vp, projection, view, parallax_vp;

ofAnimator* anim;
bool m_super = false,
     m_oldsuper = false;

void PlayerCharacter::init() {
    script = new ofScheme;
    AddComponent("script", script);
    anim = &sprite;
}

void PlayerCharacter::load() {
    sonic = ofTexturePool::load("res/sprites/sonic.png");
    super = ofTexturePool::load("res/sprites/supersonic.png");
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
    sprite.reg("Push",  4, pushing, 32.0f, true);
    sprite.reg("Crouch", 1, crouch, 1.0f);
    sprite.reg("LookUp", 1, lookup, 1.0f);
    sprite.reg("Dead", 1, death, 1.0f);
    
    sprite.SetAnimation("Idle");
    //sprite.SetAnimation("Walk");
    //sprite.SetAnimation("Run");
    

    script->regFunc("animation-set!",
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

    script->regFunc("animation-setspd!",
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

    script->regFunc("animation-spd?",
		   [] (scheme* scm, pointer args) -> pointer
		   {
		       return scm->vptr->mk_real(scm, anim->GetAnimationSpeed());
		   });

    script->regFunc("animation-defspd?",
		   [] (scheme* scm, pointer args) -> pointer
		   {
		       return scm->vptr->mk_real(scm, anim->GetDefaultAnimationSpeed());
		   });

    script->regFunc("animation-setrunning!",
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

    auto superfunc =
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
	};

    auto issuper =
	[] (scheme* scm, pointer args) -> pointer
	{
	    return (m_super ? scm->T : scm->F);
	};
    
    script->regFunc("setsuper!", superfunc);
    ofScmDefineFunc("setsuper!", superfunc);

    script->regFunc("super?", issuper);
    ofScmDefineFunc("super?", issuper);

    script->loadfile("res/scripts/player.scm");
}

void PlayerCharacter::unload() {
    ClearComponents();
    sprite.unload();
    ofTexturePool::unload(sonic);
    ofTexturePool::unload(super);
    ofScmUndefineFunc("setsuper!");
    ofScmUndefineFunc("super?");
}

void PlayerCharacter::update(float dt) {
    //sprite.setPosition(glm::vec2(position.x, position.y));
    sprite.update(dt);

    if(m_super != m_oldsuper) {
	m_oldsuper = m_super;
	sprite.SetAnimationTexture(m_super ? super : sonic);
	ofLog(ofLogInfo, "Changing to %s...\n", m_super ? "Super Sonic" : "Sonic");
    }

    if(ofButtonTap(ofPadBack))
    	script->loadfile("res/GameCharacter.scm");
    UpdateComponents(dt);
}

void PlayerCharacter::draw(glm::mat4 vp) {
    sprite.draw(vp * getModelMatrix(), 1.0f);
    DrawComponents();
}











void GameScreen::init() {
    ofSetVSync(true);
    glm::vec2 vwprt = glm::vec2(640.0f, 360.0f);
    cameraPosition = glm::vec2(0.0f, 16.0f);
    projection = glm::ortho(0.0f, vwprt.x, -vwprt.y, 0.0f, 1.0f, 10.0f);
    view = glm::lookAt(glm::vec3(cameraPosition.x, cameraPosition.y, -1.2f),
		       glm::vec3(cameraPosition.x, cameraPosition.y, 0.0f),
		       glm::vec3(0.0f, -1.0f, 0.0f));
    vp = projection * view;
    parallax_vp = projection * view;

    player.init();
}

void GameScreen::load() {
    player.load();
    //tile  = ofTexturePool::load("res/tile.png");
    tile = ofTexturePool::load("res/levels/zone1/tiles.png");
    tilerender.init(tile, glm::uvec2(130));
    //ofBenchmarkStart(2.0f);
}

void GameScreen::unload() {
    player.unload();
    tilerender.unload();
    ofTexturePool::unload(tile);
    //ofBenchmarkEnd();
}

void GameScreen::update(float dt) {
    if(player.getPosition().x >= 320.0f)
	cameraPosition.x = player.getPosition().x - 320.0f;
    else cameraPosition.x = 0.0f;

    glm::vec2 cameraAdd = ofGetRightStick() * 64.0f;

    glm::vec2 cameraFinal = cameraPosition + cameraAdd;

    view =  glm::lookAt(glm::vec3(cameraFinal.x, cameraFinal.y, -1.2f),
			glm::vec3(cameraFinal.x, cameraFinal.y, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f));
    vp = projection * view;

    player.update(dt);
    //ofBenchmarkUpdateCall();
}

void GameScreen::draw() {
    glm::vec2 pos = glm::vec2(64.0f, 192.0f + 65.0f);
    glm::vec2 parpos = pos;
    parpos.y = 65.0f;

    // Parallax
    // Layer 0
    for(int i = 0; i < 2; i++) {
	tilerender.render(parpos,                                 parallax_vp, 158);
	tilerender.render(glm::vec2(parpos.x + 128.0f, parpos.y), parallax_vp, 159);
	tilerender.render(glm::vec2(parpos.x + 256.0f, parpos.y), parallax_vp, 167);
	tilerender.render(glm::vec2(parpos.x + 384.0f, parpos.y), parallax_vp, 168);
	parpos.x += 512.0f;
    }
    parpos.x = pos.x;
    parpos.y += 128.0f;
    // Layer 1
    for(int i = 0; i < 3; i++) {
	tilerender.render(parpos,                                 parallax_vp, 112);
	tilerender.render(glm::vec2(parpos.x + 128.0f, parpos.y), parallax_vp, 160);
	parpos.x += 256.0f;
    }
    parpos.x = pos.x;
    parpos.y += 128.0f;
    // Layer 2
    for(int i = 0; i < 8; i++) {
	tilerender.render(parpos, parallax_vp, 161);
	parpos.x += 128.0f;
    }
    parpos.x = pos.x;
    parpos.y += 128.0f;
    // Layer 3
    for(int i = 0; i < 2; i++) {
	tilerender.render(parpos,                                 parallax_vp, 162);
	tilerender.render(glm::vec2(parpos.x + 128.0f, parpos.y), parallax_vp, 163);
	tilerender.render(glm::vec2(parpos.x + 256.0f, parpos.y), parallax_vp, 164);
	tilerender.render(glm::vec2(parpos.x + 384.0f, parpos.y), parallax_vp, 166);
	parpos.x += 512.0f;
    }

    
    
    // Level
    for(int i = 0; i < 30; i++) {
	   tilerender.render(pos, vp, ((i % 2) ? 37 : 170));

       if(i < 2) {
            tilerender.render(glm::vec2(pos.x, pos.y - 128.0f), vp, (i == 0) ? 256 : 257);
       }
       else
       {
            tilerender.render(glm::vec2(pos.x, pos.y - 128.0f), vp, (i % 2) ? 105 : 104);
            if(!(i % 2)) tilerender.render(glm::vec2(pos.x, pos.y - 256.0f), vp, 102);
       }
       tilerender.render(glm::vec2(pos.x, pos.y + 128.0f), vp, 151 + (i % 3));
       pos.x += 128.0f;
    }
    pos.y += 128.0f;
    tilerender.render(pos, vp, 171);
    pos.y -= 128.0f;
    tilerender.render(pos, vp, 50);
    pos.y -= 128.0f;
    tilerender.render(pos, vp, 61);
    pos.y -= 128.0f;
    tilerender.render(pos, vp, 65);

    player.draw(vp);
}
