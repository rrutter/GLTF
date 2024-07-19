#include "GameLoop.h"
#include "GLTF2.h"

COMP_SYSTEM SYS;
CCamera Camera;
LoadPNG PNG;
Input in;


extern PersonalGL pGL;

float Ycheck = 0.0f;
float Xcheck = 10.0f;
float Zcheck = 0.0f;
float scale = .33;
int patchSize = 15;
bool direction = 0;
unsigned int glTextureIndexId = 0;

unsigned int frameCount = 0;
double lastTime = 0.0;
double elapsedTime = 0.0;
double fps = 0.0;


GLTFLoader gltf;




void updateFPS()
{
	frameCount++;
	double currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;
	double deltaTime = currentTime - lastTime;
	// Accumulate elapsed time
	elapsedTime += deltaTime;
	if (elapsedTime >= 1.0) // Update FPS value once every second
	{
		fps = frameCount / elapsedTime; // Calculate frames per second
		SYS.SET_FPS(fps);
		frameCount = 0;
		elapsedTime = 0;

		std::string currentTitle = std::string(WINDOW_TITLE);
		glutSetWindowTitle(currentTitle.c_str());
	}
	lastTime = currentTime;

	gltf.updateAnimation(deltaTime);

}

void renderTestAssets() {

	gltf.render();
}


void Render(void)
{
	renderTestAssets();
}

void Update(void)
{

	updateFPS();
	glutPostRedisplay();
}

void Display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearDepth(1.0f);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glClearColor(.14f, .13f, .15f, .2f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING); // Ensure lighting is enabled
	glEnable(GL_LIGHT0);   // Ensure the light is enabled
	glDisable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);

	float F = SYS.fps;
	if (F == 1) F = 9999999;
	F /= 8; //Frames this second
	F = 2 / F; //all this does is slow or speed up the camera.

	in.GetInput();
	SYS.COMP_SETTINGS.cameraSpeed = (F);
	gluLookAt(Camera.Pos.x, Camera.Pos.y, Camera.Pos.z, Camera.View.x, Camera.View.y, Camera.View.z, Camera.Up.x, Camera.Up.y, Camera.Up.z);
	Render();
	glutSwapBuffers();
}

void Reshape(int ScreenWidth, int ScreenHeight)
{
	glViewport(0, 0, SYS.COMP_SETTINGS.getScreenWidth(), SYS.COMP_SETTINGS.getScreenHeight());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)SYS.COMP_SETTINGS.getScreenWidth() / SYS.COMP_SETTINGS.getScreenHeight(), SYS.COMP_SETTINGS.ScreenNear, SYS.COMP_SETTINGS.ScreenFar);
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	Display();
}

void loadTestAssets() { // temporary until a more robust resource manager is built
	try {
		gltf.loadModel(ASSETS_DIRECTORY "soldier.glb"); //load GLTF
		gltf.initialize();
		gltf.setAnimation("Walk"); //Walk, TPose, Idle, Run
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}

}


void GameLoop::initialize(int argc, char** argv) {
	std::cout << "Initializing Game Loop" << std::endl;

	SYS.COMP_SETTINGS.ScreenFar = 50000;
	SYS.COMP_SETTINGS.ScreenNear = .5f;
	SYS.COMP_SETTINGS.MouseSensitivity = 20;
	SYS.COMP_SETTINGS.KeySensitivity = 1;
	SYS.COMP_SETTINGS.setScreenSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	SYS.FPSD = 0;

	bool doGameMode = false;
	glewInit();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	if (doGameMode) {
		SYS.COMP_SETTINGS.setScreenSize(glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT));
		char gameModeString[64];
		snprintf(gameModeString, sizeof(gameModeString), "%dx%d:%d@%d", glutGet(GLUT_SCREEN_WIDTH), glutGet(GLUT_SCREEN_HEIGHT), 32, 60); // Example: 1920x1080 resolution, 32-bit color, 60Hz refresh rate

		glutGameModeString(gameModeString);
		glutEnterGameMode();
		glutSetWindowTitle(WINDOW_TITLE);
	}
	else {
		glutInitWindowSize(SYS.COMP_SETTINGS.getScreenWidth(), SYS.COMP_SETTINGS.getScreenHeight());
		glutInitWindowPosition(0, 0);
		glutCreateWindow(WINDOW_TITLE);
	}

	Camera.SetCamera(Xcheck, Ycheck, Zcheck, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	SYS.COMP_SETTINGS.show = true;
	ShowCursor(SYS.COMP_SETTINGS.show);
	pGL.loadOpenGLFunctions();


	loadTestAssets();

	glutKeyboardFunc(KeyPress);
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutMotionFunc(MouseMotion);
	glutMouseFunc(MouseButton);
	glutPassiveMotionFunc(MousePassiveMotion);
	glutIdleFunc(Update);
	glutMainLoop();
}