#include <oficina2/oficina.hpp>
#include <sstream>
#include <iomanip>
using namespace oficina;
/* ======== Global ======== */
static glm::uvec2 score;


/* ======== Ball ======== */

class Ball : public ofEntity
{
public:
	void init();
	void load();
	void unload();
	void update(float dt);
	void draw(glm::mat4 mvp);
private:
	const float     gravity         = 0.196f;
	const float     topY            = 20.0f;
	const float     bounceFactor    = 0.9678f;
	const float     netBounceFactor = 0.259f;
	const float     minNetDistance  = 15.0f;
	glm::vec3 speed;
	glm::vec3 initial;
	glm::vec4 ballColor;
	int       nextPlayer;
	int       currentPlayer;

	ofPrimitive* pArrow;
	glm::mat4    arrowModel;
	float        arrowAngle;
	bool         showArrow;
	glm::vec4    arrowColor;
};

void Ball::init()
{
	initial.x = 360.0f;
	initial.y = 450.0f;
	scale(glm::vec3(5.0f), true);
	translate(initial);
	speed.x = 5.0f;
	arrowAngle = glm::radians(-45.0f);
	setProperty(1, true);
	setProperty(2, true);
	nextPlayer = 1;
}

void Ball::load()
{
	pArrow = []() {
		float arrowShape[] = {
			10.0f, 0.0f, 0.0f,
			20.0f, 0.0f, 0.0f,

			19.372f, -0.625f, 0.0f,
			20.0f, 0.0f, 0.0f,

			19.372f, 0.625f, 0.0f,
			20.0f, 0.0f, 0.0f
		};
		return ofPrimitiveRenderer::makePrimitive(
			ofLines, 6, sizeof(arrowShape), arrowShape);
	}();
}

void Ball::unload()
{
	delete pArrow;
}

