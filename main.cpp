#include "include.h"
#include "ball.h"
#include "camera.h"
#include "entity.h"
#include "entityRenderer.h"
#include "shader.h"
#include "skybox.h"
#include "terrain.h"
#include "terrainRenderer.h"
#include "texture.h"
#include "gui.h"
#include "water.h"

extern const int WIDTH = 1000;
extern const int HEIGHT = 1000;

extern const float WATERHEIGHT = 20;

// Keyboard and Mouse
//
// glfw is a piece of shit :-)

int keyPressed = 0;
int mouseX = 0;
int mouseY = 0;
bool mouseLeftPressed = false;
bool mouseRightPressed = false;
int mouseScrollOffset = 0;

void onKeyBoard(GLFWwindow * window, int key,
				int scancode, int action, int mods)
{
	if(key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE)
		exit(0);
	else
		keyPressed = key;
}

void onMousePosition(GLFWwindow * window, double xx, double yy)
{
	mouseX = xx;
	mouseY = yy;
}

void onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT) {
		if(action == GLFW_PRESS) mouseLeftPressed = true;
		if(action == GLFW_RELEASE) mouseLeftPressed = false;
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT) {
		if(action == GLFW_PRESS) mouseRightPressed = true;
		if(action == GLFW_RELEASE) mouseRightPressed = false;
	}
}

void onMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
	mouseScrollOffset = yoffset;
}

void InitCallbacks(GLFWwindow * window)
{
	glfwSetKeyCallback(window, onKeyBoard);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, onMousePosition);
	glfwSetMouseButtonCallback(window, onMouseButton);
	glfwSetScrollCallback(window, onMouseScroll);
}


