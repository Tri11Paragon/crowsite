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
    
    struct StringLexer
    {
        private:
            std::string str;
            size_t index = 0;
        public:
            explicit StringLexer(std::string str): str(std::move(str))
            {}
            
            inline bool hasNext()
            {
                if (index >= str.size())
                    return false;
                return true;
            }
            
            inline bool hasTemplatePrefix(char c)
            {
                if (index + 2 >= str.size())
                    return false;
                return str[index] == '{' && str[index + 1] == '{' && str[index + 2] == c;
            }
            
            inline bool hasTemplateSuffix()
            {
                if (index + 1 >= str.size())
                    return false;
                return str[index] == '}' && str[index + 1] == '}';
            }
            
            inline void consumeTemplatePrefix()
            {
                index += 3;
            }
            
            inline void consumeTemplateSuffix()
            {
                index += 2;
            }
            
            inline char consume()
            {
                return str[index++];
            }
    };
    
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
            /**
             * Attempts to resolve linked resources like CSS and JS in order to speedup page loading.
             */
            void resolveResources();
            /**
             * Uses the static context provided to resolve known variables before user requests
             * @param context context to use
             * @return string containing resolved static templates
             */
            std::string render(StaticContext& context);
            
            inline std::string& getRawSite() {
                return m_SiteData;
            }
    };
    
}

#endif //CROWSITE_WEB_H
