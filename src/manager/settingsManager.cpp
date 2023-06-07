#include "settingsManager.hpp"
#include <rapidjson/filereadstream.h>
#include "../util/jsonReader.hpp"

SettingsManager::SettingsManager()
{}

SettingsManager::~SettingsManager()
{}

/*Get Setting (String)*/
std::string SettingsManager::getString(std::string attributePath) const noexcept
{
    // Init delimiter, result, json document and loop data
    static const std::string delimiter = "/";
    attributePath += delimiter;
    std::string result {""};
    rapidjson::Document doc;
    openJsonFile(UTIL_CONFIG_FILE, doc);
    rapidjson::Value& value = doc["settings"];
    std::size_t pos {0};
    std::string token{""};
    
    // While there is path, keep diving in the tree
    while ( (pos = attributePath.find(delimiter)) != std::string::npos ) 
    {
        // Find current path step and search it in the tree
        token = attributePath.substr(0, pos);

        if( value.HasMember(token.c_str()) )
        {
            // Remove current token from path and dive in the tree
            attributePath.erase(0, pos + delimiter.length());
            value = value[token.c_str()];

            // If the path is empty we reached the end, so get the value
            if(attributePath.empty())
            {
                result = value.GetString();
                return result;
            }
        }
        // If the member does not exist, stop searching
        else
        {
            LOG_ERR("Cannot find the value of the settings");
            break;
        }
    }
    
    return result;
}

