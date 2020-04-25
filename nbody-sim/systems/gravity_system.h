#pragma once
#include <sgw/sgw.h>
#include <sgw/game.h>
#include <glm/gtx/norm.hpp>
#include "../components/physics2d.h"
#include <map>
#include <vector>

namespace sim_game::systems {
	struct gravity_system {
		using tranform2d = sgw::components::transform2d;
		using physics2d = sim_game::components::physics2d;

		static void update(sgw::game& game) {

			auto dt = game.get_delta_time();
			auto& registry = game.get_entity_registry();

			auto all_bodies_view = registry.view<tranform2d, physics2d>();

			all_bodies_view.each([&](entt::entity main_id, const tranform2d& main_t, physics2d& main_p) {

				auto all_bodies = registry.view<tranform2d, physics2d>();

				all_bodies.each([&](entt::entity other_id, const tranform2d& other_t, physics2d& other_p) {

					if (main_id != other_id) {
						auto distance_sqr = glm::length2(other_t.get_position() - main_t.get_position());

						if (distance_sqr < 2.5f) {
							return;
						}

						auto force_direction = glm::normalize(other_t.get_position() - main_t.get_position());
						auto force = force_direction * 0.000000000066742F * main_p.get_mass() * other_p.get_mass() / distance_sqr;
						auto acceleration = force / main_p.get_mass();

						main_p.add_velocity(acceleration * dt, false);
					}

				});

			});

			all_bodies_view.each([&](tranform2d& main_t, const physics2d& main_p) {
				main_t.add_position(main_p.get_velocity() * dt);
			});
		}


		static auto precalculate_orbit(sgw::game& game, std::size_t itterations, float dt) {

			std::size_t done = 0;

			entt::registry copy;
			std::map<entt::entity, std::vector<SDL_FPoint>> entity_points;

			auto& registry = game.get_entity_registry();

			registry.view<tranform2d, physics2d>().each([&](entt::entity main_id, const tranform2d& main_t, physics2d& main_p) {
				auto id = copy.create(main_id);

				copy.assign<tranform2d>(id, main_t);
				copy.assign<physics2d>(id, main_p);

				entity_points.insert_or_assign(id, std::vector<SDL_FPoint>());
			});

			

			while (done < itterations) {

				auto all_bodies_view = copy.view<tranform2d, physics2d>();

				all_bodies_view.each([&](entt::entity main_id, const tranform2d& main_t, physics2d& main_p) {

					auto all_bodies = copy.view<tranform2d, physics2d>();

					all_bodies.each([&](entt::entity other_id, const tranform2d& other_t, physics2d& other_p) {

						if (main_id != other_id) {
							auto distance_sqr = glm::length2(other_t.get_position() - main_t.get_position());
							auto force_direction = glm::normalize(other_t.get_position() - main_t.get_position());
							auto force = force_direction * 20.F * main_p.get_mass() * other_p.get_mass() / distance_sqr;
							auto acceleration = force / main_p.get_mass();

							main_p.add_velocity(acceleration * dt, false);
						}

						});

					});

				all_bodies_view.each([&](entt::entity main_id, tranform2d& main_t, const physics2d& main_p) {
					main_t.add_position(main_p.get_velocity() * dt);

					auto& vector = entity_points.at(main_id);
					const auto& pos = main_t.get_position();

					vector.push_back(SDL_FPoint{ pos.x, pos.y });
				});

				done++;
			}

			return entity_points;
		}
	};
}