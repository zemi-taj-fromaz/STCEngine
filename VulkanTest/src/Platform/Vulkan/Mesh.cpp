#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <filesystem>


glm::vec2 randomGradient(int ix, int iy) {
    // No precomputed gradients mean this works for any number of grid coordinates
    const unsigned w = 8 * sizeof(unsigned);
    const unsigned s = w / 2;
    unsigned a = ix, b = iy;
    a *= 3284157443;

    b ^= a << s | a >> w - s;
    b *= 1911520717;

    a ^= b << s | b >> w - s;
    a *= 2048419325;
    float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

    // Create the vector from the angle
    glm::vec2 v;
    v.x = sin(random);
    v.y = cos(random);

    return v;
}

// Computes the dot product of the distance and gradient vectors.
float dotGridGradient(int ix, int iy, float x, float y) {
    // Get gradient from integer coordinates
    glm::vec2 gradient = randomGradient(ix, iy);

    // Compute the distance vector
    float dx = x - (float)ix;
    float dy = y - (float)iy;

    // Compute the dot-product
    return (dx * gradient.x + dy * gradient.y);
}

float interpolate(float a0, float a1, float w)
{
    return (a1 - a0) * (3.0 - w * 2.0) * w * w + a0;
}


// Sample Perlin noise at coordinates x, y
float perlin(float x, float y) {

    // Determine grid cell corner coordinates
    int x0 = static_cast<int>(std::floor(x));
    int y0 = static_cast<int>(std::floor(y));
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    // Compute Interpolation weights
    float sx = x - (float)x0;
    float sy = y - (float)y0;

    // Compute and interpolate top two corners
    float n0 = dotGridGradient(x0, y0, x, y);
    float n1 = dotGridGradient(x1, y0, x, y);
    float ix0 = interpolate(n0, n1, sx);

    // Compute and interpolate bottom two corners
    n0 = dotGridGradient(x0, y1, x, y);
    n1 = dotGridGradient(x1, y1, x, y);
    float ix1 = interpolate(n0, n1, sx);

    // Final step: interpolate between the two previously interpolated values, now in y
    float value = interpolate(ix0, ix1, sy);

    return value;
}


std::vector<std::vector<float>> generate_perlin_noisee()
{
    std::vector<float> row(800);
    std::vector<std::vector<float>> ret(800, row);

    const int terrainWidth = 800;
    const int terrainHeight = 800;

    const int GRID_SIZE = 400;


    for (int x = 0; x < terrainWidth; x++)
    {
        for (int y = 0; y < terrainHeight; y++)
        {
            int index = (y * terrainWidth + x) * 4;

            float val = 0;

            float freq = 1;
            float amp = 1;

            for (int i = 0; i < 12; i++)
            {
                val += perlin(x * freq / GRID_SIZE, y * freq / GRID_SIZE) * amp;

                freq *= 2;
                amp /= 2;

            }

            // Contrast
            val *= 1.2;

            // Clipping
            if (val > 1.0f)
                val = 1.0f;
            else if (val < -1.0f)
                val = -1.0f;

            // Convert 1 to -1 into 255 to 0
         //   int color = (int)(((val + 1.0f) * 0.5f) * 255);

            ret[x][y] = val * 20;
        }
    }
    return ret;
}

std::vector<std::vector<float>> Mesh::heightMap = generate_perlin_noisee();

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

   static int VERTEX_COUNT{ 256 };
   static int SIZE{ 100 };

    for (int i = 0; i < VERTEX_COUNT; i++) 
    {
        for (int j = 0; j < VERTEX_COUNT; j++) 
        {
            Vertex vertex{};
            vertex.Position = glm::vec3(static_cast<float>(j) / static_cast<float>(VERTEX_COUNT - 1) * SIZE, heightMap[j][i], static_cast<float>(i) / static_cast<float>(VERTEX_COUNT - 1) * SIZE);
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

bool Mesh::load_plain(bool illuminated, bool textured)
{

    float kTileSize = 256.0f;

    const int32_t kHalfSize = 256.0f / 2.0f;
    float kScale = 1.0f;


    for (int32_t y = 0; y < kTileSize; ++y)
    {
        for (int32_t x = 0; x < kTileSize; ++x)
        {
            Vertex vertex{};
            vertex.Position = glm::vec3(
                static_cast<float>(x),      // x
                0.0f,                       // y
                static_cast<float>(y)       // z
            ) * kScale;

            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
            //   vertex.TexCoord = glm::vec2(static_cast<float>(j) / static_cast<float>(VERTEX_COUNT - 1), static_cast<float>(i) / static_cast<float>(VERTEX_COUNT - 1));
            vertex.TexCoord = glm::vec2(x, y);
            Vertices.push_back(vertex);
        }
    }

    float VERTEX_COUNT = kTileSize;

    for (int gz = 0; gz < kTileSize; gz++) {
        for (int gx = 0; gx < kTileSize; gx++) {
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
        case MeshType::Plain:
        {
            return load_plain(illuminated, textured);
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
