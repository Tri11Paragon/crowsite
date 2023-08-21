//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_WEB_H
#define CROWSITE_WEB_H

#include <memory>
#include <string>
#include <crowsite/config.h>
#include <utility>

namespace cs {
    
    class StaticContext {
        private:
            HASHMAP<std::string, std::string> replacements;
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
            std::string m_SiteData;
            explicit HTMLPage(std::string siteData);
        public:
            static std::unique_ptr<HTMLPage> load(const std::string& path);
            
            inline std::string& getRawSite() {
                return m_SiteData;
            }
    };
    
}

#endif //CROWSITE_WEB_H
