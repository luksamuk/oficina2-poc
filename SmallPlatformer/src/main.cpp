#include <oficina2/oficina.hpp>
#include <oficina2/entity.hpp>
using namespace oficina;

// Properties:
// 0. IsPlayer
// 1. IsSolid
// 2. IsJumpThru

class Solid : public ofEntity
{
private:
	static ofPrimitive* shape;
	static ofqword      shapeCount;

	ofAABB* AABB;
public:
	Solid(glm::vec2 position, bool isJumpThru = false, glm::vec2 size = glm::vec2(64.0f)) {
		scale(glm::vec3(size, 1.0f), true);
		translate(glm::vec3(position, 0.0f), true);
		setProperty(2, isJumpThru);
		setName(isJumpThru ? "Platform" : "Ground");
		AABB = new ofAABB(glm::vec3(size / 2.0f, 0.0f), glm::vec3(size / 2.0f, 0.5f));
		AABB->setVisibility(true);
		AddComponent("AABB", AABB);
	}
	
	void init() {
		setProperty(0, false);
		setProperty(1, true);
	}

	void load() {
		if(!shapeCount) {
			float theShape[] = {
				0.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f
			};
			shape = ofPrimitiveRenderer::makePrimitive(ofTriangleFan,
													   4, sizeof(theShape),
													   theShape);
			shapeCount = 1u;
		} else shapeCount++;
	}

	void unload() {
		shapeCount--;
		if(!shapeCount)
			delete shape;
		ClearComponents();
	}

	void update(float dt) {
		UpdateComponents(dt);
	}

	void draw(glm::mat4 mvp) {
		ofPrimitiveRenderer::draw(shape,
								  getProperty(2)
								  ? glm::vec4(0.0f, 0.4f, 0.01f, 1.0f)
								  : glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
								  mvp * getModelMatrix());
		DrawComponents(mvp);
	}
};

ofqword      Solid::shapeCount = 0u;
ofPrimitive* Solid::shape;

class Player : public ofEntity
{
private:
	// Handle for the animator component
    ofAnimator* animator;
	ofTexture   myTexture;

	// Some constants
	float hitboxRadX;
	float hitboxRadY;
	
	// Handle for script
	#ifdef LUA_MODE
	ofLua*      script;
	#else
	ofScheme*    script;
	#endif
	
	// Sensors
	ofAABB*           masterSensor;
	ofBoundingSphere* bottomSensor;
	ofBoundingSphere* bottomLSensor;
	ofBoundingSphere* bottomRSensor;
	ofBoundingSphere* ledgeLSensor;
	ofBoundingSphere* ledgeRSensor;
	ofBoundingSphere* leftSensor;
	ofBoundingSphere* rightSensor;
	ofBoundingSphere* topSensor;
public:
	void init() {
		hitboxRadX  = 16.0f;
		hitboxRadY  = 32.0f;
	
		setProperty(0, true);
		setProperty(1, false);
		setProperty(2, false);
		translate(glm::vec3(128.0f, 128.0f, 0.0f), true);
		setName("Player");
	}

	void load() {
		myTexture = ofTexturePool::load("res/smileman.png");
		ofTextureRenderer renderer;
		renderer.init(myTexture, glm::uvec2(64));
		animator = new ofAnimator;
		animator->SetRenderer(renderer, true);
		AddComponent("animator", animator);
		{
			ofdword stopped[] = {0};
			ofdword walking[] = {0, 1, 0, 2};
			ofdword jumping[] = {2};
			animator->reg("stopped", 1, stopped, 1.0f, true);
			animator->reg("walking", 4, walking, 6.0f, true);
			animator->reg("jumping", 1, jumping, 1.0f, true);
		}
		animator->SetAnimation("stopped");

		// Add sensors
		masterSensor  =
			new ofAABB(glm::vec3(), glm::vec3(hitboxRadX, hitboxRadY, 0.5f));
		bottomSensor  =
			new ofBoundingSphere(glm::vec3(0.0f, hitboxRadY, 0.0f), 5.0f);
		bottomLSensor =
			new ofBoundingSphere(glm::vec3(-hitboxRadX + 8.0f, hitboxRadY, 0.0f), 5.0f);
		bottomRSensor =
			new ofBoundingSphere(glm::vec3(hitboxRadX - 8.0f, hitboxRadY, 0.0f), 5.0f);
		ledgeLSensor  =
			new ofBoundingSphere(glm::vec3(-hitboxRadX + 5.0f, hitboxRadY, 0.0f), 5.0f);
		ledgeRSensor  =
			new ofBoundingSphere(glm::vec3(hitboxRadX - 5.0f, hitboxRadY, 0.0f), 5.0f);
		leftSensor    = new ofBoundingSphere(glm::vec3(-hitboxRadX, 0.0f, 0.0f), 5.0f);
		rightSensor   = new ofBoundingSphere(glm::vec3(hitboxRadX, 0.0f, 0.0f), 5.0f);
		topSensor     = new ofBoundingSphere(glm::vec3(0.0f, -hitboxRadY, 0.0f), 5.0f);

		masterSensor->setVisibility(true);
		//bottomSensor->setVisibility(true);
		
		AddComponent("MasterSensor", masterSensor);
		AddComponent("BottomSensor", bottomSensor);
		AddComponent("BottomLSensor", bottomLSensor);
		AddComponent("BottomRSensor", bottomRSensor);
		AddComponent("LedgeLSensor", ledgeLSensor);
		AddComponent("LedgeRSensor", ledgeRSensor);
		AddComponent("LeftSensor",   leftSensor);
		AddComponent("RightSensor",  rightSensor);
		AddComponent("TopSensor",    topSensor);

		// Fine description of object behaviour is all
		// on this script file

		// Lua
		#ifdef LUA_MODE
		script = new ofLua();
		AddComponent("Lua", script);
		script->loadfile("res/scripts/Player.lua");
		ofLuaDefineSymbol("player", this);
		#else
		// Scheme
		script = new ofScheme();
		AddComponent("Scheme", script);
		script->loadfile("SmallPlatformer Player", "res/scripts/Player.scm");
		ofScmDefineSymbol("*player*", this);
		#endif
	}

