#include "Globals.h"
#include "GameLoop.h"

PersonalGL pGL;



int main(int argc, char** argv)
{
	GameLoop game;
	game.initialize(argc, argv);
	return 0;
}