#pragma once
#include "../util/jsonReader.hpp"
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "../util/paths.hpp"
#include "../util/macros.hpp"

using Cstring = const char*;
using StringBuffer = rapidjson::GenericStringBuffer<rapidjson::UTF8<>, rapidjson::MemoryPoolAllocator<>>;

class SettingsManager
{
public:
    explicit SettingsManager();
    ~SettingsManager();

    /*Get Setting (No template)*/
    std::string getString(std::string attributePath) const noexcept;

    /*Get Setting*/
    // Gets a setting given path and type
    // Accepted types are bool, int, float and Cstring (const char*)
    // Examples:
    // - bool b = settingsManager.get<bool>("multiplayer/isHosting");
    // - int i = settingsManager.get<int>("route/to/whatever");
    // - float f = settingsManager.get<float>("audio/globalVolume");
    // - std::string s = std::string(settingsManager.get<Cstring>("multiplayer/host"));
    template<typename T>
    T get(std::string attributePath)
    {
        // Init delimiter, result, json document and loop data
        static const std::string delimiter = "/";
        attributePath += delimiter;
        T result {};
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
                    result = value.Get<T>();
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

    /*Set Setting*/
    // Sets a setting given a value, and path and type
    // Accepted types are bool, int, float and Cstring (const char*)
    // Returns true if the propery was found and modified, or false if not
    // Examples:
    // - settingsManager.set("multiplayer/isHosting", true);
    // - settingsManager.set("route/to/whatever", 3);
    // - settingsManager.set("audio/globalVolume", 0.2);
    // - settingsManager.set("multiplayer/host", "localhost");
    template<typename T>
    bool set(std::string attributePath, T newValue)
    {
        // Init delimiter, result, json document and loop data
        static const std::string delimiter = "/";
        attributePath += delimiter;

        bool result = false;

        rapidjson::Document doc;
        openJsonFile(UTIL_CONFIG_FILE, doc);
        std::vector<rapidjson::Value*> values;
        values.push_back(&doc["settings"]);

        std::size_t pos {0};
        std::string token{""};
        
        // While there is path, keep diving in the tree
        while ( (pos = attributePath.find(delimiter)) != std::string::npos ) 
        {
            // Find current path step and search it in the tree
            token = attributePath.substr(0, pos);

            if( values[values.size()-1]->HasMember(token.c_str()) )
            {
                // Remove current token from path and dive in the tree
                attributePath.erase(0, pos + delimiter.length());
                auto& val = *values[values.size()-1];
                values.push_back(&val[token.c_str()]);

                // If the path is empty we reached the end, so get the value
                if(attributePath.empty())
                {
                    values[values.size()-1]->Set<T>(newValue);
                    result = true;

                    StringBuffer buf (&doc.GetAllocator());
                    rapidjson::PrettyWriter<StringBuffer> writer(buf);
                    doc.Accept (writer);
                    std::string json (buf.GetString(), buf.GetSize());
                    std::ofstream of (UTIL_CONFIG_FILE);
                    of << json;

                    break;
                }
            }
            // If the member does not exist, stop searching
            else
            {
                LOG_ERR("Cannot find the value of the settings");
                break;
            }
        }

        values.clear();
        return result;
    }
};

