#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

#include "many_lights/mesh.h"

ml::Mesh::Mesh(std::vector<ml::Vertex> vertices, std::vector<unsigned int> indices, std::vector<ml::Texture> textures) :
    vertices(vertices),
    indices(indices),
    textures(textures)
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);

    unsigned int diffuse_tex_num = 1;
    unsigned int specular_tex_num = 1;

    std::vector<std::string> material_uniforms;

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

        material_uniforms.push_back(("material." + name + number.c_str()).c_str());
    }

    for (int z = 0; z < material_uniforms.size(); ++z)
    {
        std::cout << material_uniforms[z] << std::endl;
    }
}

void ml::Mesh::draw(ml::Shader & shader)
{
    unsigned int diffuse_tex_num = 1;
    unsigned int specular_tex_num = 1;

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

        glUniform1f(glGetUniformLocation(shader.ID, ("material." + name + number.c_str()).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glActiveTexture(GL_TEXTURE0);

    // draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}