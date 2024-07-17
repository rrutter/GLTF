#ifndef GLOBALS
#define GLOBALS

#define WINDOW_TITLE "Learning GLTF"
#define DEFAULT_WINDOW_WIDTH 1200
#define DEFAULT_WINDOW_HEIGHT 900
#define SHADER_LOCATION = "shaders/"
#define ASSETS_DIRECTORY "assets/"
#define GLM_ENABLE_EXPERIMENTAL

#ifndef M_PI // Check if M_PI is already defined
#define M_PI 3.1415926535897931
#endif

#ifndef GET_RADIANS
#define GET_RADIANS(degree) (float)((degree * M_PI) / 180.0f)
#endif
#pragma once
#endif // !GLOBALS