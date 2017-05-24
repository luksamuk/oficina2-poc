#include <oficina2/oficina.hpp>
using namespace oficina;

class MyScene : public ofCanvas
{
private:
	glm::mat4 model;
	glm::mat4 boxmodel;
public:
	void init() {
		// Escalona os modelos para metade de seus tamanhos
		model = glm::scale(model, glm::vec3(0.5f));
		boxmodel = glm::scale(boxmodel, glm::vec3(0.5f));
		// Roda a esfera pra cima
		model = glm::rotate(model, float(M_PI / 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		// Adiciona ambas as matrizes ao Watcher
		watch("Sphere_Model", ofWatcherMat4, &model);
		watch("Box_Model", ofWatcherMat4, &boxmodel);
		ofScmEval("(clear)", true);
	}

	void load() {
	}

	void unload() {
	}

	void update(float dt) {
		model = glm::rotate(model, 3.0f * dt, glm::vec3(1.0f, 1.0f, 0.0f));
		boxmodel = glm::rotate(boxmodel, 3.0f * dt, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	void draw() {
		ofPrimitiveRenderer::drawSphere(glm::vec4(1.0f), model, true);
		ofPrimitiveRenderer::drawBox(glm::vec4(1.0f), boxmodel, true);
	}
};


int main(int argc, char** argv)
{
    ofInit({"wname=Basic 3D", "winsz=500x500", "vsync=on"});
	ofCanvasManager::add(new MyScene, 0, "MyScene");
    ofGameLoop();
    ofQuit();
    return 0;
}
