//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_WEB_H
#define CROWSITE_WEB_H

#include <memory>
#include <string>
#include <crowsite/config.h>

namespace cs {
    
    class StaticContext {
        private:
            hashmap<std::string, std::string> replacements;
        public:
            inline auto begin() {
                return replacements.begin();
            }
            inline auto end() {
                return replacements.end();
            }
            inline std::string& operator[](const std::string& key){
                return replacements[key];
            }
            inline void add(const std::string& key, const std::string& value){
                replacements[key] = value;
            }
    };
    
    class HTMLPage {
        private:
            static constexpr std::string opening = "{{$";
            static constexpr std::string closing = "}}";
            
            std::string rawSite;
            std::string processedSite;
            
            void throwSyntaxError(size_t begin);
        public:
            explicit HTMLPage(std::string siteData);
            std::string render(StaticContext& context);
    };
    
    std::unique_ptr<HTMLPage> loadHTMLPage(const std::string& path);
    
}

#endif //CROWSITE_WEB_H
