#ifndef SSYSTEM_H
#define SSYSTEM_H

#include <cmath>
#include "glm/glm.hpp"

struct COMP_RES
{
	void setScreenSize(unsigned int width, unsigned int height);
	unsigned int getScreenWidth(void);
	unsigned int getScreenHeight(void);
	bool FullScreen, show;
	float MouseSensitivity, KeySensitivity, cameraSpeed, ScreenNear, ScreenFar;

private:
	unsigned int screenWidth, screenHeight;
};

struct Player
{
	glm::vec3 position;
	glm::vec3 lastposition;
};

class COMP_SYSTEM
{
public:
	COMP_RES COMP_SETTINGS;
	Player PLAYER;
	glm::vec3 UpdatePosition();
	void SET_FPS(double fps);
	double FPS;
	int fps, FPSD;
	int GET_FPS(void);

};

#endif
