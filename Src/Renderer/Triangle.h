#pragma once
#include "Core/Constructors.h"

#include "Math/Math.h"
#include "Math/AABB.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"

#include "Util/Util.h"

struct Triangle {
	Vector3 position_0;
	Vector3 position_1;
	Vector3 position_2;

	Vector3 normal_0;
	Vector3 normal_1;
	Vector3 normal_2;

	Vector2 tex_coord_0;
	Vector2 tex_coord_1;
	Vector2 tex_coord_2;

	Triangle() = default;

	Triangle(
		Vector3 position_0,
		Vector3 position_1,
		Vector3 position_2,
		Vector3 normal_0,
		Vector3 normal_1,
		Vector3 normal_2,
		Vector2 tex_coord_0,
		Vector2 tex_coord_1,
		Vector2 tex_coord_2
	) :
		position_0(position_0),
		position_1(position_1),
		position_2(position_2),
		normal_0(normal_0),
		normal_1(normal_1),
		normal_2(normal_2),
		tex_coord_0(tex_coord_0),
		tex_coord_1(tex_coord_1),
		tex_coord_2(tex_coord_2)
	{
		// Check wheter any normal is invalid, if so replace it with the geometric normal of this Triangle
		bool normal_0_invalid = Math::approx_equal(Vector3::length(normal_0), 0.0f);
		bool normal_1_invalid = Math::approx_equal(Vector3::length(normal_1), 0.0f);
		bool normal_2_invalid = Math::approx_equal(Vector3::length(normal_2), 0.0f);

		if (normal_0_invalid || normal_1_invalid || normal_2_invalid) {
			Vector3 geometric_normal = Vector3::normalize(Vector3::cross(
				position_1 - position_0,
				position_2 - position_0
			));
			if (normal_0_invalid) this->normal_0 = geometric_normal;
			if (normal_1_invalid) this->normal_1 = geometric_normal;
			if (normal_2_invalid) this->normal_2 = geometric_normal;
		}

		fix_winding_order_if_needed();
	}

	DEFAULT_COPYABLE(Triangle);
	DEFAULT_MOVEABLE(Triangle);

	~Triangle() = default;

	void fix_winding_order_if_needed() {
		Vector3 geometric_normal = Vector3::normalize(Vector3::cross(
			position_1 - position_0,
			position_2 - position_0
		));
		// Check if the direction of the geometric normal does not agree with the shading normals
		bool all_normals_have_wrong_orientation =
			Vector3::dot(geometric_normal, normal_0) < 0.0f &&
			Vector3::dot(geometric_normal, normal_1) < 0.0f &&
			Vector3::dot(geometric_normal, normal_2) < 0.0f;
		bool some_normals_have_wrong_orientation =
			Vector3::dot(geometric_normal, normal_0) < 0.0f &&
			Vector3::dot(geometric_normal, normal_1) < 0.0f &&
			Vector3::dot(geometric_normal, normal_2) < 0.0f;

		if (all_normals_have_wrong_orientation) {
			// Reverse winding order
			Util::swap(position_1,  position_2);
			Util::swap(normal_1,    normal_2);
			Util::swap(tex_coord_1, tex_coord_2);
		} else {
			ASSERT(!some_normals_have_wrong_orientation);
		}
	}

	Vector3 get_center() const {
		return (position_0 + position_1 + position_2) / 3.0f;
	}

	AABB get_aabb() const {
		Vector3 vertices[3] = { position_0, position_1, position_2 };
		return AABB::from_points(vertices, 3);
	}
};
