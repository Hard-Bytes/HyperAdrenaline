#include "jsonReader.hpp"
#include <rapidjson/filereadstream.h>

void openJsonFile(std::string filepath, rapidjson::Document& document) noexcept
{
    // It'll follow a strict format
    FILE* pFile = fopen(filepath.c_str(), "rb");
    if(!pFile) return;

    char buffer[65536];
    rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
    document.ParseStream<0, rapidjson::UTF8<>, rapidjson::FileReadStream>(is);

    fclose(pFile);
}
