#pragma once
#include <random>

#include "pbt/PBT.h"

#include <glm/gtx/norm.hpp>

// Marsaglia's xorshf
// very fast random integer generation
class XorshfRand
{
	uint32_t rand_x = 123456789;
	uint32_t rand_y = 362436069;
	uint32_t rand_z = 521288629;

public:
	uint32_t xorshf96()
	{
		rand_x ^= rand_x << 16;
		rand_x ^= rand_x >> 5;
		rand_x ^= rand_x << 1;

		uint32_t rand_t = rand_x;
		rand_x = rand_y;
		rand_y = rand_z;
		rand_z = rand_t ^ rand_x ^ rand_y;

		return rand_z;
	}
};

// LightCuts algorithm
template <typename SpaceT, typename LightT, size_t max_lights>
class LightCut
{
	XorshfRand rand_gen;

public:
	LightCut() = default;
	~LightCut() = default;
	LightCut(LightCut const& other) = default;
	LightCut& operator=(LightCut const& other) = default;
	LightCut(LightCut&& other) = default;
	LightCut& operator=(LightCut&& other) = default;

	// used to weight light intensity based on position in scene
	//float geometry_term(glm::vec<3, SpaceT> const& in_pos, glm::vec<3, SpaceT> const& light_pos) const
	//{
	//	return 1 / glm::distance2(in_pos, light_pos);
	//}

	// used to weight light intensity based on BDRF value
	//float material_term(glm::vec<3, SpaceT> const& in_pos, glm::vec<3, SpaceT> const& norm, glm::vec<3, SpaceT> const& light_pos, UBO const& ubo) const
	//{
	/*	// blinn phong
		glm::vec3 light_dir = glm::normalize(in_pos - light_pos);

		float ambient = 0.1f;
		float diffuse = 0.9f * glm::max(0.0f, glm::dot(norm, -light_dir));

		float specular = 0.0;
		if (diffuse != 0.0f)
		{
			glm::vec3 camera_dir = glm::normalize(in_pos - glm::vec3(-2.0, 0.0, 0.0));
			glm::vec3 reflection_dir = glm::normalize(glm::reflect(light_dir, norm));

			float spec_val = glm::pow(glm::max(glm::dot(reflection_dir, -camera_dir), 0.0f), 0.2f);
			specular = glm::clamp(0.1f * spec_val, 0.0f, 1.0f);
		}

		glm::vec3 col = glm::clamp(ubo.Ke + ubo.color * (ambient * ubo.Ka + diffuse * ubo.Kd + specular * ubo.Ks), glm::vec3(0.0f), glm::vec3(1.0f));

		return (col.r + col.g + col.b) / 3.0f;
	}

	// combination of BRDF and position terms
	float reflectance_bound(PBT<SpaceT, LightT, max_lights>& pbt, glm::vec<3, SpaceT> const& in_pos, glm::vec<3, SpaceT> const& norm, glm::vec<3, SpaceT> const& light_pos, UBO const& ubo) const
	{
		return geometry_term(in_pos, light_pos) * material_term(in_pos, norm, light_pos, ubo);
	}

	float calculate_min_dist(PBT<SpaceT, LightT, max_lights>& pbt, glm::vec<3, SpaceT> const& scene_point, size_t const & current_index) const
	{
		return glm::distance2(
			glm::clamp(
				scene_point,
				pbt[pbt.get_first_child_index(current_index)].bb.min_bounds,
				pbt[pbt.get_first_child_index(current_index)].bb.max_bounds),
			scene_point
		); // d_[left/right]_min(x) ^ 2
	}

	float calculate_max_dist(PBT<SpaceT, LightT, max_lights>& pbt, glm::vec<3, SpaceT> const& scene_point, size_t const& current_index) const
	{
		return glm::distance2(
			glm::max(
				pbt[pbt.get_first_child_index(current_index)].bb.max_bounds - scene_point,
				pbt[pbt.get_first_child_index(current_index)].bb.min_bounds),
			scene_point
		); // d_[left/right]_max(x) ^ 2
	}*/

	// generates a light cut, and output to out_cut vector
	/*void generate_cut(PBT<SpaceT, LightT, max_lights>& pbt, size_t const& sample_count, glm::vec<3, SpaceT> const& scene_point, UBO const& ubo, std::vector<size_t>& out_cut)
	{
		// setup output vector
		out_cut.clear();
		out_cut.reserve(sample_count);

		// lightcuts uses a hand picked number of samples to take
		while(out_cut.size() < sample_count)
		{
			// start at root node
			size_t current_index = 0;

			// until we reach a leaf node
			while(!pbt.node_is_leaf(current_index))
			{
				// calculate light intensity at point in space from left and right child nodes 
				auto left_reflectance_bound = reflectance_bound(pbt,
					scene_point,
					(pbt[pbt.get_first_child_index(current_index)].bb.max_bounds + pbt[pbt.get_first_child_index(current_index)].bb.min_bounds) / 2.0f,
					glm::normalize(glm::vec<3, SpaceT>(0.91f, 0.875f, 0.234f)),
					ubo); // F(x, w)

				auto right_reflectance_bound = reflectance_bound(pbt,
					scene_point,
					(pbt[pbt.get_first_child_index(current_index) + 1].bb.max_bounds + pbt[pbt.get_first_child_index(current_index) + 1].bb.min_bounds) / 2.0f,
					glm::normalize(glm::vec<3, SpaceT>(0.91f, 0.875f, 0.234f)),
					ubo); // F(x, w)

				auto left_intensity = pbt[pbt.get_first_child_index(current_index)].total_intensity; // I_left
				auto right_intensity = pbt[pbt.get_first_child_index(current_index) + 1].total_intensity; // I_right

				// calculate weightings from intensities
				auto w_min_left = left_reflectance_bound;
				w_min_left *= left_intensity;
				w_min_left /= calculate_min_dist(pbt, scene_point, pbt.get_first_child_index(current_index));
				
				auto w_min_right = right_reflectance_bound;
				w_min_right *= right_intensity;
				w_min_right /= calculate_min_dist(pbt, scene_point, pbt.get_first_child_index(current_index) + 1);
				
				auto w_max_left = left_reflectance_bound;
				w_max_left *= left_intensity;
				w_max_left /= calculate_max_dist(pbt, scene_point, pbt.get_first_child_index(current_index));
				
				auto w_max_right = right_reflectance_bound;
				w_max_right *= right_intensity;
				w_max_right /= calculate_max_dist(pbt, scene_point, pbt.get_first_child_index(current_index) + 1);

				// calculate probabilities from weightings
				float p_min_left;
				if(w_min_left <= 0 || w_min_right <= 0)
				{
					p_min_left = 0.0f;
				}
				else
				{
					p_min_left = w_min_left / (w_min_left + w_min_right);
				}

				float p_max_left;
				if (w_max_left <= 0 || w_max_right <= 0)
				{
					p_max_left = 0.0f;
				}
				else
				{
					p_max_left = w_max_left / (w_max_left + w_max_right);
				}

				// create variable required to calculate final node to choose
				auto const prob_to_beat = static_cast<uint32_t>(4294967295.0f * (p_min_left + p_max_left / 2.0f));
				auto const curr_prob = rand_gen.xorshf96();

				// set next node as the probability sees fit
				if (curr_prob > prob_to_beat)
				{
					current_index = pbt.get_first_child_index(current_index);
				}
				else
				{
					current_index = pbt.get_first_child_index(current_index) + 1;
				}
			}

			// output final light to output vector
			out_cut.emplace_back(pbt.get_first_child_index(current_index));
		}		
	}*/
};
