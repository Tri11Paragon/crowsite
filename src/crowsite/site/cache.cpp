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
#include <blt/std/assert.h>

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
                    blt_throw(LexerSyntaxError());
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
                    // stmt -> (stmt) | expr | (expr)
                    // expr -> ident && ident | ident || ident
                    // ident -> lit | !lit
                    enum class TokenType
                    {
                        AND,    // &&
                        OR,     // ||
                        NOT,    // !
                        IDENT,  // literal
                        OPEN,   // (
                        CLOSE   // )
                    };
                    
                    static std::string decodeName(TokenType type)
                    {
                        switch (type)
                        {
                            case TokenType::AND:
                                return "AND";
                            case TokenType::OR:
                                return "OR";
                            case TokenType::NOT:
                                return "NOT";
                            case TokenType::IDENT:
                                return "IDENT";
                            case TokenType::OPEN:
                                return "OPEN";
                            case TokenType::CLOSE:
                                return "CLOSE";
                        }
                    }
                    
                    struct Token
                    {
                        TokenType type;
                        std::optional<std::string> value;
                    };
                    
                    static inline bool isSpecial(char c)
                    {
                        return c == '&' || c == '|' || c == '!' || c == '(' || c == ')' || std::isspace(c);
                    }
                    
                    std::vector<Token> tokens;
                    size_t t_index = 0;
                    size_t s_index = 0;
                    std::string str;
                    
                    inline bool hasNextToken()
                    {
                        return t_index < tokens.size();
                    }
                    
                    inline Token& peekToken()
                    {
                        return tokens[t_index];
                    }
                    
                    inline Token& consumeToken()
                    {
                        return tokens[t_index++];
                    }
                    
                    inline bool hasNext()
                    {
                        return s_index < str.size();
                    }
                    
                    inline char peek()
                    {
                        return str[s_index];
                    }
                    
                    inline char consume()
                    {
                        return str[s_index++];
                    }
                    
                    void processString()
                    {
                        while (hasNext())
                        {
                            char c = consume();
                            // ignore whitespace
                            if (isspace(c))
                                continue;
                            switch (c)
                            {
                                case '&':
                                    if (consume() != '&')
                                    blt_throw(LexerSyntaxError("Unable to parse logical expression. Found single '&' missing second '&'"));
                                    tokens.emplace_back(TokenType::AND);
                                    break;
                                case '|':
                                    if (consume() != '|')
                                    blt_throw(LexerSyntaxError("Unable to parse logical expression. Found single '|' missing second '|'"));
                                    tokens.emplace_back(TokenType::OR);
                                    break;
                                case '!':
                                    tokens.emplace_back(TokenType::NOT);
                                    break;
                                case '(':
                                    tokens.emplace_back(TokenType::OPEN);
                                    break;
                                case ')':
                                    tokens.emplace_back(TokenType::CLOSE);
                                    break;
                                default:
                                    std::string token;
                                    token += c;
                                    while (hasNext() && !isSpecial(peek()))
                                        token += consume();
                                    tokens.emplace_back(TokenType::IDENT, token);
                                    break;
                            }
                        }
                    }
                    
                    static inline bool isTrue(const context& context, const std::string& token)
                    {
                        //BLT_DEBUG("isTrue for token '%s' contains? %s", token.c_str(), context.contains(token) ? "True" : "False");
                        return context.contains(token) && !context.at(token).empty();
                    }
                    
                    // http://www.cs.unb.ca/~wdu/cs4613/a2ans.htm
                    bool factor(const context& context)
                    {
                        if (!hasNextToken())
                        blt_throw(LexerSyntaxError("Processing boolean factor but no token was found!"));
                        auto next = consumeToken();
                        switch (next.type)
                        {
                            case TokenType::IDENT:
                                if (!next.value.has_value())
                                blt_throw(LexerSyntaxError("Token identifier does not have a value!"));
                                return isTrue(context, next.value.value());
                            case TokenType::NOT:
                                return !factor(context);
                            case TokenType::OPEN:
                            {
//                                auto ret = ;
//                                if (consume() != ')')
//                                    blt_throw( LexerSyntaxError("Found token '(', parsed expression, but not ')' was found!");
                                return expr(context);
                            }
                            default:
                            blt_throw(LexerSyntaxError("Weird token found while parsing tokens, type: " + decodeName(next.type)));
                        }
                    }
                    
                    bool expr(const context& context)
                    {
                        auto fac = factor(context);
                        if (!hasNextToken())
                            return fac;
                        auto next = consumeToken();
                        switch (next.type)
                        {
                            case TokenType::AND:
                                return fac && expr(context);
                            case TokenType::OR:
                                return fac || expr(context);
                            case TokenType::CLOSE:
                            default:
                                //BLT_WARN("I've reached the default, help! %s", decodeName(next.type).c_str());
                                return fac;
                        }
                    }
                
                public:
                    explicit LogicalEval(std::string str): str(std::move(str))
                    {
                        processString();
                    }
                    
                    bool eval(const context& context)
                    {
                        return expr(context);
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
            
            static void getTagLocations(std::vector<size_t>& tagLocations, const std::string& tag, const std::string& data)
            {
                RuntimeLexer lexer(data);
                while (lexer.hasNext())
                {
                    if (lexer.hasTemplatePrefix('/') || lexer.hasTemplatePrefix('*'))
                    {
                        auto loc = lexer.index;
                        auto token = lexer.consumeToken();
                        if (tag == token)
                            tagLocations.push_back(loc);
                    } else
                        lexer.consume();
                }
                if (tagLocations.empty())
                blt_throw(LexerSearchFailure(tag));
            }
            
            static size_t findLastTagLocation(const std::string& tag, const std::string& data)
            {
                std::vector<size_t> tagLocations{};
                getTagLocations(tagLocations, tag, data);
                return tagLocations[tagLocations.size() - 1];
            }
            
            static size_t findNextTagLocation(const std::string& tag, const std::string& data)
            {
                std::vector<size_t> tagLocations{};
                getTagLocations(tagLocations, tag, data);
                return tagLocations[0];
            }
            
            static std::string searchAndReplace(const std::string& data, const context& context)
            {
                RuntimeLexer lexer(data);
                std::string results;
                while (lexer.hasNext())
                {
                    if (lexer.hasTemplatePrefix())
                    {
                        auto token = lexer.consumeToken();
                        auto searchField = lexer.str.substr(lexer.index);
                        auto endTokenLoc = RuntimeLexer::findNextTagLocation(token, searchField);
                        auto internalData = searchField.substr(0, endTokenLoc);
                        
                        LogicalEval eval(token);
                        bool expressionValue = eval.eval(context);
                        
                        if (expressionValue)
                        {
                            results += searchAndReplace(internalData, context);
                        }
                        
                        lexer.index += endTokenLoc;
                        if (lexer.hasTemplatePrefix('*'))
                        {
                            lexer.consumeToken();
                            auto nextSearchField = lexer.str.substr(lexer.index);
                            auto nextEndLoc = RuntimeLexer::findNextTagLocation(token, nextSearchField);
                            auto nextInternalData = nextSearchField.substr(0, nextEndLoc);
                            
                            if (!expressionValue)
                            {
                                results += searchAndReplace(nextInternalData, context);
                            }
                            lexer.index += nextEndLoc;
                        }
                        if (lexer.hasTemplatePrefix('/'))
                            lexer.consumeToken();
                        else
                        blt_throw(LexerSyntaxError("Ending token not found!"));
                        
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
    
    CacheEngine::CacheEngine(context& ctx, const CacheSettings& settings): m_Context(ctx),
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
            BLT_TRACE(
                    "Pruning page (%d bytes) aged %f seconds", page.memoryUsage,
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
    
    std::string CacheEngine::fetch(const std::string& path, const context& context)
    {
        auto fetched = fetch(path);
        return RuntimeLexer::searchAndReplace(fetched, context);
    }
    
    
}