#ifndef INPUT_H
#define INPUT_H

#include "System.h"
#include "Camera.h"
#include "GL/freeglut.h"
#include "Globals.h"

struct SMOUSE
{
	int x, y;
	bool lmb, rmb, mmb;
};

class Input
{

public:
	void GetInput();

private:

};
void KeyPress(unsigned char key, int x, int y);
void MouseMotion(int x, int y);
void MousePassiveMotion(int x, int y);
void MouseButton(int Button, int state, int x, int y);
void ToggleFullScreen();

#endif // INPUT_H
