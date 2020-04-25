#pragma once
#include <sgw/sgw.h>
#include <sgw/game.h>
#include <sgw/util/math.h>
#include <vector>
#include <map>
#include <fmt/format.h>
#include "../components/physics2d.h"
#include "../components/camera_focus.h"

namespace sim_game::systems {
	struct ui_info_system {
		using tranform2d = sgw::components::transform2d;
		using physics2d = components::physics2d;

		void update(sgw::game& game) {
			const auto& renderer = game.get_renderer();
			renderer.copy_f(m_help_texture, SDL_FPoint{ 10.F, 10.F });

			if (!m_show_ui) {
				return;
			}

			auto& registry = game.get_entity_registry();
			const auto& fm = game.get_font_manager();
			const auto& font = fm.get_font(m_font_key);
			auto [mouse_x, mouse_y] = game.get_mouse_position();
			auto help_size = m_help_texture.get_size<glm::vec2>();

			float offset_y = 14.F + help_size.y;
			bool any_mouse_over = false;


			for (std::size_t i = 0; i < m_info_textures.size(); i++)
			{
				const auto& texture = m_info_textures.at(i);
				const auto& entity = m_info_entt_map.at(i);

				auto [w, h] = texture.get_size();

				auto info_pos = glm::vec2(10.F, offset_y);

				const auto& transform = registry.get<tranform2d>(entity);
				const auto& physics = registry.get<physics2d>(entity);
				const auto& color = registry.get<SDL_Color>(entity);

				auto speed_str = fmt::format("Speed: {0:>6.2F}m/s [{1:>7.1F};{2:>7.1F}]", physics.get_speed(), transform.get_position().x, transform.get_position().y);

				auto speed_texture = renderer.create_texture_from_surface(
					font.render_solid(speed_str, color));

				auto [sw, sh] = speed_texture.get_size();
				auto speed_pos = glm::vec2(20.F + static_cast<float>(w), offset_y);

				auto mouse_over = sgw::math::in_rect(
					glm::vec2{ mouse_x, mouse_y }, 
					glm::vec4{ info_pos.x, info_pos.y - 5.F, static_cast<float>(sw + w) + 20.F,  static_cast<float>(sh) + 5.F });

				auto info_guard = texture.get_alpha_mod_guard();
				auto speed_guard = speed_texture.get_alpha_mod_guard();

				if (mouse_over) {

					auto midpoint = renderer.get_output_size_f<glm::vec2>() * 0.5F;
					auto focus_id = registry.view<components::camera_focus>().front();
					auto camera_id = registry.view<components::camera>().front();
					const auto& focus_transform = registry.get<tranform2d>(focus_id);
					const auto& camera_transform = registry.get<tranform2d>(camera_id);

					auto camera_offset = midpoint - camera_transform.get_position();

					auto offset = (midpoint - camera_offset) - focus_transform.get_position();


					const auto& target_pos = transform.get_position() + offset;

					renderer.draw_line_f(
						SDL_FPoint{ speed_pos.x + static_cast<float>(sw) + 10.F, speed_pos.y + (static_cast<float>(sh) * 0.5F) },
						SDL_FPoint{ target_pos.x, target_pos.y },
						SDL_Color{ color.r, color.g, color.b, 64 });

					m_mouse_over = entity;
					any_mouse_over = true;
				}
				else {
					texture.set_alpha_mod(128);
					speed_texture.set_alpha_mod(128);
				}

				//renderer.draw_rect_f(SDL_FRect{ 
				//	info_pos.x - 5.f, info_pos.y - 2.5F,
				//	static_cast<float>(sw + w) + 20.F,  static_cast<float>(sh) + 5.F }, SDL_Color{ 255, 255, 255, 32 });

				renderer.copy_f(texture, info_pos);
				renderer.copy_f(speed_texture, speed_pos);

				offset_y += static_cast<float>(h) + 5.F;

				if (!any_mouse_over) {
					m_mouse_over = entt::null;
				}
			}


			//renderer.copy(m_hw_texture, SDL_Point{ 10, 10 });
		}

		void handle_event(sgw::game& game, SDL_Event event) {

			if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_H) {
				m_show_ui = !m_show_ui;
			}
		}

		void on_object_added(entt::registry& registry, entt::entity id) {
			
			const auto& fm = m_game->get_font_manager();
			const auto& renderer = m_game->get_renderer();
			const auto& font = fm.get_font(m_font_key);

			const auto& physics = registry.get<physics2d>(id);

			auto str = fmt::format("Mass: {0:>7.2e}kg", physics.get_mass());

			m_info_entt_map.insert_or_assign(m_info_textures.size(), id);

			m_info_textures.emplace_back(renderer.create_texture_from_surface(
				font.render_blended(str, SDL_Color{ 255, 255, 255, 255 })));

		}

		void on_focus_added([[maybe_unused]] entt::registry& registry, entt::entity id) {
			m_camera_focus = id;
		}

		void setup(sgw::game& game) {

			auto dpi_info = sdl::lib::get_display_dpi(0);
			m_scale = std::get<2>(dpi_info) / 96.F;

			m_game = &game;

			auto& fm = game.get_font_manager();
			const auto& renderer = game.get_renderer();

			m_font_key = fm.add_font("assets/fonts/consola.ttf", "consolas", 10 * static_cast<int>(m_scale));
			const auto& font = fm.get_font(m_font_key);

			m_help_texture = renderer.create_texture_from_surface(
				font.render_solid("Press [H] to toggle UI", SDL_Color{ 255, 255, 255, 255 }));
			m_help_texture.set_alpha_mod(128);

			game.get_entity_registry().on_construct<physics2d>().connect<&ui_info_system::on_object_added>(*this);
			game.get_entity_registry().on_construct<components::camera_focus>().connect<&ui_info_system::on_focus_added>(*this);
		}

		
	private:
		sgw::font_manager::key m_font_key;
		sdl::texture m_help_texture;

		std::vector<sdl::texture> m_info_textures;
		std::map<std::size_t, entt::entity> m_info_entt_map;
		sgw::game* m_game = nullptr;
		entt::entity m_mouse_over = entt::null;
		entt::entity m_camera_focus = entt::null;

		bool m_show_ui = true;
		float m_scale = 1.F;
	};


}