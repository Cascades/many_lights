#include <glad/glad.h>

#include <utility>
#include <vector>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#include "many_lights/mesh.h"

ml::Mesh::Mesh(std::vector<ml::Vertex> vertices, std::vector<unsigned int> indices, std::vector<ml::Texture> textures) :
    VAO(0),
    VBO(0),
    EBO(0),
	vertices(std::move(vertices)),
    indices(std::move(indices)),
    textures(std::move(textures))
{
    setupMesh();
}

void ml::Mesh::setupMesh()
{	
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // vertex colors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    // vertex texture coords
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

    glBindVertexArray(0);

    unsigned int ambient_tex_num = 1;
    unsigned int diffuse_tex_num = 1;
    unsigned int specular_tex_num = 1;
    unsigned int bump_tex_num = 1;
    unsigned int dissolve_tex_num = 1;

    std::cout << "-------------" << std::endl;

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
        {
            number = std::to_string(diffuse_tex_num++);
        }
        else if (name == "texture_specular")
        {
            number = std::to_string(specular_tex_num++);
        }
        else if (name == "texture_ambient")
        {
            number = std::to_string(ambient_tex_num++);
        }
        else if (name == "texture_bump")
        {
            number = std::to_string(bump_tex_num++);
        }
        else if (name == "texture_dissolve")
        {
            number = std::to_string(dissolve_tex_num++);
        }

        material_uniform_names.emplace_back((name + number).c_str());
    }

    for (int z = 0; z < material_uniform_names.size(); ++z)
    {
        std::cout << material_uniform_names[z] << " : " << textures[z].path << " : " << textures[z].id << std::endl;
    }
}

void ml::Mesh::draw(ml::Shader const & shader) const
{
    shader.use();

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    for (unsigned int i = 0; i < textures.size(); i++)
    {
        glUniform1i(glGetUniformLocation(*shader.id, material_uniform_names[i].c_str()), i);
    }

    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}