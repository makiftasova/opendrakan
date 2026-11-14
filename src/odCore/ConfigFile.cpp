/*
 * ConfigFile.cpp
 *
 *  Created on: 8 Dec 2018
 *      Author: zal
 */

#include <odCore/ConfigFile.h>

#include <regex>

#include <odCore/Logger.h>

namespace od
{

    void ConfigFile::parse(std::istream &in)
    {
        std::regex sectionRegex("\\s*\\[\\s*(.+)\\s*\\]\\s*");
        std::regex keyValueRegex("\\s*(\\w+)\\s*[\\:\\=]\\s*(.*)");
        std::regex keyOnlyRegex("\\s*(.+)");
        std::regex commentRegex("\\s*(#.*)?");

        auto trim = [](const std::string &str)
        {
            size_t first = str.find_first_not_of(" \t");
            if(first == std::string::npos) return std::string();
            size_t last = str.find_last_not_of(" \t");
            return str.substr(first, last - first + 1);
        };

        std::string line;
        Section *currentSection = &mSections[""];
        while(std::getline(in, line))
        {
            // getline leaves the CR byte (0x0D) in the string if given windows line endings. remove if it is there
            if(line.size() != 0 && line[line.size() - 1] == 0x0D)
            {
                line.erase(line.size() - 1);
            }

            std::smatch results;

            // comments have to be checked first, as they would also match the key-only pattern
            if(std::regex_match(line, results, commentRegex))
            {
                continue;

            }else if(std::regex_match(line, results, sectionRegex))
            {
                currentSection = &mSections[trim(results[1].str())];

            }else if(std::regex_match(line, results, keyValueRegex))
            {
                currentSection->keyValueMap.insert(std::make_pair(results[1].str(), trim(results[2].str())));

            }else if(std::regex_match(line, results, keyOnlyRegex))
            {
                currentSection->rawValueList.push_back(trim(results[1].str()));

            }else
            {
                OD_PANIC() << "Malformed line in config file: " << line;
            }
        }
    }

    template <>
    std::string ConfigFile::get<std::string>(const std::string &section, const std::string &key)
    {
        auto sectionIt = mSections.find(section);
        if(sectionIt == mSections.end())
        {
            OD_PANIC() << "Section not found in config file: " << section;
        }

        auto keyIt = sectionIt->second.keyValueMap.find(key);
        if(keyIt == sectionIt->second.keyValueMap.end())
        {
            OD_PANIC() << "Key not found in config file: " << key;
        }

        return keyIt->second;
    }

    template <>
    std::string ConfigFile::get<std::string>(const std::string &section, const std::string &key, const std::string &defaultValue)
    {
        auto sectionIt = mSections.find(section);
        if(sectionIt == mSections.end())
        {
            return defaultValue;
        }

        auto keyIt = sectionIt->second.keyValueMap.find(key);
        if(keyIt == sectionIt->second.keyValueMap.end())
        {
            return defaultValue;
        }

        return keyIt->second;
    }

}
