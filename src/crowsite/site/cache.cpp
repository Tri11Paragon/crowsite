//
// Created by brett on 6/20/23.
//
#include <crowsite/site/cache.h>
#include <vector>
#include "blt/std/logging.h"
#include "crowsite/utility.h"
#include <algorithm>
#include <blt/std/time.h>

namespace cs
{
    
    double toSeconds(uint64_t v)
    {
        return (double) (v) / 1000000000.0;
    }
    
    CacheEngine::CacheEngine(StaticContext& context, const CacheSettings& settings): m_Context(context),
                                                                                     m_Settings((settings))
    {}
    
    uint64_t CacheEngine::calculateMemoryUsage(const std::string& path, const CacheEngine::CacheValue& value)
    {
        uint64_t pageContentSize = path.size() * sizeof(char);
        pageContentSize += value.page->getRawSite().size() * sizeof(char);
        pageContentSize += value.renderedPage.size() * sizeof(char);
        return pageContentSize;
    }
    
    uint64_t CacheEngine::calculateMemoryUsage()
    {
        auto pagesBaseSize = m_Pages.size() * sizeof(CacheValue);
        
        uint64_t pageContentSizes = 0;
        for (const auto& value : m_Pages)
            pageContentSizes += calculateMemoryUsage(value.first, value.second);
        
        return pagesBaseSize + pageContentSizes;
    }
    
    const std::string& CacheEngine::fetch(const std::string& path)
    {
        bool load = false;
        auto find = m_Pages.find(path);
        if (find == m_Pages.end())
        {
            BLT_DEBUG("Page '%s' was not found in cache, loading now!", path.c_str());
            load = true;
        } else
        {
            auto lastWrite = std::filesystem::last_write_time(cs::fs::createWebFilePath(path));
            if (lastWrite != m_Pages[path].lastModified)
            {
                load = true;
                BLT_DEBUG("Page '%s' has been modified! Reloading now!", path.c_str());
            }
        }
        
        if (load)
        {
            auto memory = calculateMemoryUsage();
            
            if (memory > m_Settings.hardMaxMemory)
            {
                BLT_WARN("Hard memory limit was reached! Pruning to soft limit now!");
                prune(
                        m_Settings.hardMaxMemory - m_Settings.softMaxMemory
                        + memory - m_Settings.hardMaxMemory
                );
            }
            
            if (memory > m_Settings.softMaxMemory)
            {
                auto amount = std::min(m_Settings.softPruneAmount, memory - m_Settings.softMaxMemory);
                BLT_INFO("Soft memory limit was reached! Pruning %d bytes of memory", amount);
                prune(amount);
            }
            
            BLT_TRACE("Page storage memory usage: %fkb", memory / 1024.0);
            loadPage(path);
        }
        
        BLT_INFO("Fetched page %s", path.c_str());
        return m_Pages[path].renderedPage;
    }
    
    void CacheEngine::loadPage(const std::string& path)
    {
        auto start = blt::system::getCurrentTimeNanoseconds();
        
        auto fullPath = cs::fs::createWebFilePath(path);
        auto page = HTMLPage::load(fullPath);
        resolveLinks(path, *page);
        auto renderedPage = page->render(m_Context);
        m_Pages[path] = CacheValue{
                blt::system::getCurrentTimeNanoseconds(),
                std::filesystem::last_write_time(fullPath),
                std::move(page),
                renderedPage
        };
        
        auto end = blt::system::getCurrentTimeNanoseconds();
        BLT_INFO("Loaded page %s in %fms", path.c_str(), (end - start) / 1000000.0);
    }
    
    void CacheEngine::prune(uint64_t amount)
    {
        struct CacheSorting_t
        {
            uint64_t memoryUsage;
            std::string key;
        };
        
        std::vector<CacheSorting_t> cachedPages;
        for (auto& page : m_Pages)
            cachedPages.emplace_back(calculateMemoryUsage(page.first, page.second), page.first);
        
        std::sort(
                cachedPages.begin(), cachedPages.end(), [&](const CacheSorting_t& i1, const CacheSorting_t& i2) -> bool {
                    return m_Pages[i1.key].cacheTime < m_Pages[i2.key].cacheTime;
                }
        );
        
        uint64_t prunedAmount = 0;
        uint64_t prunedPages = 0;
        while (prunedAmount < amount)
        {
            auto page = cachedPages[0];
            BLT_TRACE("Pruning page (%d bytes) aged %f seconds", page.memoryUsage,
                      toSeconds(blt::system::getCurrentTimeNanoseconds() - m_Pages[page.key].cacheTime));
            prunedAmount += page.memoryUsage;
            m_Pages.erase(page.key);
            prunedPages++;
            cachedPages.erase(cachedPages.begin());
        }
        BLT_INFO("Pruned %d pages", prunedPages);
    }
    
    void CacheEngine::resolveLinks(const std::string& file, HTMLPage& page)
    {
        StringLexer lexer(page.getRawSite());
        std::string resolvedSite;
        
        const std::string valid_file_endings[3] = {
                ".css",
                ".js",
                ".part",
        };
        
        while (lexer.hasNext())
        {
            if (lexer.hasTemplatePrefix('@'))
            {
                lexer.consumeTemplatePrefix();
                std::string token;
                while (!lexer.hasTemplateSuffix()) {
                    if (!lexer.hasNext()) {
                        BLT_WARN("Invalid template syntax. EOF occurred before template was fully processed!");
                        break;
                    }
                    token += lexer.consume();
                }
                lexer.consumeTemplateSuffix();
                for (const auto& suffix : valid_file_endings){
                    if (token.ends_with(suffix)) {
                        auto path = cs::fs::createWebFilePath(token);
                        if (path == file){
                            BLT_WARN("Recursive load detected!");
                            BLT_WARN("Caching Engine will ignore this attempt, however, it is recommended that you remove the recursive call.");
                            BLT_WARN("Detected in file '%s' offending link '%s'", file.c_str(), token.c_str());
                        }
                        resolvedSite += fetch(path);
                        break;
                    }
                }
            } else
                resolvedSite += lexer.consume();
        }
        
        page.getRawSite() = resolvedSite;
    }
    
    
}