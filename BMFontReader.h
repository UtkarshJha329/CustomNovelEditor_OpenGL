#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

struct Glyph {
    int id;
    int x, y;
    int width, height;
    int xOffset, yOffset;
    int xAdvance;
};

class BMFontReader {
public:
    BMFontReader(const std::string& filename) : m_filename(filename) {}

    bool read() {
        std::ifstream file(m_filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << m_filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "char") {
                Glyph glyph;
                bool filled = false;
                while (iss >> token) {
                    std::string key = token.substr(0, token.find('='));
                    std::string value = token.substr(token.find('=') + 1);
                    if (key == "id") glyph.id = std::stoi(value);
                    else if (key == "x") glyph.x = std::stoi(value);
                    else if (key == "y") glyph.y = std::stoi(value);
                    else if (key == "width") glyph.width = std::stoi(value);
                    else if (key == "height") glyph.height = std::stoi(value);
                    else if (key == "xoffset") glyph.xOffset = std::stoi(value);
                    else if (key == "yoffset") glyph.yOffset = std::stoi(value);
                    else if (key == "xadvance") glyph.xAdvance = std::stoi(value);
                    
                    filled = true;
                }
                if (filled) {
                    char c = (char)glyph.id;
                    m_glyphs.insert({ c, glyph });
                }
            }
        }
        file.close();
        return true;
    }

    const std::unordered_map<char, Glyph>& getGlyphs() const {
        return m_glyphs;
    }

public:
    std::string m_filename;
private:
    std::unordered_map<char, Glyph> m_glyphs;
};
