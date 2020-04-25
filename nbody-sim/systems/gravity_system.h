#pragma once
#include <sgw/sgw.h>
#include <sgw/game.h>
#include <glm/gtx/norm.hpp>
#include <execution>
#include "../components/physics2d.h"

namespace sim_game::systems {
	struct gravity_system {
		using tranform2d = sgw::components::transform2d;
		using physics2d = sim_game::components::physics2d;

		constexpr static physics2d::value_type default_g_constant{ 0.000000000066742F };
		constexpr static physics2d::value_type default_min_distance_for_acceleration{ 2.5F };

		void update(sgw::game& game) {

			auto dt = game.get_delta_time();
			auto& registry = game.get_entity_registry();

			update_velocities(registry, dt);
			update_positions(registry, dt);
		}

		[[nodiscard]] physics2d::vector_type calculate_acceleration(
			const tranform2d& body_b_transform,
			const tranform2d& body_a_transform,
			const physics2d& body_a_physics,
			const physics2d& body_b_physics) const
		{
			const auto& body_a_pos = body_a_transform.get_position();
			const auto& body_b_pos = body_b_transform.get_position();

			auto distance = body_b_pos - body_a_pos;
			auto distance_sqr = glm::length2(distance);

			if (distance_sqr < m_min_distance_for_acceleration) {
				return physics2d::vector_type();
			}

			auto force_direction = glm::normalize(distance);
			auto force = force_direction * m_g_constant * body_a_physics.get_mass() * body_b_physics.get_mass() / distance_sqr;
			auto acceleration = force / body_a_physics.get_mass();

			return acceleration;
		}

		[[nodiscard]] physics2d::value_type get_g_constant() const noexcept {
			return m_g_constant;
		}

		void set_g_constant(physics2d::value_type g) noexcept {
			m_g_constant = g;
		}

		[[nodiscard]] physics2d::value_type get_min_distance_for_acceleration() const noexcept {
			return m_min_distance_for_acceleration;
		}

		void set_min_distance_for_acceleration(physics2d::value_type min_distance_for_acceleration) noexcept {
			m_min_distance_for_acceleration = min_distance_for_acceleration;
		}

	private:
		physics2d::value_type m_g_constant = default_g_constant;
		physics2d::value_type m_min_distance_for_acceleration = default_min_distance_for_acceleration;

		void update_velocities(entt::registry& registry, float dt) {

			auto bodies = registry.view<tranform2d, physics2d>();

			std::for_each(std::execution::par_unseq, bodies.begin(), bodies.end(), [&](entt::entity body_a) {

				const auto& main_t = bodies.get<tranform2d>(body_a);
				auto& main_p = bodies.get<physics2d>(body_a);

				std::mutex m;

				std::for_each(std::execution::par, bodies.begin(), bodies.end(), [&](entt::entity body_b) {
					if (body_b == body_a) {
						return;
					}

					const auto& other_t = bodies.get<tranform2d>(body_b);
					const auto& other_p = bodies.get<physics2d>(body_b);

					auto acceleration = calculate_acceleration(other_t, main_t, main_p, other_p);

					std::lock_guard<std::mutex> guard(m);
					main_p.add_velocity(acceleration * dt, false);
				});
			});
		}

		void update_positions(entt::registry& registry, float dt) {
			auto bodies = registry.view<tranform2d, physics2d>();

			std::for_each(std::execution::par_unseq, bodies.begin(), bodies.end(), [&bodies, dt](auto entity) {

				auto& t = bodies.get<tranform2d>(entity);
				const auto& p = bodies.get<physics2d>(entity);

				t.add_position(p.get_velocity() * dt);
			});
		}
	};
}