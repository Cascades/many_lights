#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb.h>
#include <stb_image.h>

#include <vector>
#include <string>
#include <filesystem>

#include "many_lights/model.h"
#include "many_lights/shader.h"
#include "many_lights/mesh.h"


ml::Model::Model(std::filesystem::path path)
{
    load_model(path);
}

void ml::Model::draw(ml::Shader& shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].draw(shader);
    }
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
        ml::Vertex vertex;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.Normal = vector;
        
        if (mesh->mTextureCoords[0])
        {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
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
        std::vector<ml::Texture> diffuseMaps = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        std::vector<ml::Texture> specularMaps = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, indices, textures);
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
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Texture texture;
            texture.id = texture_from_file(std::filesystem::path(str.C_Str()), false);
            texture.type = typeName;
            texture.path = std::filesystem::path(str.C_Str());
            textures.push_back(texture);
            textures_loaded.push_back(texture);
        }

    }
    return textures;
}

unsigned int ml::Model::texture_from_file(std::filesystem::path path, bool gamma)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load((directory / path).generic_string().c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        stbi_image_free(data);
        throw std::string("Texture failed to load at path: ") + path.string();
    }

    return textureID;
}


