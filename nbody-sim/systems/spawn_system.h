#pragma once
#include <sgw/sgw.h>
#include <sgw/game.h>
#include "../components/physics2d.h"
#include "../components/camera_focus.h"
#include "../components/camera.h"

namespace sim_game::systems {
	struct spawn_system {
		static constexpr float default_velocity = -20.F;
		static constexpr float default_heavy_mass = 1.E15F;
		static constexpr std::size_t default_spawn_amount = 24;

		using tranform2d = sgw::components::transform2d;
		using physics2d = components::physics2d;

	private:

		[[nodiscard]] float get_random_spawn_offset() const {
			return static_cast<float>(sgw::random::next(0llu, m_spawn_amount));
		}
	public:

		void update(sgw::game& game) {
			auto& registry = game.get_entity_registry();
			
			check_despawn_and_update_midpoint(game, registry);
		}

		void setup(sgw::game& game) {
			const auto& renderer = game.get_renderer();

			m_midpoint = renderer.get_output_size_f<glm::vec2>() * 0.5F;

			spawn_heavy_body(game.get_entity_registry());

			for (std::size_t i = 0; i < m_spawn_amount; i++)
			{
				spawn_body(game.get_entity_registry(), static_cast<float>(i));
			}

			spawn_camera(game.get_entity_registry());
		}

		void handle_event(sgw::game& game, SDL_Event event) {
			if (event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
				spawn_body(game.get_entity_registry(), get_random_spawn_offset());
			}

			if (event.type == SDL_KEYUP && event.key.keysym.scancode == SDL_SCANCODE_R) {
				auto& physics = game.get_entity_registry().get<physics2d>(m_heavy);
				auto& transform = game.get_entity_registry().get<tranform2d>(m_heavy);

				m_midpoint = game.get_renderer().get_output_size_f<glm::vec2>() * 0.5F;

				physics.set_velocity(0.F, 0.F);
				transform.set_position(m_midpoint);
			}

			if (event.type == SDL_MOUSEMOTION && event.motion.state & SDL_BUTTON_LMASK) {
				auto& transform = game.get_entity_registry().get<tranform2d>(m_camera);

				transform.add_position(glm::vec2(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel)));
			}
		}

	private:

		void spawn_camera(entt::registry& registry) {
			m_camera = registry.create();
			registry.assign<tranform2d>(m_camera, m_midpoint.x, m_midpoint.y);
			registry.assign<components::camera>(m_camera);
		}

		void spawn_heavy_body(entt::registry& registry) {
			m_heavy = registry.create();

			registry.assign<tranform2d>(m_heavy, m_midpoint.x, m_midpoint.y);
			registry.assign<physics2d>(m_heavy, 0.F, 0.F, default_heavy_mass);
			registry.assign<SDL_Color>(m_heavy, SDL_Color{ 255, 255, 255, 255 });
			registry.assign<components::camera_focus>(m_heavy);

			//auto heavy_2 = registry.create();

			//registry.assign<tranform2d>(heavy_2, m_midpoint.x + 200.F, m_midpoint.y);
			//registry.assign<physics2d>(heavy_2, 0.F, -20.F, default_heavy_mass);
			//registry.assign<SDL_Color>(heavy_2, SDL_Color{ 255, 255, 255, 255 });
		}

		void spawn_body(entt::registry& registry, float offset_location_from_midpoint, entt::entity entity = entt::null) {
			auto random_r = static_cast<unsigned char>(sgw::random::next(100, 255));
			auto random_g = static_cast<unsigned char>(sgw::random::next(100, 255));
			auto random_b = static_cast<unsigned char>(sgw::random::next(100, 255));

			auto random_mass = sgw::random::next(1.F, default_heavy_mass * 0.001F);

			auto random_bool_1 = static_cast<bool>(sgw::random::next(0, 1));
			auto velo = m_velocity;
			auto spawn_point = glm::vec2((30.F + m_midpoint.x) + (10.F * offset_location_from_midpoint), m_midpoint.y);

			if (random_bool_1) {

				velo *= -1.F;
				spawn_point = glm::vec2((m_midpoint.x - 30.F) - (10.F * offset_location_from_midpoint), m_midpoint.y);
			}

			if (!registry.valid(entity)) {
				entity = registry.create();
			}

			registry.assign_or_replace<tranform2d>(entity, spawn_point.x, spawn_point.y);
			registry.assign_or_replace<physics2d>(entity, 0.F, velo, random_mass);
			registry.assign_or_replace<SDL_Color>(entity, SDL_Color{ random_r, random_g, random_b, 255 });
			//registry.assign_or_replace<components::camera_focus>(entity);
		}

		void check_despawn_and_update_midpoint(sgw::game& game, entt::registry& registry) {

			auto [w, h] = game.get_renderer().get_output_size_f();

			const auto& heavy_pos = registry.get<tranform2d>(m_heavy);
			m_midpoint = heavy_pos.get_position();

			registry.view<tranform2d, physics2d>().each([&](const entt::entity e, const tranform2d& t, const physics2d& p) {

				auto pos = t.get_position() - heavy_pos.get_position();

				//if (e == m_heavy) {
				//	m_midpoint = pos;
				//}

				if (pos.x > ( w * 2.F ) || pos.x < -( w * 2.F ) || pos.y > ( h * 2.F ) || pos.y < -(h * 2.F)) {

					//if (e == m_heavy) {
					//	p.set_velocity(0.F, 0.F);
					//	t.set_position(m_midpoint);
					//}
					//else {
					spawn_body(registry, get_random_spawn_offset(), e);
					//}
				}
			});
		}

		

		sdl::texture m_texture_trail;
		sdl::texture m_texture_circle;

		glm::vec2 m_midpoint;
		float m_velocity = default_velocity;
		std::size_t m_spawn_amount = default_spawn_amount;
		entt::entity m_heavy;
		entt::entity m_camera;
	};
}