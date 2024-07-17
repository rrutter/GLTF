#include "Input.h"

SMOUSE Mouse;
extern COMP_SYSTEM SYS;
extern CCamera Camera;

bool isFullScreen = false;

void KeyPress(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: /*ESC*/
		glutDestroyWindow(glutGetWindow());
		exit(0);
		break;
	case 'f': // Toggle full-screen mode
		ToggleFullScreen();
		break;
	}
	
}

void ToggleFullScreen() {
	if (isFullScreen) {
		// Switch to windowed mode
		SYS.COMP_SETTINGS.setScreenSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
		glutReshapeWindow(SYS.COMP_SETTINGS.getScreenWidth(), SYS.COMP_SETTINGS.getScreenHeight());
		glutPositionWindow(100, 100);
		isFullScreen = false;
	}
	else {
		// Get the display resolution
		int displayWidth = glutGet(GLUT_SCREEN_WIDTH);
		int displayHeight = glutGet(GLUT_SCREEN_HEIGHT);
		
		SYS.COMP_SETTINGS.setScreenSize(displayWidth, displayHeight);

		// Set the window size to the display resolution
		glutReshapeWindow(displayWidth, displayHeight);

		// Switch to full-screen mode
		glutFullScreen();
		isFullScreen = true;
	}
}


void MouseMotion(int x, int y)
{
	Mouse.x = x;
	Mouse.y = y;
}

void MousePassiveMotion(int x, int y)
{
	Mouse.x = x;
	Mouse.y = y;
}

void Input::GetInput()
{
	// Camera.CalculateTime();

	if (Mouse.lmb == 1)
	{
		SYS.COMP_SETTINGS.show = false;
		POINT mousePos;
		GetCursorPos(&mousePos);

		// Get the handle to the current OpenGL window
		HWND hwnd = FindWindowA(NULL, WINDOW_TITLE);
		if (hwnd == NULL) {
			std::cerr << "Failed to find OpenGL window." << std::endl;
			return;
		}

		// Get the window rectangle
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);

		// Calculate the center of the OpenGL window
		int centerX = (windowRect.left + windowRect.right) / 2;
		int centerY = (windowRect.top + windowRect.bottom) / 2;


		// Set the cursor position to the center of the screen
		SetCursorPos(centerX, centerY);

		// Rotate the camera based on the mouse position
		Camera.RotateByMouse(mousePos.x, mousePos.y, centerX, centerY);
	}
	else if (Mouse.lmb == 0)
	{
		SYS.COMP_SETTINGS.show = true;
	}

	float moveDirection = 0.0f;
	float strafeDirection = 0.0f;

	if (GetKeyState(VK_UP) & 0x80) {
		moveDirection += SYS.COMP_SETTINGS.KeySensitivity;
	}

	if (GetKeyState(VK_DOWN) & 0x80) {
		moveDirection -= SYS.COMP_SETTINGS.KeySensitivity;
	}

	if (GetKeyState(VK_LEFT) & 0x80) {
		strafeDirection -= SYS.COMP_SETTINGS.KeySensitivity;
	}

	if (GetKeyState(VK_RIGHT) & 0x80) {
		strafeDirection += SYS.COMP_SETTINGS.KeySensitivity;
	}

	if (moveDirection != 0.0f || strafeDirection != 0.0f) {
		Camera.MoveAndStrafe(moveDirection, strafeDirection);
	}

	ShowCursor(SYS.COMP_SETTINGS.show);
}

void MouseButton(int Button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		switch (Button)
		{
		case GLUT_LEFT_BUTTON:
			Mouse.lmb = true;
			break;
		case GLUT_MIDDLE_BUTTON:
			Mouse.mmb = true;
			break;
		case GLUT_RIGHT_BUTTON:
			Mouse.rmb = true;
			break;
		}
	}
	else
	{
		switch (Button)
		{
		case GLUT_LEFT_BUTTON:
			Mouse.lmb = false;
			break;
		case GLUT_MIDDLE_BUTTON:
			Mouse.mmb = false;
			break;
		case GLUT_RIGHT_BUTTON:
			Mouse.rmb = false;
			break;
		}
	}
}