int main()
{

	glfwInit();

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_CENTER_CURSOR, GL_TRUE);

	GLFWwindow * window = glfwCreateWindow(WIDTH, HEIGHT,
											"Rolling Ball", NULL, NULL);

	glfwMakeContextCurrent(window);

	glewExperimental = true;
	glewInit();

	InitCallbacks(window);

	glEnable(GL_DEPTH_TEST);


	glm::mat4 projectionMatrix = glm::perspective(45.0f, 1.0f, 0.1f, 10000.0f);

	// entity
	Shader entityShader("entity.vs", "entity.fs");
	EntityRenderer entityRenderer(&entityShader, projectionMatrix);

	vector<Entity*> entities;

	RawModel rawModelBall = LoadObjModel("ball.obj");
	TexturedModel texturedBall(rawModelBall, Texture("box.png"));
	Ball * ball = new Ball(&texturedBall, 
								glm::vec3(3.0f, 0.0f, 3.0f),
								// glm::vec3(82.0f, 100.f, 70.0f),
								glm::vec3(0.0f), 1.0f);
	entities.push_back(ball);
	TexturedModel * texturedTree = new TexturedModel(
										LoadObjModel("tree.obj"),
										Texture("tree.png")
									);
	Entity * tree = new Entity(texturedTree,
								glm::vec3(15.0f, 25.0f, 15.0f),
								glm::vec3(0.0f), 1.0f);
	entities.push_back(tree);

	// terrain
	vector<Terrain> terrains;
	terrains.push_back(Terrain("heightmap.jpg"));

	Shader terrainShader("terrain.vs", "terrain.fs");
	TerrainRenderer terrainRenderer(&terrainShader, projectionMatrix);

	// skybox
	Shader skyboxShader("skybox.vs", "skybox.fs");
	SkyboxRenderer skyboxRenderer(&skyboxShader, projectionMatrix);


	// camera
	Camera * camera = new Camera(ball);

	// water
	WaterFrameBuffer * waterFrameBuffer = new WaterFrameBuffer();
	Shader * waterShader = new Shader("water.vs", "water.fs");
	Texture * dudvMap = new Texture("dudv2.png");
	WaterRenderer * waterRenderer = new WaterRenderer(waterShader, 
										projectionMatrix, 
										waterFrameBuffer,
										dudvMap);
	vector<Water> waters;
	waters.push_back(Water(40.0f, WATERHEIGHT, 40.0f, 40.0f));



	// gui
	vector<GUI> guis;
	GUIRenderer guiRenderer;

	// GUI dudvGUI(Texture("dudv.jpg"));
	// dudvGUI.setPositionAndSize(0, 0, 300, 300);
	// guis.push_back(dudvGUI);

	// GUI guiReflection(Texture(waterFrameBuffer->getReflectionTexture()));
	// guiReflection.setPositionAndSize(0, 0, 1000, 1000);
	// guis.push_back(guiReflection);

	// GUI guiRefraction(Texture(waterFrameBuffer->getRefractionTexture()));
	// guiRefraction.setPositionAndSize(600, 0, 400, 400);
	// guis.push_back(guiRefraction);


	glViewport(0, 0, WIDTH, HEIGHT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// do some update
		ball->update(terrains[0]);
		camera->update(terrains[0]);


		glm::vec3 cameraPostion = camera->getPosition();
		glm::vec3 ballPosition = ball->getPosition();

		float ballDistanceFromWater = ballPosition.y - WATERHEIGHT;
		ball->setPosition(ballPosition.x,
							ballPosition.y - 2 * ballDistanceFromWater,
							ballPosition.z);
		float cameraDistanceFromWater = cameraPostion.y - WATERHEIGHT;
		camera->setPosition(cameraPostion.x, 
							cameraPostion.y - 2 * cameraDistanceFromWater,
							cameraPostion.z);
		glm::mat4 viewMatrix = camera->getViewMatrix();


		glEnable(GL_CLIP_DISTANCE0);
		// water render to reflection buffer
		waterFrameBuffer->bindReflectionBuffer();

				// skybox
				skyboxShader.bindGL();
				skyboxShader.setViewMatrix(glm::mat4(glm::mat3(viewMatrix)));
				skyboxRenderer.render();
				skyboxShader.unbindGL();

				// terrain
				terrainShader.bindGL();
				terrainShader.setUniform4f("clipPlane", 0, 1, 0, -20);
				terrainShader.setViewMatrix(viewMatrix);
				terrainRenderer.render(terrains);
				terrainShader.unbindGL();

				// entity
				entityShader.bindGL();
				entityShader.setUniform4f("clipPlane", 0, 1, 0, 20);
				entityShader.setViewMatrix(viewMatrix);
				entityRenderer.render(entities);
				entityShader.unbindGL();

		ball->setPosition(ballPosition);
		camera->setPosition(cameraPostion);
		viewMatrix = camera->getViewMatrix();

		// water render to refraction buffer
		waterFrameBuffer->bindRefractionBuffer();

				// skybox
				// skyboxShader.bindGL();
				// skyboxShader.setViewMatrix(glm::mat4(glm::mat3(viewMatrix)));
				// skyboxRenderer.render();
				// skyboxShader.unbindGL();

				// terrain
				terrainShader.bindGL();
				terrainShader.setUniform4f("clipPlane", 0, -1, 0, 20);
				terrainShader.setViewMatrix(viewMatrix);
				terrainRenderer.render(terrains);
				terrainShader.unbindGL();

				// entity
				// entityShader.bindGL();
				// entityShader.setUniform4f("clipPlane", 0, -1, 0, 20);
				// entityShader.setViewMatrix(viewMatrix);
				// entityRenderer.render(entities);
				// entityShader.unbindGL();

		glDisable(GL_CLIP_DISTANCE0);
		// render to screen
		waterFrameBuffer->unbindCurrentFrameBuffer();

				// skybox
				skyboxShader.bindGL();
				skyboxShader.setViewMatrix(glm::mat4(glm::mat3(viewMatrix)));
				skyboxRenderer.render();
				skyboxShader.unbindGL();

				// terrain
				terrainShader.bindGL();
				terrainShader.setViewMatrix(viewMatrix);
				terrainRenderer.render(terrains);
				terrainShader.unbindGL();

				// entity
				entityShader.bindGL();
				entityShader.setViewMatrix(viewMatrix);
				entityRenderer.render(entities);
				entityShader.unbindGL();



		static float rx = 0;
		ball->setRotation(rx, 0.0f, 0.0f);
		rx += 0.005f;

		waterRenderer->render(waters, camera);

		// gui
		guiRenderer.render(guis);


		// some shit

		keyPressed = 0;
		mouseScrollOffset = 0;

		glfwSwapBuffers(window);
		glfwWaitEventsTimeout(1.0/60);
	} while(true);

	glfwTerminate();

}