	void unload() {
		#ifdef LUA_MODE
		ofLuaUndefine("player");
		#else
		ofScmUndefine("*player*");
		#endif
		ClearComponents();
		ofTexturePool::unload(myTexture);
	}

	void update(float dt) {
		UpdateComponents(dt);

		if(ofButtonTap(ofPadY))
			script->reload();
	}

	void draw(glm::mat4 mvp) {
		DrawComponents(mvp);
	}
};

class Game : public ofCanvas
{
private:
	ofIEntityManager* manager;
	glm::mat4         projection;
	glm::mat4         view;
	glm::mat4         mvp;
	glm::vec2         camPos;

	Player*           player;
public:
	void init() {
		projection = glm::ortho(0.0f, 640.0f, -360.0f, 0.0f, -100.0f, 100.0f);
		view = glm::lookAt(glm::vec3(camPos.x, camPos.y, -1.2f),
						   glm::vec3(camPos.x, camPos.y, 0.0f),
						   glm::vec3(0.0f, -1.0f, 0.0f));
		mvp = projection * view;
		manager = new ofUniformEntityGrid(glm::vec3(384.0f));
		ofSetClearColor(glm::vec4(0.3f, 0.5f, 0.8f, 1.0f));
	}

	void load() {
		// Straight path
		for(int i = 0; i < 5; i++)
			manager->add(new Solid(glm::vec2(64.0f * i, 192.0f)));
		// Straight path, a little below
		for(int i = 0; i < 7; i++)
			manager->add(new Solid(glm::vec2(256.0f + (64.0f * i), 256.0f)));
		// Straight path
		for(int i = 0; i < 5; i++)
			manager->add(new Solid(glm::vec2(640.0f + (64.0f * i), 192.0f)));
		// Straight way down
		for(int i = 0; i < 5; i++)
			manager->add(new Solid(glm::vec2(960.0f, 192.0f + (64.0f * i))));
		// Straight path
		for(int i = 0; i < 15; i++)
			manager->add(new Solid(glm::vec2(960.0f + (64.0f * i), 576.0f)));

		// Straight paths, jumpthru
		for(int j = 0; j > -5; j--)
			for(int i = 0; i < 5; i++)
				manager->add(new Solid(glm::vec2(1216.0f + (64.0f * i), 512.0f + (96.0f * j)),
									   true,
									   glm::vec2(64.0f, 16.0f)));

		// Small cave
		manager->add(new Solid(glm::vec2(896.0f, 576.0f)));
		manager->add(new Solid(glm::vec2(832.0f, 576.0f)));
		manager->add(new Solid(glm::vec2(768.0f, 576.0f)));
		manager->add(new Solid(glm::vec2(768.0f, 512.0f)));
		manager->add(new Solid(glm::vec2(768.0f, 448.0f)));
		manager->add(new Solid(glm::vec2(768.0f, 384.0f)));
		manager->add(new Solid(glm::vec2(832.0f, 384.0f)));
		manager->add(new Solid(glm::vec2(896.0f, 384.0f)));

		player = new Player;
		manager->add(player);
		watch("Player", ofWatcherEntity, player);
		watch("Manager", ofWatcherPtr, manager);
		
	}

	void unload() {
		unwatch("Player");
		unwatch("Manager");
		delete manager;
	}
     
	void updateCamera() {
		// Fetch camera from player position
		if(player) {
			glm::vec3 playerPos = player->getPosition();
			camPos.x = playerPos.x - 320.0f;
			camPos.y = playerPos.y - 180.0f;
		}

		// Limit camera to a minimum X
		camPos.x = (camPos.x < 0.0f) ? 0.0f : camPos.x;
		camPos.y = (camPos.y < 0.0f) ? 0.0f : camPos.y;
		
		view = glm::lookAt(glm::vec3(camPos.x, camPos.y, -1.2f),
						   glm::vec3(camPos.x, camPos.y, 0.0f),
						   glm::vec3(0.0f, -1.0f, 0.0f));
		mvp = projection * view;
	}
	
	void update(float dt) {
		manager->update(dt);
		updateCamera();
	}

	void draw() {
		manager->draw(mvp);
	}
};


int main(int argc, char** argv)
{
	ofInit({"wname=Small Platformer",
			//"frmrt=60c20m",
			"vsync=on"});
	ofMapDefaultsP1();
	#ifdef LUA_MODE
	ofSetReplType(ofReplLua);
	#endif
	ofCanvasManager::add(new Game, 0, "Game");
	ofGameLoop();
	ofQuit();
	return 0;
}

