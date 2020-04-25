#include <sgw/sgw.h>
#include "game.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {

	constexpr sgw::game_parameters params {
		.sdl_lib_flags = sdl::lib::init_everything,
		.sdl_image_flags = IMG_INIT_PNG,
		.initial_window_title = "Sim Game",
		.window_x = sdl::window::position_centered,
		.window_y = sdl::window::position_centered,
		.window_w = 1280,
		.window_h = 720,
		.window_flags = SDL_WINDOW_ALLOW_HIGHDPI,
		.renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	};

	sim_game::game g(params);
	g.start();

	return 0;
}