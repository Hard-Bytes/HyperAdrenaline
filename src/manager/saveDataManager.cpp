#include "saveDataManager.hpp"
#include "../util/paths.hpp"
#include "../util/macros.hpp"
#include "../util/jsonReader.hpp"
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include <iostream>
#include <fstream>

using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>;

#define SECTION_WEAPONS "weapons"

void SaveDataManager::saveObtainedWeapon(int weapon, bool obtained)
{
    // Open document and navigate to weapons
    rapidjson::Document doc;
    openJsonFile(UTIL_SAVE_FILE, doc);
    rapidjson::Value& value { doc[SECTION_WEAPONS] };

    if(!value.HasMember(weaponNames[weapon-1]))
    {
        LOG_ERR("Save Manager tried to save a weapon that does not exist (" << weapon << ")");
        return;
    }
    rapidjson::Value& weaponData { value[weaponNames[weapon-1]] };
    weaponData.SetBool(obtained);

    // Save
    StringBuffer buf (&doc.GetAllocator());
    rapidjson::PrettyWriter<StringBuffer> writer(buf);
    doc.Accept (writer);
    std::string json (buf.GetString(), buf.GetSize());
    std::ofstream of (UTIL_SAVE_FILE);
    of << json;
}

std::vector<int> SaveDataManager::loadObtainedWeapons()
{
    rapidjson::Document doc;
    openJsonFile(UTIL_SAVE_FILE, doc);
    rapidjson::Value& value { doc[SECTION_WEAPONS] };

    std::vector<int> result;
    int maxWeapons { 7 };
    result.reserve(maxWeapons);

    for(int i=0; i<maxWeapons; ++i)
    {
        if(!value.HasMember(weaponNames[i])) continue;
        rapidjson::Value& weaponData { value[weaponNames[i]] };
        if(weaponData.GetBool())
            result.push_back(i+1);
    }

    return result;
}
