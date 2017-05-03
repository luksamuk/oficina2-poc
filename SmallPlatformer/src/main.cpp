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
		//DrawComponents(mvp);
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
    float     gravity;
    float     hitboxRadX;
    float     hitboxRadY;
    float     groundY;
    // Variables for our object
    glm::vec3 speed;
    float     accel;
    float     decel;
    float     jmpStg;
	float     minJmp;
	float     defaultMaxSpd;
	float     runMaxSpd;
    float     maxSpd;
    float     direction;
    bool      ground;

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
		gravity     = 0.8165f;
		hitboxRadX  = 16.0f;
		hitboxRadY  = 32.0f;
		accel       = 0.166f;
		decel       = 0.3f;
		jmpStg      = -13.0f;
		minJmp      = -6.5f;
		defaultMaxSpd = 4.0f;
		runMaxSpd     = 8.5f;
		maxSpd      = defaultMaxSpd;
		direction   = 1.0f;
		ground      = false;
	
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
		masterSensor  = new ofAABB(glm::vec3(), glm::vec3(hitboxRadX, hitboxRadY, 0.5f));
		bottomSensor  = new ofBoundingSphere(glm::vec3(0.0f, hitboxRadY, 0.0f), 5.0f);
		bottomLSensor = new ofBoundingSphere(glm::vec3(-hitboxRadX + 8.0f, hitboxRadY, 0.0f), 5.0f);
		bottomRSensor = new ofBoundingSphere(glm::vec3(hitboxRadX - 8.0f, hitboxRadY, 0.0f), 5.0f);
		ledgeLSensor  = new ofBoundingSphere(glm::vec3(-hitboxRadX + 5.0f, hitboxRadY, 0.0f), 5.0f);
		ledgeRSensor  = new ofBoundingSphere(glm::vec3(hitboxRadX - 5.0f, hitboxRadY, 0.0f), 5.0f);
		leftSensor    = new ofBoundingSphere(glm::vec3(-hitboxRadX, 0.0f, 0.0f), 5.0f);
		rightSensor   = new ofBoundingSphere(glm::vec3(hitboxRadX, 0.0f, 0.0f), 5.0f);
		topSensor     = new ofBoundingSphere(glm::vec3(0.0f, -hitboxRadY, 0.0f), 5.0f);
		
		AddComponent("MasterSensor", masterSensor);
		AddComponent("BottomSensor", bottomSensor);
		AddComponent("BottomLSensor", bottomLSensor);
		AddComponent("BottomRSensor", bottomRSensor);
		AddComponent("LedgeLSensor", ledgeLSensor);
		AddComponent("LedgeRSensor", ledgeRSensor);
		AddComponent("LeftSensor",   leftSensor);
		AddComponent("RightSensor",  rightSensor);
		AddComponent("TopSensor",    topSensor);

		ofLuaDefineSymbol("player", this);
	}

	void unload() {
		ofLuaUndefine("player");
		ClearComponents();
		ofTexturePool::unload(myTexture);
	}

	void update(float dt) {
		auto pos    = getPosition();
		auto lstick = ofGetLeftStick();
		UpdateComponents(dt);

		// Y axis movement
		if(!ground) speed.y += gravity;
		if(ground && ofButtonTap(ofPadA)) {
			ground = false;
			speed.y = jmpStg ;
		}

		// X axis movement
		speed.x += (lstick.x * accel);
		speed.x = ofClamp(speed.x, -maxSpd, maxSpd);
		if(ground) {
			if(lstick.x == 0.0f) {
				if(speed.x > 0.0f) speed.x -= decel;
				else if(speed.x < 0.0f) speed.x += decel;

				if(abs(speed.x) < decel) speed.x = 0.0f;
			} else if(lstick.x < 0.0f && speed.x > 0.0f) {
				speed.x -= decel * 2.0f;
			} else if(lstick.x > 0.0f && speed.x < 0.0f) {
				speed.x += decel * 2.0f;
			}
		}
		if(ofButtonPress(ofPadX)) maxSpd = runMaxSpd;
		else                      maxSpd = defaultMaxSpd;

		// New collision
		ground = false;
		for(auto obj : parent->getNearest(this))
		{
			if(obj->getProperty(0))
				continue;

			auto objBV = static_cast<ofAABB*>(obj->GetComponent("AABB"));

			glm::vec3 solidpos = obj->getPosition();
			glm::vec3 solidsz  = obj->getScale();
			
			// Ground collision
			if(!ground                // No ground found previously
			   && speed.y >= 0.0f     // Player is not going up
			   && obj->getProperty(1) // Is Solid
			   && (bottomSensor->isOverlapping(objBV)
				   || bottomLSensor->isOverlapping(objBV)
				   || bottomRSensor->isOverlapping(objBV))) {
				// It is indeed a solid object.
				// Ground collisions ahoy
				pos.y = (solidpos.y - hitboxRadY);
				speed.y = 0.0f;
				ground = true;
				//ofLog(ofLogInfo, "Ground collision\n");
			}

			// Top collision
			if(speed.y < 0.0f          // Player is going up
			   && obj->getProperty(1)  // Is Solid
			   && !obj->getProperty(2) // Is NOT jumpthru
			   && topSensor->isOverlapping(objBV)) {
				pos.y = (solidpos.y + solidsz.y) + hitboxRadY;
				speed.y = 0.0f;
				//ofLog(ofLogInfo, "Top collision\n");
			}

			// Left collision
			if(speed.x < 0.0f
			   && obj->getProperty(1)  // Is Solid
			   && !obj->getProperty(2) // Is NOT jumpthru
			   && leftSensor->isOverlapping(objBV)) {
				pos.x = (solidpos.x + solidsz.x) + hitboxRadX;
				speed.x = 0.0f;
				//ofLog(ofLogInfo, "Left collision\n");
			}

			// Right collision
			if(speed.x > 0.0f
			   && obj->getProperty(1)  // Is Solid
			   && !obj->getProperty(2) // Is NOT jumpthru
			   && rightSensor->isOverlapping(objBV)) {
				pos.x = solidpos.x - hitboxRadX;
				speed.x = 0.0f;
				//ofLog(ofLogInfo, "Right collision\n");
			}
		}

		// Platformer-like jump
		if(!ground
		   && (speed.y < minJmp)
		   && !ofButtonPress(ofPadA))
			speed.y = minJmp;

		// Transform position
		pos += speed;

		// Hand position back to engine
		translate(pos, true);

		// Direction
		direction = (speed.x > 0.0f) ? 1.0f :
			((speed.x < 0.0f) ? -1.0f :
			 direction);
		scale(glm::vec3(direction, 1.0f, 1.0f), true);
		// Animation
		if(ground)
		{
			if(speed.x == 0.0f) // If not moving, stop
				animator->SetAnimation("stopped");
			else
			{
				animator->SetAnimation("walking");
				float norm = abs(speed.x) / runMaxSpd;
				norm = 1.0f - norm;
				float animspd = norm * 6.0f;
				animator->SetAnimationSpeed(animspd + animator->GetDefaultAnimationSpeed());
			}
		}
		else animator->SetAnimation("jumping");
		
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
		projection = glm::ortho(0.0f, 640.0f, -360.0f, 0.0f, 1.0f, 10.0f);
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
		
	}

	void unload() {
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
			"frmrt=60c20m",
			"vsync=on"});
	ofMapDefaultsP1();
	ofSetReplType(ofReplLua);
	ofCanvasManager::add(new Game);
	ofGameLoop();
	ofQuit();
	return 0;
}

