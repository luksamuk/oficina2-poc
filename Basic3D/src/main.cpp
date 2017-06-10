#include <oficina2/oficina.hpp>
using namespace oficina;

class MyScene : public ofCanvas
{
private:
	glm::mat4 model;
	glm::mat4 boxmodel;
	glm::mat4 mvp;
	glm::mat4 view;
	glm::mat4 projection;
	float     angle = 30.0f;
public:
	void init() {
		/*glm::frustum(-1.0f, 1.0f, 1.0f, -1.0f,
		  -1.0f, 1.0f)*/
		projection = glm::perspective(30.0f, 1.0f, 0.0f, -1.0f);
			view = glm::lookAt(glm::vec3(0.0f, 0.0f, -1.5f),
							   glm::vec3(0.0f, 0.0f, 0.0f),
							   glm::vec3(0.0f, -1.0f, 0.0f));
		mvp = projection * view;
			
		// Escalona os modelos para metade de seus tamanhos
		model = glm::scale(model, glm::vec3(0.5f));
		boxmodel = glm::scale(boxmodel, glm::vec3(0.5f));
		// Roda a esfera pra cima
		model = glm::rotate(model, float(M_PI / 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		// Adiciona ambas as matrizes ao Watcher
		watch("MVP", ofWatcherMat4, &mvp);
		watch("Sphere_Model", ofWatcherMat4, &model);
		watch("Box_Model", ofWatcherMat4, &boxmodel);
		ofScmEval("(clear)", false);
	}

	void load() {
	}

	void unload() {
	}

	void update(float dt) {
		projection = glm::perspective(angle, 1.0f, 0.0f, -1.0f);
		mvp = projection * view;
		
		model = glm::rotate(model, 3.0f * dt, glm::vec3(1.0f, 1.0f, 0.0f));
		boxmodel = glm::rotate(boxmodel, 3.0f * dt, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void draw() {
		ofPrimitiveRenderer::drawSphere(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f), mvp * model, true);
		ofPrimitiveRenderer::drawBox(glm::vec4(1.0f, 1.0f, 1.0f, 0.5f), mvp * boxmodel, true);
	}
};


int main(int argc, char** argv)
{
    ofInit({"wname=Basic 3D", "winsz=500x500", "vsync=on"});
	ofSetClearColor(glm::vec4(0.3f, 0.5f, 0.8f, 1.0f));
	ofCanvasManager::add(new MyScene, 0, "MyScene");
    ofGameLoop();
    ofQuit();
    return 0;
}
