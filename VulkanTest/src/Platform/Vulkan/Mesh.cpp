#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <filesystem>

bool Mesh::load_from_obj(bool illuminated, bool texture)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (std::filesystem::current_path().parent_path().string() + "/" + MODEL_PATH + this->Filename).c_str())) {
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
        }
    }

    return true;
}

bool Mesh::load_sphere(bool illuminated, bool textured)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};


    const unsigned int X_SEGMENTS = 128;
    const unsigned int Y_SEGMENTS = 128;
    const float PI = 3.14159265359f;

    for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
    {
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            float xSegment = static_cast<float>(x) / static_cast<float>(X_SEGMENTS);
            float ySegment = static_cast<float>(y) / static_cast<float>(Y_SEGMENTS);
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            Vertex vertex{};

            vertex.Position = glm::vec3(xPos, yPos, zPos);
            if (textured) vertex.TexCoord = glm::vec2(xSegment, ySegment);
            if (illuminated) vertex.Normal = glm::vec3(xPos, yPos, zPos);

            Vertices.push_back(vertex);
        }
    }

    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x)
        {
            // Define indices for the two triangles in each quad
            unsigned int first = y * (X_SEGMENTS + 1) + x;
            unsigned int second = (y + 1) * (X_SEGMENTS + 1) + x;

            // First triangle
            Indices.push_back(first);
            Indices.push_back(second);
            Indices.push_back(first + 1);

            // Second triangle
            Indices.push_back(second);
            Indices.push_back(second + 1);
            Indices.push_back(first + 1);
        }
    }
    
    return true;
}

bool Mesh::load_cube(bool illuminated, bool textured)
{
    // Clear existing data
    Vertices.clear();
    Indices.clear();

    // Define cube vertices
    std::vector<glm::vec3> positions = {
        // Front face
        glm::vec3(-0.5f, 0.5f, 0.5f),  // Top-left
        glm::vec3(0.5f, 0.5f, 0.5f),   // Top-right
        glm::vec3(0.5f, -0.5f, 0.5f),  // Bottom-right
        glm::vec3(-0.5f, -0.5f, 0.5f), // Bottom-left

        // Back face
        glm::vec3(-0.5f, 0.5f, -0.5f),  // Top-left
        glm::vec3(0.5f, 0.5f, -0.5f),   // Top-right
        glm::vec3(0.5f, -0.5f, -0.5f),  // Bottom-right
        glm::vec3(-0.5f, -0.5f, -0.5f) // Bottom-left
    };

    std::vector<glm::vec2> uv;
    if (textured) {
        // Define texture coordinates for each face (assuming seamless texture)
        uv = {
            //// Front face
            //glm::vec2(0.0f, 1.0f), // Top-left
            //glm::vec2(1.0f, 1.0f), // Top-right
            //glm::vec2(1.0f, 0.0f), // Bottom-right
            //glm::vec2(0.0f, 0.0f), // Bottom-left

            //// Back face
            //glm::vec2(1.0f, 1.0f), // Top-left
            //glm::vec2(0.0f, 1.0f), // Top-right
            //glm::vec2(0.0f, 0.0f), // Bottom-right
            //glm::vec2(1.0f, 0.0f)  // Bottom-left
        };
    }

    std::vector<glm::vec3> normals;
    if (illuminated) {
        // Define normals for each face
        normals = {
            // Front face
            glm::vec3(0.0f, 0.0f, 1.0f),

            // Back face
            glm::vec3(0.0f, 0.0f, -1.0f)
        };
    }

    // Create vertices
    for (size_t i = 0; i < positions.size(); ++i)
    {
        Vertex vertex{};
        vertex.Position = positions[i];
        if (textured) vertex.TexCoord = uv.empty() ? glm::vec2(0.0f) : uv[i];
        if (illuminated) vertex.Normal = normals.empty() ? glm::vec3(0.0f) : normals[i];
        Vertices.push_back(vertex);
    }

    Indices = {
        // Front face
        3,2,1,3,1,0,

        // Back face
        6,4,5,6,7,4,

        // Top face
        0,1,5,0,5,4,

        // Bottom face
        2,3,7,2,7,6,

        // Right face
        6,5,1,6,1,2,

        // Left face
        7,3,0,7,0,4
    };

    return true;
}

