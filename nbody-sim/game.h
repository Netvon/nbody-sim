#pragma once
#include <sgw/game.h>
#include <sgw/sgw.h>
#include "components/physics2d.h"
#include "systems/point_render_system.h"
#include "systems/gravity_system.h"
#include "systems/spawn_system.h"
#include "systems/ui_info_system.h"

namespace sim_game {
	struct game : sgw::game {
		using tranform2d = sgw::components::transform2d;
		using physics2d = components::physics2d;

		explicit game(sgw::game_parameters params) : sgw::game(params) {}
		void game_logic() override;
		void game_draw(const sdl::renderer& renderer) override;
		void handle_event(SDL_Event event) override;
		void game_preload() override;

	private:
		systems::point_render_system m_points_render_system;
		systems::spawn_system m_spawn_system;
		systems::ui_info_system m_ui_info_system;
		glm::vec2 m_midpoint;
	};
}