#pragma once
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <filesystem>

#include "many_lights/shader.h"

namespace ml
{
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    struct Texture {
        unsigned int id;
        std::string type;
        std::filesystem::path path;
        glm::vec2 size;
    };

    class Mesh {
    public:
        std::vector<ml::Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<ml::Texture> textures;
        std::vector<std::string> material_uniform_names;
        std::vector<GLint> material_uniform_locations;
        ml::Shader shader;

        Mesh(std::vector<ml::Vertex> vertices, std::vector<unsigned int> indices, std::vector<ml::Texture> textures);
        void set_shader(ml::Shader& shader);
        void set_shader(ml::Shader&& shader);
        void draw() const;
    private:
        //  render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
    };
}