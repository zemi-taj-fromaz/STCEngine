#include "HelperObjects.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <fstream>
#include <vector>
#include <string>
#include <sstream>

bool Mesh::load_from_obj(std::string filename, bool illuminated, bool texture)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (MODEL_PATH + filename).c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.Position = {
              attrib.vertices[3 * index.vertex_index + 0],
              attrib.vertices[3 * index.vertex_index + 1],
              attrib.vertices[3 * index.vertex_index + 2]
            };

            if (illuminated)
            {
                vertex.Normal = {
                    attrib.normals[3 * index.vertex_index + 0],
                    attrib.normals[3 * index.vertex_index + 1],
                    attrib.normals[3 * index.vertex_index + 2]
                };
            }



            vertex.Color = {
                attrib.colors[3 * index.vertex_index + 0],
                attrib.colors[3 * index.vertex_index + 1],
                attrib.colors[3 * index.vertex_index + 2]
            };

            if (texture)
            {
                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(Vertices.size());
                Vertices.push_back(vertex);
            }

            Indices.push_back(uniqueVertices[vertex]);

      




            //Vertices.push_back(vertex);
            //Indices.push_back(Indices.size());
        }
    }

    return true;
}



bool Mesh::load_animation(std::string filename)
{
    Animated = true;
    std::string line;
    std::ifstream inputFile(ANIMATION_PATH + filename);

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return 1;
    }

    while (std::getline(inputFile, line)) {
        glm::vec3 vec;
        std::istringstream lineStream(line);
        std::string token;

        for (int i = 0; i < 3; i++) {
            if (std::getline(lineStream, token, ',')) {
                vec[i] = std::stof(token); // Convert the token to a float
            }
            else {
                throw std::runtime_error("LOADING ANIMATION");
            }
        }

        Animation.push_back(vec);
    }

    inputFile.close();

    return true;
}

const std::string Texture::PATH = "resources/textures/";
