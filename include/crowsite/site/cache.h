//
// Created by brett on 6/20/23.
//

#ifndef CROWSITE_CACHE_H
#define CROWSITE_CACHE_H

#include <crowsite/site/web.h>
#include <filesystem>

namespace cs {
    
    constexpr uint64_t toMB = 1024 * 1024;

    struct CacheSettings {
        // amount to hard prune at when reached, note: the engine will reduce all the way down to soft max memory
        uint64_t hardMaxMemory = 2048 * toMB;
        // it's more likely this will never be exceeded but the engine will make no attempt to prune more than softPruneAmount
        uint64_t softMaxMemory = 1024 * toMB;
        // max amount to soft prune
        uint64_t softPruneAmount = 2 * toMB;
    };
    
    class CacheEngine {
        private:
            struct CacheValue {
                int64_t cacheTime;
                std::filesystem::file_time_type lastModified;
                std::unique_ptr<HTMLPage> page;
                std::string renderedPage;
            };
            
            StaticContext& m_Context;
            CacheSettings m_Settings;
            hashmap<std::string, CacheValue> m_Pages;
            
            static uint64_t calculateMemoryUsage(const std::string& path, const CacheValue& value);
            /**
             * @return memory usage of the pages cache in bytes
             */
            uint64_t calculateMemoryUsage();
            void loadPage(const std::string& path);
            /**
             * Prunes the cache starting with the oldest pages we have loaded. (in bytes)
             */
            void prune(uint64_t amount);
        public:
            explicit CacheEngine(StaticContext& context, const CacheSettings& settings = {});
            const std::string& fetch(const std::string& path);
    };

}

#endif //CROWSITE_CACHE_H
