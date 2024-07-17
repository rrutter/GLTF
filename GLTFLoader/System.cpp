#include <windows.h>
#include "Camera.h"
#include "System.h"
#include <iostream>

extern CCamera Camera;


void COMP_SYSTEM::SET_FPS(double FPS)
{
	fps = static_cast<int>(std::round(FPS));
}


int COMP_SYSTEM::GET_FPS(void)
{
   return fps;
}

void COMP_RES::setScreenSize(unsigned int width, unsigned int height)
{
	screenHeight = height;
	screenWidth = width;
}

unsigned int COMP_RES::getScreenWidth(void)
{
	return screenWidth;
}

unsigned int COMP_RES::getScreenHeight(void)
{
	return screenHeight;
}



glm::vec3 COMP_SYSTEM::UpdatePosition(void)
{
	PLAYER.lastposition.x = PLAYER.position.x;
	PLAYER.lastposition.y = PLAYER.position.y;
	PLAYER.lastposition.z = PLAYER.position.z;
	
	if(PLAYER.lastposition.x!=Camera.Pos.x)
	{
		PLAYER.position.x = Camera.Pos.x;
		PLAYER.position.y = Camera.Pos.y;
		PLAYER.position.z = Camera.Pos.z;
	}
	return PLAYER.position;
}