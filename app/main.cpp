#define SDL_MAIN_HANDLED
#include "SDL.h"
#include "chip-8.h"

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return -1;
	}

	chipotto::Emulator emulator;

	if (emulator.IsValid())
	{
		emulator.LoadFromFile("D:\\AIV\\Terzo anno\\C++\\c8games\\PONG");
		while (true)
		{
			if (!emulator.Tick())
			{
				break;
			}
		}
	}

	SDL_Quit();
	return 0;
}