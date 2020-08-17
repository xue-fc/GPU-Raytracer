#pragma once
#include <algorithm>

#include "BVH.h"
#include "BVHPartitions.h"

#include "Mesh.h"

struct BVHBuilder {
private:
	BVH * bvh = nullptr;

	int * indices_x = nullptr;
	int * indices_y = nullptr;
	int * indices_z = nullptr;
		
	float * sah  = nullptr;
	int   * temp = nullptr;
	
	template<typename Primitive>
	void build_bvh_recursive(BVHNode & node, const Primitive * primitives, int * indices[3], BVHNode nodes[], int & node_index, int first_index, int index_count, int max_primitives_in_leaf) {
		node.aabb = BVHPartitions::calculate_bounds(primitives, indices[0], first_index, first_index + index_count);
		
		if (index_count == 1) {
			// Leaf Node, terminate recursion
			node.first = first_index;
			node.count = index_count;

			return;
		}
		
		node.left = node_index;
		node_index += 2;
		
		int split_dimension;
		float split_cost;
		int split_index = BVHPartitions::partition_sah(primitives, indices, first_index, index_count, sah, split_dimension, split_cost);

		if (index_count <= max_primitives_in_leaf){
			// Check SAH termination condition
			float parent_cost = node.aabb.surface_area() * float(index_count); 
			if (split_cost >= parent_cost) {
				node.first = first_index;
				node.count = index_count;

				return;
			}
		}

		float split = primitives[indices[split_dimension][split_index]].get_center()[split_dimension];
		BVHPartitions::split_indices(primitives, indices, first_index, index_count, temp, split_dimension, split_index, split);

		node.count = (split_dimension + 1) << 30;

		int n_left  = split_index - first_index;
		int n_right = first_index + index_count - split_index;

		build_bvh_recursive(nodes[node.left    ], primitives, indices, nodes, node_index, first_index,          n_left,  max_primitives_in_leaf);
		build_bvh_recursive(nodes[node.left + 1], primitives, indices, nodes, node_index, first_index + n_left, n_right, max_primitives_in_leaf);
	}

	template<typename Primitive>
	inline void build_bvh_impl(const Primitive * primitives, int primitive_count, int max_primitives_in_leaf) {
		std::sort(indices_x, indices_x + primitive_count, [&](int a, int b) { return primitives[a].get_center().x < primitives[b].get_center().x; });
		std::sort(indices_y, indices_y + primitive_count, [&](int a, int b) { return primitives[a].get_center().y < primitives[b].get_center().y; });
		std::sort(indices_z, indices_z + primitive_count, [&](int a, int b) { return primitives[a].get_center().z < primitives[b].get_center().z; });
		
		int * indices_3[3] = { indices_x, indices_y, indices_z };
	
		int node_index = 2;
		build_bvh_recursive(bvh->nodes[0], primitives, indices_3, bvh->nodes, node_index, 0, primitive_count, max_primitives_in_leaf);

		assert(node_index <= 2 * primitive_count);

		bvh->node_count  = node_index;
		bvh->index_count = primitive_count;
	}

public:
	inline void init(BVH * bvh, int primitive_count) {
		this->bvh = bvh;

		indices_x = new int[primitive_count];
		indices_y = new int[primitive_count];
		indices_z = new int[primitive_count];

		for (int i = 0; i < primitive_count; i++) {
			indices_x[i] = i;
			indices_y[i] = i;
			indices_z[i] = i;
		}
			
		sah  = new float[primitive_count];
		temp = new int  [primitive_count];
		
		bvh->indices = indices_x;
		bvh->nodes   = new BVHNode[2 * primitive_count];
	}

	inline void free() {
		delete [] indices_y;
		delete [] indices_z;

		delete [] sah;
		delete [] temp;
	}
	
	inline void build(const Triangle * triangles, int triangle_count) {
		return build_bvh_impl(triangles, triangle_count, INT_MAX);
	}

	inline void build(const Mesh * meshes, int mesh_count) {
		return build_bvh_impl(meshes, mesh_count, 1);
	}
};