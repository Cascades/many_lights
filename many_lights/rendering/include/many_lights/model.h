#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <vector>
#include <filesystem>

#include "assimp/material.h"
#include "assimp/mesh.h"
#include "many_lights/shader.h"
#include "many_lights/mesh.h"

namespace ml
{
    class Model
    {
    public:
        Model(std::filesystem::path path);

        void draw(ml::Shader const & shader) const;

        std::vector<ml::Mesh>& get_meshes();
    private:
        // model data
    // EDIT HACK
    public:
        std::vector<ml::Mesh> meshes;
        std::filesystem::path directory;
        std::vector<ml::Texture> textures_loaded;

        void load_model(std::filesystem::path path);
        void process_node(aiNode* node, const aiScene* scene);
        Mesh process_mesh(aiMesh* mesh, const aiScene* scene);
        std::vector<ml::Texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name);
        unsigned int texture_from_file(std::filesystem::path const& path, [[maybe_unused]] bool gamma, glm::vec2& dimensions, std::vector<unsigned char>& temp_text_data, uint& channels);
    };
}