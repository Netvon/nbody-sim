#pragma once
#include <thread>
#include <functional>
#include <sgw/sgw.h>
#include <sgw/game.h>
#include "gravity_system.h"
#include <mutex>
#include <shared_mutex>
#include <array>
#include "../components/camera_focus.h"
#include "../components/camera.h"

namespace sim_game::systems {
	struct point_render_system {
		using tranform2d = sgw::components::transform2d;

		void update(sgw::game& game) {
			auto& registry = game.get_entity_registry();
			const auto& renderer = game.get_renderer();

			auto midpoint = (renderer.get_output_size_f<glm::vec2>() * 0.5F) / m_scale;
			auto focus_id = registry.view<components::camera_focus>().front();
			auto camera_id = registry.view<components::camera>().front();
			const auto& focus_transform = registry.get<tranform2d>(focus_id);
			const auto& camera_transform = registry.get<tranform2d>(camera_id);

			auto camera_offset = midpoint - camera_transform.get_position();

			auto offset = (midpoint - camera_offset) - focus_transform.get_position();

			renderer.copy_f(m_texture_trail, SDL_FPoint{ offset.x, offset.y });


			registry.view<tranform2d, SDL_Color>().each([&](const tranform2d& t, const SDL_Color& c) {
				const auto& pos = t.get_position();
				auto guard = m_texture_circle.get_color_mod_guard();
				m_texture_circle.set_color_mod(c);

				renderer.copy_ex_f(m_texture_circle, pos + offset, 0.F, glm::vec2(0.05F, 0.05F) * m_scale);

				renderer.set_render_target(m_texture_trail);
				renderer.copy_ex_f(m_texture_circle, pos, 0.F, glm::vec2(0.05F, 0.05F) * m_scale);
				renderer.set_default_render_target();
			});

			renderer.set_render_target(m_texture_trail);

			renderer.copy(m_texture_black);
			renderer.set_default_render_target();

		}

		void setup(sgw::game& game) {
			const auto& renderer = game.get_renderer();

			auto dpi_info = sdl::lib::get_display_dpi(0);
			m_scale = std::get<2>(dpi_info) / 96.F;

			auto [w, h] = renderer.get_output_size();
			m_texture_trail = renderer.create_texture(SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
			m_texture_trail.set_blend_mode(SDL_BLENDMODE_BLEND);

			renderer.set_blend_mode(SDL_BLENDMODE_BLEND);
			renderer.set_render_target(m_texture_trail);
			renderer.set_draw_color(SDL_Color{ 12, 12, 12, 255 });
			renderer.clear();
			renderer.set_default_render_target();

			auto circle_image = sgw::image_manager::load_image("assets/textures/circle_smooth_large.png");
			m_texture_circle = renderer.create_texture_from_surface(circle_image);
			m_texture_circle.set_blend_mode(SDL_BLENDMODE_BLEND);

			auto black_image = sgw::image_manager::load_image("assets/textures/black.png");
			m_texture_black = renderer.create_texture_from_surface(black_image);
			m_texture_black.set_blend_mode(SDL_BLENDMODE_BLEND);
			m_texture_black.set_color_mod(SDL_Color{ 0, 0, 0, 255 });
			m_texture_black.set_alpha_mod(10);
		}

	private:
		sdl::texture m_texture_trail;
		sdl::texture m_texture_circle;
		sdl::texture m_texture_black;

		float m_scale = 1.F;

	};


}