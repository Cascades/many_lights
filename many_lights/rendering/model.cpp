#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb.h>
#include <stb_image.h>

#include <vector>
#include <string>
#include <filesystem>
#include <iostream>

#include "many_lights/model.h"

#include "assimp/mesh.h"
#include "many_lights/shader.h"
#include "many_lights/mesh.h"

ml::Model::Model(std::filesystem::path path)
{
    load_model(path);
}

void ml::Model::draw(ml::Shader const & shader) const
{
    //sort by shader
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].draw(shader);
    }
}

std::vector<ml::Mesh>& ml::Model::get_meshes()
{
    return meshes;
}

void ml::Model::load_model(std::filesystem::path path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path.generic_string().c_str(), aiProcess_Triangulate |
                                                                            aiProcess_FlipUVs |
                                                                            aiProcess_GenNormals |
                                                                            aiProcess_JoinIdenticalVertices |
                                                                            aiProcess_ImproveCacheLocality |
                                                                            aiProcess_FindDegenerates |
                                                                            aiProcess_RemoveRedundantMaterials |
                                                                            aiProcess_OptimizeGraph |
                                                                            aiProcess_OptimizeMeshes);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode || !path.has_parent_path())
    {
        std::cout << std::string("Error loading assimp: ") + std::string(importer.GetErrorString()) << std::endl;
        throw std::string("Error loading assimp: ") + std::string(importer.GetErrorString());
    }

    directory = path.parent_path();

    process_node(scene->mRootNode, scene);
}

void ml::Model::process_node(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        process_node(node->mChildren[i], scene);
    }
}

ml::Mesh ml::Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<ml::Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<ml::Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        ml::Vertex vertex{};
    	
        aiColor4D diffuse;
        aiString name_0;
        scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, name_0);
        scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);

        if (strcmp(name_0.C_Str(), "None") == 0)
        {
            vertex.color = glm::vec3(1.0f);
        }
        else
        {
            vertex.color.r = diffuse.r;
            vertex.color.g = diffuse.g;
            vertex.color.b = diffuse.b;
        }
    	
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;
        
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.tex_coords = vec;
        }
        else
        {
            vertex.tex_coords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<ml::Texture> ambientMaps = load_material_textures(material, aiTextureType_AMBIENT, "texture_ambient");
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());

        std::vector<ml::Texture> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<ml::Texture> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        std::vector<ml::Texture> bumpMaps = load_material_textures(material, aiTextureType_HEIGHT, "texture_bump");
        textures.insert(textures.end(), bumpMaps.begin(), bumpMaps.end());

        std::vector<ml::Texture> dissolveMaps = load_material_textures(material, aiTextureType_OPACITY, "texture_dissolve");
        textures.insert(textures.end(), dissolveMaps.begin(), dissolveMaps.end());
    }

    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<ml::Texture> ml::Model::load_material_textures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
    std::vector<ml::Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (textures_loaded[j].path == std::filesystem::path(str.C_Str()))
            {
                textures.push_back(textures_loaded[j]);
                textures.back().type = typeName;
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Texture texture;
            std::vector<unsigned char> temp_text_data;
            uint channels;
            texture.id = texture_from_file(std::filesystem::path(str.C_Str()), false, texture.size, temp_text_data, channels);
            texture.type = typeName;
            texture.path = std::filesystem::path(str.C_Str());
            texture.image_data = temp_text_data;
            texture.channels = channels;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    if (textures.empty())
    {
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (textures_loaded[j].path == std::filesystem::path("../blank.png"))
            {
                textures.push_back(textures_loaded[j]);
                textures.back().type = typeName;
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Texture texture;
            std::vector<unsigned char> temp_text_data;
            uint channels;
            texture.id = texture_from_file(std::filesystem::path("../blank.png"), false, texture.size, temp_text_data, channels);
            texture.type = typeName;
            texture.path = std::filesystem::path("../blank.png");
            texture.image_data = temp_text_data;
            texture.channels = channels;
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }
    }

    return textures;
}

unsigned int ml::Model::texture_from_file(std::filesystem::path const & path, [[maybe_unused]] bool gamma, glm::vec2 & dimensions, std::vector<unsigned char> & temp_text_data, uint& channels)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    std::cout << (directory / path).generic_string().c_str() << std::endl;
    unsigned char* data = stbi_load((directory / path).generic_string().c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format{};
        if (nrComponents == 1)
        {
            format = GL_RED;
        }
        else if (nrComponents == 2)
        {
            format = GL_RG;
        }
        else if (nrComponents == 3)
        {
            format = GL_RGB;
        }
        else
        {
            format = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        channels = nrComponents;

        temp_text_data.assign(data, data + (width * height * nrComponents));

        stbi_image_free(data);
        dimensions.x = static_cast<float>(width);
        dimensions.y = static_cast<float>(height);
    }
    else
    {
        stbi_image_free(data);
        throw std::string("Texture failed to load at path: ") + path.string();
    }

    return textureID;
}


