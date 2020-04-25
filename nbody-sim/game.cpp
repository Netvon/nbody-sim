#include "game.h"

namespace sim_game {

	void game::game_logic() {
		m_gravity_system.update(*this);
		m_spawn_system.update(*this);
	}

	void game::game_draw([[maybe_unused]] const sdl::renderer& renderer)
	{
		m_points_render_system.update(*this);
		m_ui_info_system.update(*this);
	}

	void game::handle_event(SDL_Event event)
	{
		m_spawn_system.handle_event(*this, event);
		m_ui_info_system.handle_event(*this, event);
	}

	void game::game_preload()
	{
		m_ui_info_system.setup(*this);
		m_spawn_system.setup(*this);
		m_points_render_system.setup(*this);
	}
}
