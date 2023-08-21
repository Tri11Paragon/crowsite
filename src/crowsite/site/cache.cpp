//
// Created by brett on 6/20/23.
//
#include <crowsite/site/cache.h>
#include <vector>
#include "blt/std/logging.h"
#include "crowsite/utility.h"
#include <algorithm>
#include <blt/std/time.h>
#include <blt/parse/mustache.h>

namespace cs
{
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
                // because any custom mustache syntax will have to have a prefix like '$' or '@'
                // it is fine that we make the assumption of 3 characters consumed.
                index += 3;
            }
            
            inline void consumeTemplateSuffix()
            {
                index += 2;
            }
            
            /**
             * This function assumes hasTemplatePrefix(char) has returned true and will consume both the prefix and suffix
             * @return the token found between the prefix and suffix
             * @throws LexerSyntaxError if the parser is unable to process a full token
             */
            inline std::string consumeToken(){
                consumeTemplatePrefix();
                std::string token;
                while (!hasTemplateSuffix()) {
                    if (!hasNext()) {
                        throw LexerSyntaxError();
                    }
                    token += consume();
                }
                consumeTemplateSuffix();
                return token;
            }
            
            inline char consume()
            {
                return str[index++];
            }
    };
    
    
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
        const auto& renderedPage = page->getRawSite();
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
                auto token = lexer.consumeToken();
                for (const auto& suffix : valid_file_endings)
                {
                    if (token.ends_with(suffix))
                    {
                        if (token == file)
                        {
                            BLT_WARN("Recursive load detected!");
                            BLT_WARN("Caching Engine will ignore this, however, it is recommended that you remove the recursive call.");
                            BLT_WARN("Detected in file '%s' offending link '%s'", file.c_str(), token.c_str());
                            break;
                        }
                        resolvedSite += fetch(token);
                        break;
                    }
                }
            } else if (lexer.hasTemplatePrefix('$'))
            {
                auto token = lexer.consumeToken();
                if (std::find_if(
                        m_Context.begin(), m_Context.end(),
                        [&token](auto in) -> bool {
                            return token == in.first;
                        }
                ) == m_Context.end())
                {
                    // unable to find the token, we should throw an error to tell the user! (or admin in this case)
                    BLT_WARN("Unable to find token '%s'!", token.c_str());
                } else
                    resolvedSite += m_Context[token];
            } else
                resolvedSite += lexer.consume();
        }
        
        page.getRawSite() = resolvedSite;
    }
    
    
}