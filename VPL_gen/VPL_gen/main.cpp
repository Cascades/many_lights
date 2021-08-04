#include <fstream>
#include <many_lights/many_lights.h>
#include <memory>

#include <glm/gtx/string_cast.hpp>

#include <nanort.h>

#include "glm/gtc/random.hpp"

int main()
{
	std::cout << "hello" << std::endl;
    constexpr size_t max_lights = 50000;

    std::unique_ptr<ml::ManyLights<max_lights>> many_lights = std::make_unique<ml::ManyLights<max_lights>>();

	std::string input_model = std::string("../assets/sponza/sponza.obj");
	
    many_lights->add_model(input_model);
    many_lights->set_lights(20, 3.0f);

	std::cout << "started" << std::endl;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::vector<std::pair<size_t, size_t>> vertices_sizes;
    std::vector<std::pair<size_t, size_t>> indices_sizes;

	std::vector<size_t> original_vertex_index;

    size_t count_0 = 0;
	size_t count_1 = 0;

    size_t total_till_now = 0;
	
	for(auto const& model : many_lights->models->models)
	{
        for (auto const& mesh : model.meshes)
        {
            for (auto const& i : mesh.indices)
            {
				indices_sizes.emplace_back(count_0, count_1);
                indices.push_back(i + (vertices.size() / 3));
            }

			size_t count_2 = 0;
            for(auto const & v : mesh.vertices)
            {
				vertices_sizes.emplace_back(count_0, count_1);
				vertices_sizes.emplace_back(count_0, count_1);
				vertices_sizes.emplace_back(count_0, count_1);
                vertices.push_back(v.position.x);
                vertices.push_back(v.position.y);
                vertices.push_back(v.position.z);
				original_vertex_index.push_back(count_2);
				count_2++;
            }

			count_1++;
        }
        count_0++;
	}

    nanort::BVHBuildOptions<float> options; // Use default option
    nanort::TriangleMesh<float> triangle_mesh(vertices.data(), indices.data(), /* stride */sizeof(float) * 3);
    nanort::TriangleSAHPred<float> triangle_pred(vertices.data(), indices.data(), /* stride */sizeof(float) * 3);
    nanort::BVHAccel<float> accel;
    auto ret = accel.Build(indices.size() / 3, triangle_mesh, triangle_pred, options);
    assert(ret);
    nanort::BVHBuildStatistics stats = accel.GetStatistics();
    printf("  BVH statistics:\n");
    printf("    # of leaf   nodes: %d\n", stats.num_leaf_nodes);
    printf("    # of branch nodes: %d\n", stats.num_branch_nodes);
    printf("  Max tree depth   : %d\n", stats.max_tree_depth);

    const float tFar = 1.0e+30f;
	
    nanort::BVHTraceOptions trace_options;
    // Simple camera. change eye pos and direction fit to .obj model.

	std::vector<float> points;

	std::vector<glm::vec3> start_poses;
	std::vector<glm::vec3> start_vals;

	if (input_model == "../assets/sibenik/sibenik_medium.obj")
	{
		start_poses.push_back(glm::vec3(885.0f, 200.0f, 0.0f));
		start_poses.push_back(glm::vec3(-600.0f, -600.0f, 0.0f));
	}
	else if (input_model == "../assets/conference/conference.obj")
	{
		start_poses.push_back(glm::vec3(0.0f, 500.0f, 0.0f));
		start_poses.push_back(glm::vec3(500.0f, 500.0f, 0.0f));
		start_poses.push_back(glm::vec3(1000.0f, 500.0f, 0.0f));
	}
	else if (input_model == "../assets/sponza/sponza.obj")
	{
		start_poses.push_back(glm::vec3(0.0f, 500.0f, 0.0f));
	}

	glm::vec3 org = start_poses[0];

	if (input_model == "../assets/sibenik/sibenik_medium.obj")
	{
		start_vals.push_back(glm::vec3(3.0f));
		start_vals.push_back(glm::vec3(3.0f));
	}
	else if (input_model == "../assets/conference/conference.obj")
	{
		start_vals.push_back(glm::vec3(1.2f));
		start_vals.push_back(glm::vec3(1.2f));
		start_vals.push_back(glm::vec3(1.2f));
	}
	else if (input_model == "../assets/sponza/sponza.obj")
	{
		start_vals.push_back(glm::vec3(5.0f));
	}

	glm::vec3 org_value = start_vals[0];

	for (size_t curr_val_in = 0; curr_val_in < start_poses.size(); curr_val_in++)
	{
		points.push_back(start_poses[curr_val_in].x);
		points.push_back(start_poses[curr_val_in].y);
		points.push_back(start_poses[curr_val_in].z);

		points.push_back(start_vals[curr_val_in].r);
		points.push_back(start_vals[curr_val_in].g);
		points.push_back(start_vals[curr_val_in].b);
	}

	float atten_multiplier;

	if (input_model == "../assets/conference/conference.obj")
	{
		atten_multiplier = 20000.0f;
	}
	else if (input_model == "../assets/sibenik/sibenik_medium.obj")
	{
		atten_multiplier = 50000.0f;
	}
	else if (input_model == "../assets/sponza/sponza.obj")
	{
		atten_multiplier = 20000.0f;
	}
	

	size_t curr_index = 0;
	
	while (points.size() < 6 * 10000)
	{
		org = start_poses[curr_index];

		org_value = start_vals[curr_index];
		
		nanort::Ray<float> ray;
		ray.min_t = 0.0f;
		ray.max_t = tFar;
		ray.org[0] = org[0];
		ray.org[1] = org[1];
		ray.org[2] = org[2];
		glm::vec3 dir = glm::sphericalRand(1.0f);
		//std::cout << glm::to_string(dir) << std::endl;
		dir = glm::normalize(dir);
		ray.dir[0] = dir[0];
		ray.dir[1] = dir[1];
		ray.dir[2] = dir[2];

		nanort::TriangleIntersector<> triangle_intersecter(vertices.data(), indices.data(), /* stride */sizeof(float) * 3);
		nanort::TriangleIntersection<> isect{};

		size_t bounces = 0;
		size_t max_bounces;
		uint32_t attempts = 0;

		if (input_model == "../assets/conference/conference.obj")
		{
			max_bounces = 3;
		}
		else if (input_model == "../assets/sibenik/sibenik_medium.obj")
		{
			max_bounces = 30;
		}
		else if (input_model == "../assets/sponza/sponza.obj")
		{
			max_bounces = 3;
		}
		
		while (bounces < max_bounces && attempts < 50)
		{
			attempts++;

			bool hit = accel.Traverse(ray, triangle_intersecter, &isect, trace_options);

			glm::vec3 normal_avg;
			
			if (hit) {
				float new_t = isect.t * 0.999999999999f;

				//std::cout << new_t << " " << (new_t * new_t) << " " << 3.0f / (new_t * new_t) << std::endl;

				float atten = glm::min(atten_multiplier / (new_t * new_t), 0.7f);//(1.0f / (new_t * new_t));// *(glm::dot(org_value, glm::vec3(1.0)) / 3.0f);
				//float atten = 1.0f;

				auto& curr_mesh = many_lights->models->models[indices_sizes[3 * isect.prim_id + 0].first].meshes[indices_sizes[3 * isect.prim_id + 0].second];
				
				auto& curr_tex = curr_mesh.textures[0];

				/*if (bounces == 0)
				{
					original_bounce = curr_tex.path;
					if (original_bounce.filename().generic_string().find("curtain") == std::string::npos) {
						break;
					}*/
					/*if(original_bounce.filename().generic_string() != "sponza_curtain_green_diff.png")
					{
						dir = glm::sphericalRand(1.0f);

						ray.dir[0] = dir[0];
						ray.dir[1] = dir[1];
						ray.dir[2] = dir[2];
						continue;
					}*/
				/*}
				else
				{
					org_value *= 4.0f;
				}*/

				glm::uvec2 pixel_coords = curr_tex.size * glm::vec2(isect.u, isect.v);

				uint32_t base_index = (pixel_coords.y * curr_tex.size.x + pixel_coords.x) * curr_tex.channels;

				unsigned char col_r = curr_tex.image_data[base_index + 0];
				unsigned char col_g = curr_tex.image_data[base_index + 1];
				unsigned char col_b = curr_tex.image_data[base_index + 2];

				glm::vec3 float_col;

				float_col = (glm::vec3(col_r, col_g, col_b) / 255.0f) * org_value;

				auto const& vert_0_index = original_vertex_index[indices[3 * isect.prim_id + 0]];
				auto const& vert_1_index = original_vertex_index[indices[3 * isect.prim_id + 1]];
				auto const& vert_2_index = original_vertex_index[indices[3 * isect.prim_id + 2]];

				auto const& vert_0_color = curr_mesh.vertices[vert_0_index].color;
				auto const& vert_1_color = curr_mesh.vertices[vert_1_index].color;
				auto const& vert_2_color = curr_mesh.vertices[vert_2_index].color;

				glm::vec3 col_avg = (vert_0_color + vert_1_color + vert_2_color) / 3.0f;

				float_col *= col_avg;

				if ((glm::dot(atten * float_col * 2.0f, glm::vec3(1.0f)) / 3.0f) < 0.1f)
				{
					break;
				}
				points.push_back((org + new_t * dir).x);
				points.push_back((org + new_t * dir).y);
				points.push_back((org + new_t * dir).z);

				points.push_back(atten * float_col.r * 2.0f);
				points.push_back(atten * float_col.g * 2.0f);
				points.push_back(atten * float_col.b * 2.0f);

				org = (org + isect.t * dir * 0.9999f);

				ray.org[0] = org[0];
				ray.org[1] = org[1];
				ray.org[2] = org[2];

				auto const& vert_0_normal = curr_mesh.vertices[vert_0_index].normal;
				auto const& vert_1_normal = curr_mesh.vertices[vert_1_index].normal;
				auto const& vert_2_normal = curr_mesh.vertices[vert_2_index].normal;

				normal_avg = glm::normalize(vert_0_normal + vert_1_normal + vert_2_normal);

				if(glm::dot(normal_avg, dir) > 0.0f)
				{
					normal_avg *= -1;
				}
				
				dir = glm::sphericalRand(1.0f);
				while(glm::dot(dir, normal_avg) < 0)
				{
					dir = glm::sphericalRand(1.0f);
				}

				ray.dir[0] = dir[0];
				ray.dir[1] = dir[1];
				ray.dir[2] = dir[2];

				org_value = atten * float_col * 1.0f;

				bounces++;

				//std::cout << "tri: " << vertices[3 * indices[3 * isect.prim_id + 0]] << " " << vertices[3 * indices[3 * isect.prim_id + 0] + 1] << " " << vertices[3 * indices[3 * isect.prim_id + 0] + 2] << std::endl;
				//std::cout << "     " << vertices[3 * indices[3 * isect.prim_id + 1]] << " " << vertices[3 * indices[3 * isect.prim_id + 1] + 1] << " " << vertices[3 * indices[3 * isect.prim_id + 1] + 2] << std::endl;
				//std::cout << "     " << vertices[3 * indices[3 * isect.prim_id + 2]] << " " << vertices[3 * indices[3 * isect.prim_id + 2] + 1] << " " << vertices[3 * indices[3 * isect.prim_id + 2] + 2] << std::endl;
			}
			else
			{
				if(bounces == 0)
				{
					dir = glm::sphericalRand(1.0f);

					ray.dir[0] = dir[0];
					ray.dir[1] = dir[1];
					ray.dir[2] = dir[2];
				}
				else
				{
					dir = glm::sphericalRand(1.0f);
					while (glm::dot(dir, normal_avg) < 0)
					{
						dir = glm::sphericalRand(1.0f);
					}

					ray.dir[0] = dir[0];
					ray.dir[1] = dir[1];
					ray.dir[2] = dir[2];
				}
			}
		}

		curr_index = (curr_index + 1) % start_poses.size();
	}

	std::ofstream outFile("my_file.txt");
	// the important part
	for (const auto& e : points) outFile << e << "\n";
	
    return 0;
}