bool Mesh::load_quad(bool illuminated, bool textured)
{
    // Clear existing data
    Vertices.clear();
    Indices.clear();

    // Define cube vertices
    std::vector<glm::vec3> positions = {
        // Front face
        glm::vec3(-0.5f, 0.5f, 0.0f),  // Top-left
        glm::vec3(0.5f, 0.5f, 0.0f),   // Top-right
        glm::vec3(0.5f, -0.5f, 0.0f),  // Bottom-right
        glm::vec3(-0.5f, -0.5f, 0.0f), // Bottom-left
    };

    std::vector<glm::vec2> uv;
    if (textured) {
        // Define texture coordinates for each face (assuming seamless texture)
        uv = {
            // Front face
 
            glm::vec2(1.0f, 0.0f), // Bottom-right
            glm::vec2(0.0f, 0.0f), // Bottom-left
            glm::vec2(0.0f, 1.0f), // Top-left
            glm::vec2(1.0f, 1.0f), // Top-right
        };
    }

    std::vector<glm::vec3> normals;
    if (illuminated) {
        // Define normals for each face
        normals = {
            // Front face
            glm::vec3(0.0f, 0.0f, 1.0f),

            // Back face
         //   glm::vec3(0.0f, 0.0f, -1.0f)
        };
    }

    // Create vertices
    for (size_t i = 0; i < positions.size(); ++i)
    {
        Vertex vertex{};
        vertex.Position = positions[i];
        if (textured) vertex.TexCoord = uv.empty() ? glm::vec2(0.0f) : uv[i];
        if (illuminated) vertex.Normal = normals.empty() ? glm::vec3(0.0f) : normals[i];
        Vertices.push_back(vertex);
    }

    Indices = {
        // Front face
        3,2,1,3,1,0,
    };

    return true;
}

bool Mesh::load_line(bool illuminated, bool textured)
{

    // Define cube vertices
    std::vector<glm::vec3> positions = {
        // Front face
        glm::vec3(0.0f, 0.5f, 0.0f),  // Top-left
        glm::vec3(0.0f, -0.5f, 0.0f),   // Top-right
    };

    // Create vertices
    for (size_t i = 0; i < positions.size(); ++i)
    {
        Vertex vertex{};
        vertex.Position = positions[i];
        vertex.Color = glm::vec3(0.0f, 0.0f, 0.0f);
        Vertices.push_back(vertex);
    }

    Indices = {
        // Front face
        0,1
    };

    return true;
}

bool Mesh::load_terrain(bool illuminated, bool textured)
{

    static int VERTEX_COUNT{ 128 };
    static int SIZE{ 800 };

    for (int i = 0; i < VERTEX_COUNT; i++) 
    {
        for (int j = 0; j < VERTEX_COUNT; j++) 
        {
            Vertex vertex{};
            vertex.Position = glm::vec3(static_cast<float>(j) / static_cast<float>(VERTEX_COUNT - 1) * SIZE, -5.0f, static_cast<float>(i) / static_cast<float>(VERTEX_COUNT - 1) * SIZE);
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            vertex.TexCoord = glm::vec2(static_cast<float>(j) / static_cast<float>(VERTEX_COUNT - 1) * 40.0f, static_cast<float>(i) / static_cast<float>(VERTEX_COUNT - 1) * 40.0f);
            Vertices.push_back(vertex);
        }
    }

    for (int gz = 0; gz < VERTEX_COUNT - 1; gz++) {
        for (int gx = 0; gx < VERTEX_COUNT - 1; gx++) {
            int topLeft = (gz * VERTEX_COUNT) + gx;
            int topRight = topLeft + 1;
            int bottomLeft = ((gz + 1) * VERTEX_COUNT) + gx;
            int bottomRight = bottomLeft + 1;

            Indices.push_back(topLeft);
            Indices.push_back(bottomLeft);
            Indices.push_back(topRight);
            Indices.push_back(topRight);
            Indices.push_back(bottomLeft);
            Indices.push_back(bottomRight);
        }
    }

    return true;
}

bool Mesh::load(bool illuminated, bool textured)
{
    if (!this->Filename.empty())
    {
        return load_from_obj(illuminated, textured);
    }

    switch (this->meshType)
    {
        case MeshType::Sphere:
        {
            return load_sphere(illuminated, textured);
            break;
        }
        case MeshType::Cube:
        {
            return load_cube(illuminated, textured);
            break;
        }
        case MeshType::Quad:
        {
            return load_quad(illuminated, textured);
            break;
        }
        case MeshType::Line:
        {
            return load_line(illuminated, textured);
            break;
        }
        case MeshType::Terrain:
        {
            return load_terrain(illuminated, textured);
            break;
        }
    }
    return false;
}



bool Mesh::load_animation(std::string filename)
{
    Animated = true;
    std::string line;
    std::ifstream inputFile(std::filesystem::current_path().parent_path().string() + "/" + ANIMATION_PATH + filename);

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
