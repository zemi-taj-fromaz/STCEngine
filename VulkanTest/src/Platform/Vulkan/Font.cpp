//#include "Font.h"
//
//#include <filesystem>
//
//#include <ft2build.h>
//#include FT_FREETYPE_H  
//
//const std::string Font::PATH = "resources/fonts";
//
//void Font::create_font()
//{
//    FT_Library ft;
//    // All functions return a value different than 0 whenever an error occurred
//    if (FT_Init_FreeType(&ft))
//    {
//        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
//        throw std::runtime_error("failed to load freetype!");
//    }
//
//    std::string filename;
//
//    // find path to font
//    std::string font_name = std::filesystem::current_path().parent_path().string() + "/" + Font::PATH + filename;
//
//    if (font_name.empty())
//    {
//        std::cout << "ERROR::FREETYPE: Failed to load font_name" << std::endl;
//        throw std::runtime_error("failed to load font file!");
//    }
//
//    // load font as face
//    FT_Face face;
//    if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
//        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
//        throw std::runtime_error("failed to load font!");
//    }
//    
//    // set size to load glyphs as
//    int fontSize;
//    FT_Set_Pixel_Sizes(face, 0, fontSize);
//
//
//    uint32_t bmpWidth = 0;
//    std::vector<uint8_t> pixels;
//
//    std::unordered_map<char, std::vector<uint8_t>> data;
//
//    // load first 128 characters of ASCII set
//    for (unsigned char c = 0; c < 128; c++)
//    {
//        // Load character glyph 
//        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
//        {
//            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
//            continue;
//        }
//
//        bmpHeight = std::max(bmpHeight, face->glyph->bitmap.rows);
//
//
//        unsigned int pitch = face->glyph->bitmap.pitch;
//        Character character = {
//            glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),
//            glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),
//            bmpWidth,
//            static_cast<unsigned int>(face->glyph->advance.x)
//        };
//
//        Characters.insert(std::pair<char, Character>(c, character));
//
//        if (face->glyph->bitmap.width > 0) 
//        {
//            void* ptr = face->glyph->bitmap.buffer;
//
//            std::vector<uint8_t> charData(face->glyph->bitmap.width * face->glyph->bitmap.rows);
//
//            int rows = face->glyph->bitmap.rows;
//            int width = face->glyph->bitmap.width;
//            for (int i = 0; i < rows; i++) 
//            {
//                for (int j = 0; j < width; j++) 
//                {
//                    uint8_t byte = face->glyph->bitmap.buffer[i * pitch + j];
//                    charData[i * pitch + j] = byte;
//                }
//            }
//            data.insert(std::pair<char, std::vector<uint8_t>>(c, charData));
//        }
//        bmpWidth += face->glyph->bitmap.width;
//
//
//    }
//    
//    // destroy FreeType once we're finished
//    FT_Done_Face(face);
//
//    invBmpWidth = 1 / (float)bmpWidth;
//
//    uint8_t* buffer = new uint8_t[bmpHeight * bmpWidth];
//    memset(buffer, 0, bmpHeight * bmpWidth);
//
//    uint32_t xpos = 0;
//    for (unsigned char c = 0; c < 128; c++)
//    {
//        Character& character = Characters[c];
//
//        std::vector<uint8_t>& charData = data[c];
//        uint32_t width = character.size.x;
//        uint32_t height = character.size.y;
//        for (uint32_t i = 0; i < height; i++) {
//            for (uint32_t j = 0; j < width; j++) {
//                uint8_t byte = charData[i * width + j];
//                buffer[i * bmpWidth + xpos + j] = byte;
//            }
//        }
//        xpos += width;
//    }
//
//    FT_Done_FreeType(ft);
//}
