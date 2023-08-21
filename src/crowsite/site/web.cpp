//
// Created by brett on 6/20/23.
//
#include <crowsite/site/web.h>
#include <fstream>
#include <ios>
#include <blt/std/logging.h>

#include <utility>
#include <sstream>
#include <algorithm>

namespace cs
{
    
    class LexerSyntaxException : public std::runtime_error
    {
        public:
            explicit LexerSyntaxException(const std::string& token):
                    std::runtime_error(
                            "Extended-mustache syntax error! An opening '{{' must be closed by '}}'! (near: '" +
                            token + "')"
                    )
            {}
    };
    
    class LexerException : public std::runtime_error
    {
        public:
            explicit LexerException(const std::string& message):
                    std::runtime_error("Extended-mustache syntax processing error! " + message)
            {}
    };
    
    class SyntaxException : public std::runtime_error
    {
        public:
            explicit SyntaxException():
                    std::runtime_error(
                            "Extended-mustache syntax error! Static context keys should not contain $"
                    )
            {}
    };
    
    std::unique_ptr<HTMLPage> HTMLPage::load(const std::string& path)
    {
        std::string htmlSource;
        std::ifstream htmlFile;
        if (!htmlFile.good())
            BLT_ERROR("Input stream not good!\n");
        // ensure we can throw exceptions:
        htmlFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            // open file
            htmlFile.open(path);
            std::stringstream htmlAsStream;
            // read file's buffer contents into streams
            htmlAsStream << htmlFile.rdbuf();
            // close file handlers
            htmlFile.close();
            // convert stream into std::string
            htmlSource = htmlAsStream.str();
        } catch (std::ifstream::failure& e)
        {
            BLT_ERROR("Unable to read file '%s'!\n", path.c_str());
            BLT_ERROR("Exception: %s", e.what());
            throw std::runtime_error("Failed to read file!\n");
        }
        return std::make_unique<HTMLPage>(HTMLPage(htmlSource));
    }
    
    HTMLPage::HTMLPage(std::string siteData): m_SiteData(std::move(siteData))
    {}
    
    std::string HTMLPage::render(StaticContext& context)
    {
        std::string processedSiteData = m_SiteData;
        
        std::string buffer;
        
        StringLexer lexer(processedSiteData);
        
        while (lexer.hasNext())
        {
            if (lexer.hasTemplatePrefix('$'))
            {
                lexer.consumeTemplatePrefix();
                std::string token;
                while (!lexer.hasTemplateSuffix())
                {
                    if (!lexer.hasNext())
                    {
                        BLT_FATAL("Invalid template syntax. EOF occurred before template was fully processed!");
                        throw LexerSyntaxException(token);
                    }
                    token += lexer.consume();
                }
                lexer.consumeTemplateSuffix();
                if (std::find_if(
                        context.begin(), context.end(),
                        [&token](auto in) -> bool {
                            return token == in.first;
                        }
                ) == context.end())
                {
                    // unable to find the token, we should throw an error to tell the user! (or admin in this case)
                    BLT_WARN("Unable to find token '%s'!", token.c_str());
                } else
                    buffer += context[token];
            } else
                buffer += lexer.consume();
        }
        
        return buffer;
    }
    
    void HTMLPage::resolveResources()
    {
    
    }
}
