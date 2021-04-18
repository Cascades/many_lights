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
    };

    class Mesh {
    public:
        std::vector<ml::Vertex>       vertices;
        std::vector<unsigned int>     indices;
        std::vector<ml::Texture>      textures;

        Mesh(std::vector<ml::Vertex> vertices, std::vector<unsigned int> indices, std::vector<ml::Texture> textures);
        void draw(ml::Shader& shader);
    private:
        //  render data
        unsigned int VAO, VBO, EBO;

        void setupMesh();
    };
}