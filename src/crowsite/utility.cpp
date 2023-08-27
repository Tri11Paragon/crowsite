//
// Created by brett on 6/20/23.
//
#include <crowsite/utility.h>
#include <blt/std/string.h>
#include <curl/curl.h>

namespace cs
{
    
    namespace parser
    {
        Post::Post(const std::string& input)
        {
            auto pairs = blt::string::split(input, "&");
            for (const auto& pair : pairs)
            {
                auto kv = blt::string::split(pair, "=");
                auto key = kv[0];
                auto value = kv[1];
                auto stripped_key = curl_easy_unescape(nullptr, key.c_str(), 0, nullptr);
                auto stripped_value = curl_easy_unescape(nullptr, value.c_str(), 0, nullptr);
                m_Values[stripped_key] = stripped_value;
                curl_free(stripped_key);
                curl_free(stripped_value);
            }
        }
        
        const std::string& Post::operator[](const std::string& key)
        {
            return m_Values[key];
        }
        
        std::string Post::dump()
        {
            std::string out;
            for (const auto& pair : m_Values)
            {
                out += "[";
                out += pair.first;
                out += "] = ";
                out += pair.second;
                out += "; ";
            }
            return out;
        }
        
        bool Post::hasKey(const std::string& key)
        {
            return m_Values.find(key) != m_Values.end();
        }
    }
    
    namespace fs
    {
        std::string createStaticFilePath(const std::string& file)
        {
            auto path = std::string(CROW_STATIC_DIRECTORY);
            if (!path.ends_with('/'))
                path += '/';
            path += file;
            // prevent crow from hanging web responses when we make a typo in the filename
            if (!std::filesystem::exists(path))
                throw std::runtime_error("Unable to create file path because file does not exist!");
            return path;
        }
        
        std::string createWebFilePath(const std::string& file)
        {
            auto path = std::string(SITE_FILES_PATH);
            if (!path.ends_with('/'))
                path += '/';
            path += "webcontent/";
            path += file;
            if (!std::filesystem::exists(path))
                throw std::runtime_error("Unable to create file path because file does not exist!");
            return path;
        }
        
        std::string createDataFilePath(const std::string& file)
        {
            auto path = std::string(SITE_FILES_PATH);
            if (!path.ends_with('/'))
                path += '/';
            path += "data/";
            path += file;
            if (!std::filesystem::exists(path))
                throw std::runtime_error("Unable to create file path because file does not exist!");
            return path;
        }
    }
    
}
