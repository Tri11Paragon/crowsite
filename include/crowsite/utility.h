//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_UTILITY_H
#define CROWSITE_UTILITY_H

#include <string>
#include <crowsite/config.h>
#include <filesystem>
#include <blt/std/hashmap.h>

namespace cs {
    
    namespace parser {
        class Post {
            private:
                HASHMAP<std::string, std::string> m_Values;
            public:
                explicit Post(const std::string& input);
                
                bool hasKey(const std::string& key);
                const std::string& operator[](const std::string& key);
                
                std::string dump();
        };
    }
    
    namespace fs {
        
        std::string createStaticFilePath(const std::string& file);
        std::string createWebFilePath(const std::string& file);
        std::string createDataFilePath(const std::string& file);
        
    }
    
}

#endif //CROWSITE_UTILITY_H
