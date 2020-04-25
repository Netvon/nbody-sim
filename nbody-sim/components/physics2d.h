#pragma once
#include <glm/glm.hpp>

namespace sim_game::components {

	template<typename Vector>
	constexpr Vector limit_length(const Vector& vector, float max_length) {
		float length = glm::length(vector);

		if (length > max_length) {
			return glm::normalize(vector) * max_length;
		}

		return vector;
	}

	template<typename T>
	struct tphysics2d {

		using type = tphysics2d<T>;
		using value_type = T;
		using vector_type = glm::vec<2, T, glm::packed_highp>;

		constexpr tphysics2d() = default;
		explicit constexpr tphysics2d(vector_type velocity) : m_velocity(velocity) {}
		constexpr tphysics2d(value_type x, value_type y, value_type mass) : m_velocity(x, y), m_mass(mass) {}
		constexpr tphysics2d(tphysics2d&&) noexcept = default;
		constexpr tphysics2d(const tphysics2d&) = default;
		constexpr tphysics2d& operator=(const tphysics2d&) = default;
		constexpr tphysics2d& operator=(tphysics2d&&) noexcept = default;
		~tphysics2d() = default;

		[[nodiscard]] constexpr const vector_type& get_velocity() const noexcept {
			return m_velocity;
		}

		constexpr void set_velocity(value_type x, value_type y) noexcept {
			m_velocity.x = x;
			m_velocity.y = y;

			limit_speed();
		}

		template <typename VectorType = vector_type>
		constexpr void set_velocity(const VectorType& velocity, bool enable_checks = true) noexcept {
			m_velocity.x = velocity.x;
			m_velocity.y = velocity.y;

			if (enable_checks) { limit_speed(); }
		}

		template <typename VectorType = vector_type>
		constexpr void add_velocity(const VectorType& velocity, bool enable_checks = true) noexcept {
			m_velocity += velocity;
			if (enable_checks) { limit_speed(); }
		}

		template <typename VectorType = vector_type>
		constexpr void subtract_velocity(const VectorType& velocity) noexcept {
			m_velocity -= velocity;
			limit_speed();
		}

		[[nodiscard]] constexpr value_type get_speed() const noexcept { return glm::length(m_velocity); }
		[[nodiscard]] constexpr value_type get_max_speed() const noexcept { return m_max_speed; }
		[[nodiscard]] constexpr value_type get_mass() const noexcept { return m_mass; }

		constexpr void set_mass(value_type mass) noexcept { m_mass = mass; }
		constexpr void set_max_speed(value_type max_speed) noexcept { m_max_speed = max_speed; }

	private:
		vector_type m_velocity{};
		value_type m_max_speed{ 100.F };
		value_type m_mass{ 1.F };

		constexpr void limit_speed() noexcept {
			m_velocity = limit_length(m_velocity, m_max_speed);
		}
	};

	using physics2d = tphysics2d<float>;
}