void Ball::update(float dt)
{
	// Apply gravity
	if(getProperty(0)) {
		speed.y += gravity;
		speed.y = ofClamp(speed.y, -topY, topY);
		translate(speed, false);
	}

	// Collision
	float ballTop;
	float ballBottom;
	float ballLeft;
	float ballRight;
	const float netX = 640.0f;
	auto recalcHitbox = [&]() {
		ballTop    = position.y - 5.0f;
		ballBottom = position.y + 5.0f;
		ballLeft   = position.x - 5.0f;
		ballRight  = position.x + 5.0f;
	};

	recalcHitbox();
	
	// Bounce on bottom
	if((position.x >= 320.0f && position.x <= 960.0f)
	   && (position.y > 520.0f && speed.y > 0.0f)) {
		translate(glm::vec3(position.x, 520.0f - 5.0f, 0.0f), true);
		speed.y *= -bounceFactor;
		if((position.x < netX) && getProperty(1))
			setProperty(2, true);
		else if((position.x > netX) && getProperty(1))
			setProperty(2, false);
	}

	recalcHitbox();
	
	// Bounce on net
	// If colliding with the net
	if((ballBottom >= 480.0f) && (ballTop <= 520.0f)) {
		if((ballLeft >= netX - 10.0f) && (ballRight <= netX + 10.0f)) {
			setProperty(1, false);
			showArrow = false;
			if(speed.x > 0.0f) {
				translate(glm::vec3(netX - 5.0f, position.y, 0.0f), true);
				speed.x *= -netBounceFactor;
				speed.y *= netBounceFactor;
			} else if(speed.x < 0.0f) {
				translate(glm::vec3(netX + 5.0f, position.y, 0.0f), true);
				speed.x *= -netBounceFactor;
				speed.y *= netBounceFactor;
			} else {
				// Something went wrong
			}
		}
	}

	// Color
	if(!getProperty(2)) ballColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	else                ballColor = glm::vec4(0.3f, 0.5f, 0.8f, 1.0f);

	// Arrow
	// Calculate angle
	glm::vec2 mousePos = ofGetMousePos();
	mousePos /= ofGetWindowSize();
	mousePos *= decltype(mousePos)(1280, 720);
	arrowAngle = atan2(mousePos.y - position.y, mousePos.x - position.x);

	// Clamp angle according to ball position
	if(position.x < netX) {
		arrowAngle = ofClamp(arrowAngle, glm::radians(-75.0f), 0.0f);
		arrowColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	} else if(position.x > netX) {
		if(arrowAngle < 0.0f)
			arrowAngle = ofClamp(arrowAngle, glm::radians(-180.0f), glm::radians(-105.0f));
		else arrowAngle = ofClamp(arrowAngle, glm::radians(180.0f), glm::radians(255.0f));
		arrowColor = glm::vec4(0.3f, 0.5f, 0.8f, 1.0f);
	}
	
	arrowModel = glm::translate(glm::mat4(), position)
		* glm::rotate(glm::mat4(), arrowAngle, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::scale(glm::mat4(), glm::vec3(2.0f, 1.0f, 1.0f));

	recalcHitbox();
	// Shoot towards arrow on click
	// TODO: Find a better way to do this.
	// Perharps a smart way considering the last time?
	if((position.x < netX - minNetDistance)
	   ||(position.x > netX + minNetDistance))
		currentPlayer = (position.x < netX);
	
	showArrow = (nextPlayer == currentPlayer);
	if((ballTop <= 520.0f) && (ballLeft >= 160.0f) && (ballRight <= 1120.0f)) {
		if(ofMouseButtonTap(ofMouseLeft) && getProperty(1) && (nextPlayer == currentPlayer)) {
			speed.x = 10.0f * glm::cos(arrowAngle);
			speed.y = 10.0f * glm::sin(arrowAngle);
			setProperty(0, true);
			setProperty(2, (position.x < netX));
			nextPlayer = !nextPlayer;
		}
	} else {
		showArrow = false;
		if((ballTop >= 720.0f) || (ballLeft <= 0.0f) || (ballRight >= 1280.0f)) {
			// Respawn
			speed.x = 5.0f;
			speed.y = 0.0f;
			//translate(initial, true);
			setProperty(0, false);
			setProperty(1, true);
			score[getProperty(2) ? 1 : 0] += 100;
			nextPlayer = !getProperty(2);
			translate(glm::vec3(getProperty(2)
								? 1280.0f - initial.x
								: initial.x, initial.y, 0.0f), true);
			toggleProperty(2);
		}
	}
}

void Ball::draw(glm::mat4 mvp)
{
	ofPrimitiveRenderer::drawSphere(
		ballColor, mvp * getModelMatrix());
	if(showArrow && getProperty(1))
		ofPrimitiveRenderer::draw(
			pArrow, arrowColor, mvp * arrowModel);
}



/* ======== Game Screen ======== */

class Game : public ofCanvas
{
public:
	void init();
	void load();
	void unload();
	void update(float dt);
	void draw();
private:
	glm::mat4         mvp;
	ofPrimitive*      field;
	ofIEntityManager* manager;
	ofFont            font;
};

void Game::init()
{
	mvp = glm::ortho(
		0.0f,
		1280.0f, -720.0f,
		0.0f, -100.0f, 100.0f)
		* glm::lookAt(
			glm::vec3(0.0f, 0.0f, -1.2f),
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f));
	manager = new ofEntityList;

	// Increase line stroke width
	glLineWidth(3.0f);
}

void Game::load()
{
	field = []() {
		float fieldShape[] = {
			// Field
			320.0f, 520.0f, 0.0f,
			960.0f, 520.0f, 0.0f,
			// Net
			640.0f, 520.0f, 0.0f,
			640.0f, 480.0f, 0.0f
		};
		return ofPrimitiveRenderer::makePrimitive(
			ofLines, 4, sizeof(fieldShape), fieldShape);
	}();
	manager->add(new Ball);
	font = ofTexturePool::loadDefaultFont(ofFontFaceFixedsysExcelsior);
}

void Game::unload()
{
	font.unload();
	delete field;
	delete manager;
}

void Game::update(float dt)
{
	manager->update(dt);
}

void Game::draw()
{
	manager->draw(mvp);
	ofPrimitiveRenderer::draw(field, glm::vec4(1.0f), mvp);
	
	std::stringstream oss;
	glm::vec2 textSize;

	for(int i = 0; i < 2; i++) {
		oss.str("");
		oss << std::setfill('0') << std::setw(8) << score[i];
		textSize = font.measure(oss.str());
	
		font.write(oss.str(),
				   glm::vec2((320.0f * i) + 480.0f - (textSize.x / 2.0f), 180.0f),
				   mvp, glm::vec4(1.0f));
	}
}



/* ======== Main Procedure ======== */

int main(int argc, char** argv)
{
	ofInit({"wname=Tennis for Two", "winsz=720p", "frmrt=60c", "vsync=on"});
	ofCanvasManager::add(new Game);
	ofGameLoop();
	ofQuit();
	return 0;
}
