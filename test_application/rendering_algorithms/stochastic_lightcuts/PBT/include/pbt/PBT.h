#pragma once
#include <vector>
#include <bit>
#include <algorithm>
#include <iomanip>
#include <execution>

#include "glm/glm.hpp"

#include "many_lights/scene_lights.h"
#include "morton_code.h"

// Container for bounding box
template <typename SpaceT>
struct PBTBoundingBox
{
    glm::vec<4, SpaceT> min_bounds = glm::vec<4, SpaceT>(0.0f);
    glm::vec<4, SpaceT> max_bounds = glm::vec<4, SpaceT>(0.0f);
};

// Container for PBT node including:
// * Bouding box
// * Total light intensity
template <typename SpaceT, typename LightT>
struct PBTNode
{
    PBTBoundingBox<SpaceT> bb;
    glm::vec4 total_intensity = glm::vec4(0.0f);
    glm::ivec4 original_index = glm::vec4(-1, 0, 0, 0);

    PBTNode<SpaceT, LightT> operator+(PBTNode<SpaceT, LightT> const& right)
    {
    	if(right.total_intensity == glm::vec4(0.0))
    	{
            return *this;
    	}
        if (this->total_intensity == glm::vec4(0.0))
    	{
            return right;
    	}
    	
        PBTNode<SpaceT, LightT> return_val{};
        return_val.total_intensity = this->total_intensity + right.total_intensity;
        return_val.bb.min_bounds = glm::min(this->bb.min_bounds, right.bb.min_bounds);
        return_val.bb.max_bounds = glm::max(this->bb.max_bounds, right.bb.max_bounds);
        return return_val;
    }
};

// Perfect binary tree class following Real-Time Stochastic LightCuts
template <typename SpaceT, typename LightT, size_t max_lights>
requires(max_lights <= std::numeric_limits<size_t>::max() / 2)
class PBT
{
public:
    typedef PBTNode<SpaceT, LightT> array_data_type;

    PBT() :
        num_leaves(0)
    {}

    explicit PBT(ml::SceneLights<max_lights> const& lights) :
        num_leaves(std::bit_ceil(lights.get_num_lights()))
    {
        initialize_leaf_nodes(lights);
        construct_parent_nodes();
    }

    // defaults should be enough for every other [de/con]structor
    ~PBT() = default;
    PBT(PBT const& other) = default;
    PBT& operator=(PBT const& other) = default;
    PBT(PBT&& other) = default;
    PBT& operator=(PBT&& other) = default;

    void regenerate(ml::SceneLights<max_lights> const& lights)
    {
        initialize_leaf_nodes(lights);
        construct_parent_nodes();
    }

    // Function which prints out ALL array data (formatted)
    void print() const
    {
        for (size_t light_index = 0; light_index < data.size(); light_index++)
        {
            if ((light_index + 1 & (light_index)) == 0)
            {
                std::cout << std::log2(light_index + 1);
                std::cout << " -------------------------------------------------" << std::endl;
            }
            print_item(light_index);
        }
    }

    static constexpr size_t get_first_child_index(size_t const& parent_index) noexcept
    {
        return parent_index * 2 + 1;
    }

    static constexpr size_t get_parent_index(size_t const& child_index) noexcept
    {
        return (child_index - 1) / 2;
    }

    bool node_is_empty(size_t const& index) const
    {
        return data[index].total_intensity.x;
    }

    bool node_is_leaf(size_t const& index) const
    {
        return index > (num_leaves - 1);
    }

    uint32_t get_num_leaves() const noexcept
    {
        return num_leaves;
    }

    uint64_t get_num_nodes() const noexcept
    {
        return get_num_leaves() * 2 - 1;
    }

    PBTNode<SpaceT, LightT>& operator[](size_t const& index) noexcept
    {
        assert(index <= num_leaves * 2 - 1);
        return data[index];
    }

    constexpr size_t size() noexcept
    {
        return data.size();
    }

    constexpr size_t size_bytes() noexcept
    {
        return data.size() * sizeof(PBTNode<SpaceT, LightT>);
    }

    constexpr array_data_type* get_data() noexcept
    {
        return data.data();
    }

