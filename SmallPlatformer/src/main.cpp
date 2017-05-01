#include <oficina2/oficina.hpp>
using namespace oficina;

// Properties:
// 0. IsPlayer
// 1. IsSolid
// 2. IsJumpThru

class Solid : public ofEntity
{
private:
	ofPrimitive* shape;
public:
	Solid(glm::vec2 position, glm::vec2 size = glm::vec2(64.0f)) {
		scale(glm::vec3(size, 1.0f), true);
		translate(glm::vec3(position, 0.0f), true);
	}
	
	void init() {
		setProperty(0, false);
		setProperty(1, true);
		setProperty(2, false);
	}

	void load() {
		float theShape[] = {
			0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f
		};
		shape = ofPrimitiveRenderer::makePrimitive(ofTriangleFan,
												   4, sizeof(theShape),
												   theShape);
	}

	void unload() {
		delete shape;
	}

	void update(float dt) {
	}

	void draw(glm::mat4 mvp) {
		ofPrimitiveRenderer::draw(shape, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), mvp * getModelMatrix());
	}
};

class Sensor : public ofIComponent
{
private:
	float radius = 5.0f;
	glm::vec3 position;
	glm::vec2 relpos;
public:
	Sensor(glm::vec2 relpos) {
		this->relpos = relpos;
	}
	void init() {
		radius = 5.0f;
	}
	void load() {
	}
	void unload() {
	}
	void update(float dt) {
		position = parent->getPosition();
		position.x += relpos.x;
		position.y += relpos.y;
	}
	void draw(glm::mat4 mvp) {
	}

    bool isOverlapping(ofEntity* solidptr) {
		glm::vec3 solidpos = solidptr->getPosition();
		glm::vec3 solidsz  = solidptr->getScale();
		return ((((position.y + radius) >= solidpos.y)
				 && (position.y <= (solidpos.y + solidsz.y)))
				&& (((position.x + radius) >= solidpos.x)
					&& (position.x <= (solidpos.x + solidsz.x))));
				
	}
};

class Player : public ofEntity
{
private:
	// Handle for the animator component
    ofAnimator* animator;
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
    float     maxSpd;
    float     direction;
    bool      ground;

	// Sensors
	Sensor* bottomSensor;
	Sensor* leftSensor;
	Sensor* rightSensor;
public:
	void init() {
		gravity     = 0.8165f;
		hitboxRadX  = 16.0f;
		hitboxRadY  = 32.0f;
		accel       = 0.166f;
		decel       = 0.3f;
		jmpStg      = -12.0f;
		minJmp      = -6.0f;
		maxSpd      = 8.0f;
		direction   = 1.0f;
		ground      = false;
	
		setProperty(0, true);
		setProperty(1, false);
		setProperty(2, false);
		translate(glm::vec3(128.0f, 128.0f, 0.0f), true);
	}

	void load() {
		auto myTexture = ofTexturePool::load("res/smileman.png");
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
			animator->reg("walking", 4, walking, 8.0f, true);
			animator->reg("jumping", 1, jumping, 1.0f, true);
		}
		animator->SetAnimation("stopped");

		// Add sensors
		bottomSensor = new Sensor(glm::vec2(0.0f, hitboxRadY));
		leftSensor   = new Sensor(glm::vec2(-hitboxRadX, 0.0f));
		rightSensor  = new Sensor(glm::vec2(hitboxRadX, 0.0f));
		AddComponent("BottomSensor", bottomSensor);
		AddComponent("LeftSensor",   leftSensor);
		AddComponent("RightSensor",  rightSensor);
	}

	void unload() {
		ClearComponents();
	}

	bool isOverlapping(ofEntity* solidptr) {
		glm::vec3 solidpos = solidptr->getPosition();
		glm::vec3 solidsz  = solidptr->getScale();
		return ((((position.y + hitboxRadY) >= solidpos.y)
				 && (position.y <= (solidpos.y + solidsz.y)))
				&& (((position.x + hitboxRadX) >= solidpos.x)
					&& (position.x <= (solidpos.x + solidsz.x))));
				
	}

	void update(float dt) {
		auto pos    = getPosition();
		auto lstick = ofGetLeftStick();

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
				speed.x -= decel;
			} else if(lstick.x > 0.0f && speed.x < 0.0f) {
				speed.x += decel;
			}
		}

		// New collision
		ground = false;
		for(auto obj : parent->getNearest(this))
		{
			if(obj->getProperty(0))
				continue;

			glm::vec3 solidpos = obj->getPosition();
			glm::vec3 solidsz  = obj->getScale();
			
			// Ground collision
			if(!ground                // No ground found previously
			   && speed.y >= 0.0f     // Player is not going up
			   && obj->getProperty(1) // Is Solid
			   && bottomSensor->isOverlapping(obj)) {
				// It is indeed a solid object.
				// Ground collisions ahoy
				pos.y = (solidpos.y - hitboxRadY);
				speed.y = 0.0f;
				ground = true;
			}

			// Left collision
			if(speed.x < 0.0f
			   && obj->getProperty(1)  // Is Solid
			   && !obj->getProperty(2) // Is NOT jumpthru
			   && leftSensor->isOverlapping(obj)) {
				pos.x = (solidpos.x + solidsz.x) + hitboxRadX;
				speed.x = 0.0f;
			}

			// Right collision
			if(speed.x > 0.0f
			   && obj->getProperty(1)  // Is Solid
			   && !obj->getProperty(2) // Is NOT jumpthru
			   && rightSensor->isOverlapping(obj)) {
				pos.x = solidpos.x - hitboxRadX;
				speed.x = 0.0f;
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
				float norm = abs(speed.x) / maxSpd;
				norm = 1.0f - norm;
				float animspd = norm * 6.0f;
				animator->SetAnimationSpeed(animspd + animator->GetDefaultAnimationSpeed());
			}
		}
		else animator->SetAnimation("jumping");

		UpdateComponents(dt);
		
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
		player = new Player;
		manager->add(player);
		
		for(int i = 0; i < 5; i++)
			manager->add(new Solid(glm::vec2(64.0f * i, 192.0f)));
		for(int i = 0; i < 5; i++)
			manager->add(new Solid(glm::vec2(320.0f + (64.0f * i), 256.0f)));
		for(int i = 0; i < 5; i++)
			manager->add(new Solid(glm::vec2(640.0f + (64.0f * i), 192.0f)));
	}

	void unload() {
		delete manager;
	}
     
	void updateCamera() {
		glm::vec3 playerPos = player->getPosition();
		camPos.x = playerPos.x - 320.0f;
		camPos.y = playerPos.y - 180.0f;
		
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
	ofCanvasManager::add(new Game);
	ofGameLoop();
	ofQuit();
	return 0;
}

