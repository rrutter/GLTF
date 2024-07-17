#pragma once
#ifndef GAME_LOOP_H
#define GAME_LOOP_H

#include "Input.h"
#include "Globals.h"
#include <iostream>
#include "System.h"
#include "Camera.h"


extern void KeyPress(unsigned char key, int x, int y);
extern void MouseMotion(int x, int y);
extern void MouseButton(int button, int state, int x, int y);
extern void MousePassiveMotion(int x, int y);


class GameLoop
{
public:
	void initialize(int argc, char** argv);
};

#endif