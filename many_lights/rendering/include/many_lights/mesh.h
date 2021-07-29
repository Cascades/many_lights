#pragma once
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <filesystem>

#include <stb.h>

#include "many_lights/shader.h"

namespace ml
{
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 tex_coords;
    };

    struct Texture {
        GLuint id;
        std::string type;
        std::filesystem::path path;
        glm::vec2 size;
        std::vector<unsigned char> image_data;
        uint32_t channels;
    };

    class Mesh {
    private:
        unsigned int VAO, VBO, EBO;

        void setupMesh();
    public:
        std::vector<ml::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<ml::Texture> textures;
        std::vector<std::string> material_uniform_names;
        std::vector<unsigned int> material_uniform_locations;

        Mesh(std::vector<ml::Vertex> vertices, std::vector<unsigned int> indices, std::vector<ml::Texture> textures);
        void draw(ml::Shader const& shader) const;
    };
}