//
// Created by brett on 6/20/23.
//
#include <crowsite/site/cache.h>
#include <vector>
#include "blt/std/logging.h"
#include "blt/std/string.h"
#include "crowsite/utility.h"
#include <algorithm>
#include <blt/std/time.h>
#include <optional>

namespace cs
{
    class LexerBase
    {
        protected:
            std::string str;
            size_t index = 0;
        public:
            explicit LexerBase(std::string str): str(std::move(str))
            {}
            
            inline bool hasNext()
            {
                return index < str.size();
            }
            
            inline char peekPrefix()
            {
                return str[index + 2];
            }
            
            inline char consume()
            {
                return str[index++];
            }
            
            inline void consumeTemplatePrefix()
            {
                index += 3;
            }
            
            inline void consumeTemplateSuffix()
            {
                index += 2;
            }
            
            inline bool hasTemplateSuffix()
            {
                if (index + 1 >= str.size())
                    return false;
                return str[index] == '}' && str[index + 1] == '}';
            }
            
            /**
             * This function assumes hasTemplatePrefix(char) has returned true and will consume the prefix / suffix
             * @return the token found between the prefix and suffix
             * @throws LexerSyntaxError if the parser is unable to process a full token
             */
            inline std::string consumeToken()
            {
                consumeTemplatePrefix();
                std::string token;
                while (!hasTemplateSuffix())
                {
                    if (!hasNext())
                    {
                        throw LexerSyntaxError();
                    }
                    token += consume();
                }
                consumeTemplateSuffix();
                return token;
            }
    };
    
    class CacheLexer : public LexerBase
    {
        public:
            explicit CacheLexer(std::string str): LexerBase(std::move(str))
            {}
            
            static inline bool isCharNext(char c)
            {
                switch (c)
                {
                    case '$':
                    case '@':
                        return true;
                    default:
                        return false;
                }
            }
            
            inline bool hasTemplatePrefix()
            {
                if (index + 2 >= str.size())
                    return false;
                return str[index] == '{' && str[index + 1] == '{' && isCharNext(str[index + 2]);
            }
    };
    
    class RuntimeLexer : public LexerBase
    {
        private:
            class LogicalEval
            {
                private:
                    enum class TokenType
                    {
                        AND,    // &&
                        OR,     // ||
                        NOT,    // !
                        OPEN,   // (
                        CLOSE   // )
                    };
                    struct Token {
                        TokenType type;
                        std::optional<std::string> value;
                    };
                    std::vector<Token> tokens;
                    size_t m_index = 0;
                public:
                    void processString(const std::string& str)
                    {
                        size_t index = 0;
                        while (index < str.size())
                        {
                            char c = str[index++];
                            if (c == '&' || c == '|'){
                            
                            } else if (c == '!') {
                            
                            } else if (c == '(') {
                                tokens.emplace_back(TokenType::OPEN);
                            } else if (c == ')') {
                                tokens.emplace_back(TokenType::CLOSE);
                            }
                        }
                    }
            };
        
        public:
            explicit RuntimeLexer(std::string str): LexerBase(std::move(str))
            {}
            
            inline bool hasTemplatePrefix(char c = '%')
            {
                if (index + 2 >= str.size())
                    return false;
                return str[index] == '{' && str[index + 1] == '{' && str[index + 2] == c;
            }
            
            static std::string consumeToEndTemplate(const std::string& tokenName)
            {
            
            }
            
            static size_t findLastTagLocation(const std::string& tag, const std::string& data)
            {
                std::vector<size_t> tagLocations{};
                RuntimeLexer lexer(data);
                while (lexer.hasNext())
                {
                    if (lexer.hasTemplatePrefix('/'))
                    {
                        auto loc = lexer.index;
                        auto token = lexer.consumeToken();
                        if (tag == token)
                            tagLocations.push_back(loc);
                    } else
                        lexer.consume();
                }
                if (tagLocations.empty())
                    throw LexerSearchFailure(tag);
                return tagLocations[tagLocations.size() - 1];
            }
            
            static std::string searchAndReplace(const std::string& data, const RuntimeContext& context)
            {
                RuntimeLexer lexer(data);
                std::string results;
                while (lexer.hasNext())
                {
                    if (lexer.hasTemplatePrefix())
                    {
                        auto token = lexer.consumeToken();
                        auto searchField = lexer.str.substr(lexer.index);
                        auto endTokenLoc = RuntimeLexer::findLastTagLocation(token, searchField);
                        
                        auto
                    } else
                        results += lexer.consume();
                }
                return results;
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
        CacheLexer lexer(page.getRawSite());
        std::string resolvedSite;
        
        const std::string valid_file_endings[3] = {
                ".css",
                ".js",
                ".part",
        };
        
        while (lexer.hasNext())
        {
            if (lexer.hasTemplatePrefix())
            {
                auto prefix = lexer.peekPrefix();
                auto token = lexer.consumeToken();
                
                switch (prefix)
                {
                    case '@':
                        if (token == file)
                        {
                            BLT_WARN("Recursive load detected!");
                            BLT_WARN("Caching Engine will ignore this, however, it is recommended that you remove the recursive call.");
                            BLT_WARN("Detected in file '%s' offending link '%s'", file.c_str(), token.c_str());
                            break;
                        }
                        for (const auto& suffix : valid_file_endings)
                        {
                            if (token.ends_with(suffix))
                            {
                                resolvedSite += fetch(token);
                                break;
                            }
                        }
                        break;
                    case '$':
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
                        break;
                    default:
                        break;
                }
            } else
                resolvedSite += lexer.consume();
        }
        
        page.getRawSite() = resolvedSite;
    }
    
    std::string CacheEngine::fetch(const std::string& path, const RuntimeContext& context)
    {
        auto fetched = fetch(path);
        return RuntimeLexer::searchAndReplace(fetched, context);
    }
    
    
}