    // TODO: incorrect WIP
    /*void print_valid_range() const
    {
        for (size_t light_index : data | std::ranges::views::take(num_leaves * 2 - 1) | std::views::filter([](int const i) { return i % 2 == 0; }))
        {
            if ((light_index + 1 & (light_index)) == 0)
            {
                std::cout << std::log2(light_index + 1);
                    std::cout << " -------------------------------------------------" << std::endl;
            }
            print_item(light_index);
        }
    }*/

private:
    std::array<array_data_type, std::bit_ceil(max_lights) * 2 - 1> data;

    std::vector<std::pair<size_t, uint32_t>> morton_index_code;

    uint32_t num_leaves;

    void print_item(size_t& light_index) const
    {
        std::cout << std::setw(3) << light_index << std::setw(0) << ": min: ";
        std::cout << std::setw(13) << data[light_index].bb.min_bounds.x << std::setw(0);
        std::cout << ", " << data[light_index].bb.min_bounds.y << std::setw(0);
        std::cout << ", " << std::setw(13) << data[light_index].bb.min_bounds.z << std::setw(0) << " : ";
        std::cout << data[light_index].total_intensity.x << std::endl;
        std::cout << std::setw(3) << "" << std::setw(0) << ": max: ";
        std::cout << std::setw(13) << data[light_index].bb.max_bounds.x << std::setw(0);
        std::cout << ", " << data[light_index].bb.max_bounds.y << std::setw(0);
        std::cout << ", " << std::setw(13) << data[light_index].bb.max_bounds.z << std::setw(0) << std::endl;
    }

    // assign true values to internal nodes of tree
    void construct_parent_nodes()
    {
        // for each level of the tree (bottom to top)
        for (size_t log_level = num_leaves - 1; log_level > 0; log_level /= 2)
        {
            // iterate left to right summing child nodes
            for (size_t current_child = 0; current_child < log_level; current_child += 2)
            {
                // could be made parallel safe, btu that would be an extension
                data[(log_level + current_child) / 2] = data[log_level + current_child] + data[log_level + current_child + 1];
            }
        }
        //std::cout << "fin" << std::endl;
    }

    // assign true values to leaf nodes of the tree (the lights)
    void initialize_leaf_nodes(ml::SceneLights<max_lights> const& lights)
    {
        // calculate space requirements
        num_leaves = std::bit_ceil(lights.get_max_lights());
        morton_index_code.reserve(num_leaves);
        morton_index_code.clear();

        //std::cout << "Number of leaves: " << num_leaves << std::endl;
    	
        // TODO: remove/refactor
        const auto t1 = std::chrono::high_resolution_clock::now();

        // get size of root bounding box
        auto x_dist = lights.x_bounds[1] - lights.x_bounds[0];
        auto y_dist = lights.y_bounds[1] - lights.y_bounds[0];
        auto z_dist = lights.z_bounds[1] - lights.z_bounds[0];

        // for each light, create a <index, morton code> pair
        for (size_t light_index = 0; light_index < lights.get_num_lights(); ++light_index)
        {
            glm::vec<3, LightT> const& current_pos = lights.data()[light_index].position;
            morton_index_code.emplace_back(std::make_pair(light_index, MortonCodeGenerator::get_morton_code((current_pos.x - lights.x_bounds[0]) / x_dist,
                (current_pos.y - lights.y_bounds[0]) / y_dist,
                (current_pos.z - lights.z_bounds[0]) / z_dist)));
        }

        // sort morton codes
        std::sort(std::execution::par_unseq, morton_index_code.begin(), morton_index_code.end(),
            [](std::pair<size_t, uint32_t> const& pair1, std::pair<size_t, uint32_t> const& pair2)
            {
                return (pair1.second < pair2.second);
            }
        );

        // fill leaves in order
        for (size_t light_index = 0; light_index < lights.get_num_lights(); ++light_index)
        {
            data[num_leaves - 1 + light_index].bb.min_bounds = lights.data()[morton_index_code[light_index].first].position;
            data[num_leaves - 1 + light_index].bb.max_bounds = lights.data()[morton_index_code[light_index].first].position;
            data[num_leaves - 1 + light_index].total_intensity.x = lights.data()[morton_index_code[light_index].first].color.r +
                lights.data()[morton_index_code[light_index].first].color.g +
                lights.data()[morton_index_code[light_index].first].color.b;
            data[num_leaves - 1 + light_index].original_index.r = morton_index_code[light_index].first;
        }

        // TODO: remove/refactor
        const auto t2 = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double, std::milli> ms_double = t2 - t1;
        //std::cout << ms_double.count() << "ms" << std::endl;
    }
};
