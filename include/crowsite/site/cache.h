//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_CACHE_H
#define CROWSITE_CACHE_H

#include <crowsite/site/web.h>
#include <crowsite/util/crow_typedef.h>
#include <filesystem>
#include <blt/std/hashmap.h>

namespace cs
{
    
    class LexerSyntaxError : public std::runtime_error
    {
        public:
            LexerSyntaxError(): std::runtime_error("Invalid template syntax. EOF occurred before template was fully processed!")
            {}
            
            explicit LexerSyntaxError(const std::string& err): std::runtime_error(err)
            {}
    };
    
    class LexerSearchFailure : public std::runtime_error
    {
        public:
            explicit LexerSearchFailure(const std::string& str): std::runtime_error("The lexer failed to find ending for tag " + str)
            {}
    };
    
    constexpr uint64_t toMB = 1024 * 1024;
    
    struct CacheSettings
    {
        // amount to hard prune at when reached, note: the engine will reduce all the way down to soft max memory
        uint64_t hardMaxMemory = 2048 * toMB;
        // it's more likely this will never be exceeded but the engine will make no attempt to prune more than softPruneAmount
        uint64_t softMaxMemory = 1024 * toMB;
        // max amount to soft prune
        uint64_t softPruneAmount = 2 * toMB;
    };
    
    class CacheEngine
    {
        private:
            struct CacheValue
            {
                int64_t cacheTime;
                std::filesystem::file_time_type lastModified;
                std::unique_ptr<HTMLPage> page;
                std::string renderedPage;
            };
            
            context& m_Context;
            CacheSettings m_Settings;
            HASHMAP<std::string, CacheValue> m_Pages;
            
            static uint64_t calculateMemoryUsage(const std::string& path, const CacheValue& value);
            
            /**
             * @return memory usage of the pages cache in bytes
             */
            uint64_t calculateMemoryUsage();
            
            void resolveLinks(const std::string& file, HTMLPage& page);
            
            void loadPage(const std::string& path);
            
            /**
             * Prunes the cache starting with the oldest pages we have loaded. (in bytes)
             */
            void prune(uint64_t amount);
        
        public:
            explicit CacheEngine(context& context, const CacheSettings& settings = {});
            
            const std::string& fetch(const std::string& path);
            
            std::string fetch(const std::string& path, const context& context);
    };
    
}

#endif //CROWSITE_CACHE_H
